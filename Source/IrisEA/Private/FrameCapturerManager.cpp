//Copyright(c) 2024 Electronic Arts Inc.All rights reserved.

#include "FrameCapturerManager.h"
#include "PixelCaptureOutputFrameBGR.h"
#include "PixelCaptureInputFrameRHI.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "IrisEA.h"


FrameCapturerManager::FrameCapturerManager()
{
    resizeProportion = FIrisEAModule::GetInstance()->GetFrameResizeProportion();
    pixelCapturer = PixelCaptureCapturerRHIToBGRMat::Create(resizeProportion);
    frameCounter = -1;
}

FrameCapturerManager::~FrameCapturerManager()
{
}

void FrameCapturerManager::Initialize()
{
    frameCounter = -1;

    viewport = GEngine->GameViewport->Viewport;

#if LOCAL_SAVE_FRAMES
    CreateVideosDir();
#endif
}

void FrameCapturerManager::EndSession()
{
    pixelCapturer = PixelCaptureCapturerRHIToBGRMat::Create(resizeProportion);
    if (FIrisEAModule::GetInstance()->IsDebugFrameActive())
    {
        cv::destroyWindow("LastFrame");
    }
    currentSessionTime = 0;
}

void FrameCapturerManager::Tick(float DeltaTime)
{
    AsyncTask(ENamedThreads::GameThread, [this, DeltaTime]
        {
            FIrisEAModule* irisEA = FIrisEAModule::GetInstance();
            if (!irisEA->IsIrisActive())
            {
                frameCounter = -1;
                return;
            }
            //if viewport not valid end current session (user ends the sessions using 'Esc' button), Warning error shown on the console
            if (!GEngine->GameViewport)
            {
                irisEA->EndIrisSession();
                return;
            }
            //Pause frame capture when map transition is in progress
            if (!GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport))
            {
                return;
            }

#if WITH_EDITOR
            texture = GEngine->GameViewport->Viewport->GetRenderTargetTexture();
#else
            texture = irisEA->GetFrameBuffer();
#endif

            FIrisFrame frame = {};

            if (!texture)
            {
                return;
            }

            FIntPoint viewportSize{texture->GetDesc().Extent.X, texture->GetDesc().Extent.Y};
            //Skip 1st frame
            if (frameCounter == -1)
            {
                initialViewportSize = viewportSize;
                CaptureFrame(frame.frameMatrix);
                frameCounter = 0;
                return;
            }
            else if (ViewportResized(viewportSize))
            {
                irisEA->EndIrisSession();
                return;
            }

            TRACE_CPUPROFILER_EVENT_SCOPE(TotalTickIrisCapturer);

            CaptureFrame(frame.frameMatrix);

            //DeltaTime to ms
            currentSessionTime += DeltaTime * 1000;
            frame.frameData = iris::FrameData(frameCounter, currentSessionTime);           //Number and time of the frame
            irisEA->EnqueueIrisFrame(frame);   //Enqueue the frame in order to analyze it
            frameCounter++;

            if (irisEA->IsDebugFrameActive())
            {
                cv::imshow("LastFrame", frame.frameMatrix);
            }
        });
}

void FrameCapturerManager::CaptureFrame(cv::Mat& MatDest)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(IrisCaptureFrame);
    
    ENQUEUE_RENDER_COMMAND(CopyTextureCommand)([this](FRHICommandListImmediate& RHICmdList)
        {
            FPixelCaptureInputFrameRHI inputFrame = FPixelCaptureInputFrameRHI(texture);
            pixelCapturer->Capture(inputFrame);
        }
    );
    FlushRenderingCommands();
    TSharedPtr<PixelCaptureOutputFrameBGR> outputFrame = StaticCastSharedPtr<PixelCaptureOutputFrameBGR>(pixelCapturer->ReadOutput());
    MatDest = outputFrame->GetMat();
}

bool FrameCapturerManager::ViewportResized(const FIntPoint& newViewportSize)
{
    if (newViewportSize != initialViewportSize)
    {
        frameCounter = -1;
        initialViewportSize = newViewportSize;
        return true;
    }
    return false;
}
