//Copyright (c) 2024 Electronic Arts Inc. All rights reserved.

#pragma once

#include "RHI.h"
#include "CoreMinimal.h"

THIRD_PARTY_INCLUDES_START
#include "IPixelCaptureOutputFrame.h"
#undef check
#include "opencv2/opencv.hpp"
#define check(expr)				UE_CHECK_IMPL(expr)
THIRD_PARTY_INCLUDES_END
/**
 * 
 */
class IRISEA_API PixelCaptureOutputFrameBGR : public IPixelCaptureOutputFrame
{
public:
	PixelCaptureOutputFrameBGR(int32 Width, int32 Height)
	{
		width = Width;
		height = Height;
	}
	virtual ~PixelCaptureOutputFrameBGR() = default;

	virtual int32 GetWidth() const override { return width; }
	virtual int32 GetHeight() const override { return height; }
	void SetMat(cv::Mat &BGRMat) { BGRmat = BGRMat; }
	cv::Mat& GetMat() { return BGRmat; }

private:
	cv::Mat BGRmat;
	int32 width;
	int32 height;
};
