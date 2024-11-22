//Copyright(c) 2024 Electronic Arts Inc.All rights reserved.
#pragma once


#include "RHI.h"
#include "CoreMinimal.h"

THIRD_PARTY_INCLUDES_START
#include "PixelCaptureCapturer.h"
#undef check
#include "opencv2/opencv.hpp"
#define check(expr)				UE_CHECK_IMPL(expr)
THIRD_PARTY_INCLUDES_END

/**
 * A basic capturer that will capture RHI texture frames to OpenCV BGR mat utilizing cpu functions.
 * Involves CPU readback of GPU textures and processing of that readback data.
 * Input: FPixelCaptureInputFrameRHI
 * Output: FPixelCaptureOutputFrameI420
 */
class IRISEA_API PixelCaptureCapturerRHIToBGRMat : public FPixelCaptureCapturer, public TSharedFromThis<PixelCaptureCapturerRHIToBGRMat>
{
public:
	/**
	 * Creates a new Capturer capturing the input frame at the given scale.
	 * @param InScale The scale of the resulting output capture.
	 */
	static TSharedPtr<PixelCaptureCapturerRHIToBGRMat> Create(float InScale);
	virtual ~PixelCaptureCapturerRHIToBGRMat();

protected:
	virtual FString GetCapturerName() const override { return "RHIToBGRMat"; }
	virtual void Initialize(int32 InputWidth, int32 InputHeight) override;
	virtual IPixelCaptureOutputFrame* CreateOutputBuffer(int32 InputWidth, int32 InputHeight) override;
	virtual void BeginProcess(const IPixelCaptureInputFrame& InputFrame, IPixelCaptureOutputFrame* OutputBuffer) override;

private:
	float Scale = 1.0f;

	FTextureRHIRef StagingTexture;
	FTextureRHIRef ReadbackTexture;

	void* ResultsBuffer = nullptr;
	int32 MappedStride = 0;

	PixelCaptureCapturerRHIToBGRMat(float InScale);
	void OnRHIStageComplete(IPixelCaptureOutputFrame* OutputBuffer);
	void CleanUp();
};
