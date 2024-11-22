//Copyright(c) 2024 Electronic Arts Inc.All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include <FrameStruct.h>
#include "PixelCaptureCapturerRHIToBGRMat.h"
#include "VideoRecorder.h"

class IRISEA_API FrameCapturerManager
{
public:
    FrameCapturerManager();
    ~FrameCapturerManager();

    void Tick(float DeltaTime);
    
    void Initialize();

    /// <summary>
    //Destroy the OpenCV debug window (if there's any), if a video of the session has been recorded it must be released
    /// </summary>
    void EndSession();

private:
    /// <summary>
    //Function that captures the Unreal Engine frame texture and saves it into MatDest 
    /// </summary>
    void CaptureFrame(cv::Mat& MatDest);

    /// <summary>
    // Function called when the Unreal Engine Viewport has been resized, the Iris session must end
    /// </summary>
    bool ViewportResized(const FIntPoint& newViewportSize);

    FViewport* viewport = nullptr;

    FTextureRHIRef texture = nullptr;

    TSharedPtr<PixelCaptureCapturerRHIToBGRMat> pixelCapturer;

    int frameCounter;

    float resizeProportion;

    float currentSessionTime{ 0.f };

    FIntPoint initialViewportSize;
};
