//Copyright(c) 2024 Electronic Arts Inc.All rights reserved.

#include "PixelCaptureCapturerRHIToBGRMat.h"
#include "PixelCaptureInputFrameRHI.h"
#include "PixelCaptureUtils.h"
#include "PixelCaptureOutputFrameBGR.h"
#include "PixelCaptureBufferFormat.h"

TSharedPtr<PixelCaptureCapturerRHIToBGRMat> PixelCaptureCapturerRHIToBGRMat::Create(float InScale)
{
	return TSharedPtr<PixelCaptureCapturerRHIToBGRMat>(new PixelCaptureCapturerRHIToBGRMat(InScale));
}

PixelCaptureCapturerRHIToBGRMat::PixelCaptureCapturerRHIToBGRMat(float InScale)
	: Scale(InScale)
{
}

PixelCaptureCapturerRHIToBGRMat::~PixelCaptureCapturerRHIToBGRMat()
{
	CleanUp();
}

void PixelCaptureCapturerRHIToBGRMat::Initialize(int32 InputWidth, int32 InputHeight)
{
	const int32 Width = InputWidth * Scale;
	const int32 Height = InputHeight * Scale;

	FRHITextureCreateDesc TextureDesc =
		FRHITextureCreateDesc::Create2D(TEXT("FPixelCaptureCapturerRHIToBGRMat StagingTexture"), Width, Height, EPixelFormat::PF_B8G8R8A8)
		.SetClearValue(FClearValueBinding::None)
		.SetFlags(ETextureCreateFlags::RenderTargetable)
		.SetInitialState(ERHIAccess::CopySrc)
		.DetermineInititialState();

	if (RHIGetInterfaceType() == ERHIInterfaceType::Vulkan)
	{
		TextureDesc.AddFlags(ETextureCreateFlags::External);
	}
	else
	{
		TextureDesc.AddFlags(ETextureCreateFlags::Shared);
	}

	StagingTexture = RHICreateTexture(TextureDesc);

	FRHITextureCreateDesc ReadbackDesc =
		FRHITextureCreateDesc::Create2D(TEXT("FPixelCaptureCapturerRHIToBGRMat ReadbackTexture"), Width, Height, EPixelFormat::PF_B8G8R8A8)
		.SetClearValue(FClearValueBinding::None)
		.SetFlags(ETextureCreateFlags::CPUReadback)
		.SetInitialState(ERHIAccess::CPURead)
		.DetermineInititialState();

	ReadbackTexture = RHICreateTexture(ReadbackDesc);

	int32 BufferWidth = 0, BufferHeight = 0;
	GDynamicRHI->RHIMapStagingSurface(ReadbackTexture, nullptr, ResultsBuffer, BufferWidth, BufferHeight);
	MappedStride = BufferWidth;

	FPixelCaptureCapturer::Initialize(InputWidth, InputHeight);
}

IPixelCaptureOutputFrame* PixelCaptureCapturerRHIToBGRMat::CreateOutputBuffer(int32 InputWidth, int32 InputHeight)
{
	const int32 Width = InputWidth * Scale;
	const int32 Height = InputHeight * Scale;
	return new PixelCaptureOutputFrameBGR(Width, Height);
}

void PixelCaptureCapturerRHIToBGRMat::BeginProcess(const IPixelCaptureInputFrame& InputFrame, IPixelCaptureOutputFrame* OutputBuffer)
{
	checkf(InputFrame.GetType() == StaticCast<int32>(PixelCaptureBufferFormat::FORMAT_RHI), TEXT("Incorrect source frame coming into frame capture process."));

	MarkCPUWorkStart();

	const FPixelCaptureInputFrameRHI& RHISourceFrame = StaticCast<const FPixelCaptureInputFrameRHI&>(InputFrame);
	FTexture2DRHIRef SourceTexture = RHISourceFrame.FrameTexture;

	FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

	RHICmdList.EnqueueLambda([this](FRHICommandListImmediate&) { MarkGPUWorkStart(); });

	RHICmdList.Transition(FRHITransitionInfo(SourceTexture, ERHIAccess::Unknown, ERHIAccess::CopySrc));
	RHICmdList.Transition(FRHITransitionInfo(StagingTexture, ERHIAccess::CopySrc, ERHIAccess::CopyDest));
	CopyTexture(RHICmdList, SourceTexture, StagingTexture, nullptr);

	RHICmdList.Transition(FRHITransitionInfo(StagingTexture, ERHIAccess::CopyDest, ERHIAccess::CopySrc));
	RHICmdList.Transition(FRHITransitionInfo(ReadbackTexture, ERHIAccess::CPURead, ERHIAccess::CopyDest));
	RHICmdList.CopyTexture(StagingTexture, ReadbackTexture, {});

	RHICmdList.Transition(FRHITransitionInfo(ReadbackTexture, ERHIAccess::CopyDest, ERHIAccess::CPURead));

	MarkCPUWorkEnd();

	// by adding this shared ref to the rhi lambda we can ensure that 'this' will not be destroyed
	// until after the rhi thread is done with it, so all the commands will still have valid references.
	TSharedRef<PixelCaptureCapturerRHIToBGRMat> ThisRHIRef = StaticCastSharedRef<PixelCaptureCapturerRHIToBGRMat>(AsShared());
	RHICmdList.EnqueueLambda([ThisRHIRef, OutputBuffer](FRHICommandListImmediate&) {
		ThisRHIRef->OnRHIStageComplete(OutputBuffer);
		});
}

void PixelCaptureCapturerRHIToBGRMat::OnRHIStageComplete(IPixelCaptureOutputFrame* OutputBuffer)
{
	MarkGPUWorkEnd();
	MarkCPUWorkStart();

	int32 Height = OutputBuffer->GetHeight();
	int32 Width = OutputBuffer->GetWidth();
	
	cv::Mat OutpuMat(Height, Width, CV_8UC4, ResultsBuffer, MappedStride*4);
	cv::cvtColor(OutpuMat, OutpuMat, cv::ColorConversionCodes::COLOR_BGRA2BGR);
	static_cast<PixelCaptureOutputFrameBGR*>(OutputBuffer)->SetMat(OutpuMat);

	MarkCPUWorkEnd();
	EndProcess();
}

void PixelCaptureCapturerRHIToBGRMat::CleanUp()
{
	if (ReadbackTexture)
	{
		GDynamicRHI->RHIUnmapStagingSurface(ReadbackTexture);
	}
	ResultsBuffer = nullptr;
}
