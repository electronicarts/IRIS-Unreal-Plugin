//Copyright(c) 2024 Electronic Arts Inc.All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "iris/FrameData.h"
#undef check
#include "opencv2/opencv.hpp"
#define check(expr)				UE_CHECK_IMPL(expr)

struct FIrisFrame
{
	cv::Mat frameMatrix;
	iris::FrameData frameData;

};