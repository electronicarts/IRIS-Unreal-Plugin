//Copyright(c) 2024 Electronic Arts Inc.All rights reserved.

#include "AsyncAnalysis.h"
#include "IrisEA.h"

bool AsyncAnalysis::Init()
{
	return true;
}

uint32 AsyncAnalysis::Run()
{
	FIrisEAModule* instance = FIrisEAModule::GetInstance();
	while (instance->IsIrisActive())
	{

		//Analyse all frames in the frameQueue
		while (!instance->GetFramesToAnalyse()->IsEmpty())
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(AsyncIrisAnalysis);

			FIrisFrame* frame = instance->GetFramesToAnalyse()->Peek();
			instance->GetVideoAnalyser()->AnalyseFrame(frame->frameMatrix, frame->frameData.Frame, frame->frameData);

			FString lumResult;
			FString redResult;
			FString patternResult;
			//Log frame result
			if (frame->frameData.luminanceFrameResult == iris::FlashResult::FlashFail ||
				frame->frameData.luminanceFrameResult == iris::FlashResult::ExtendedFail)
			{
				lumResult = "Luminance" + resultString[static_cast<int>(frame->frameData.luminanceFrameResult)];
				UE_LOG(LogTemp, Error, TEXT("Iris %s trigger"), *lumResult)
			}
			if (frame->frameData.redFrameResult == iris::FlashResult::FlashFail ||
				frame->frameData.redFrameResult == iris::FlashResult::ExtendedFail)
			{
				redResult = "Red" + resultString[static_cast<int>(frame->frameData.redFrameResult)];
				UE_LOG(LogTemp, Error, TEXT("Iris %s trigger"), *redResult);
			}
			if (frame->frameData.patternFrameResult == iris::PatternResult::Fail)
			{
				patternResult = "PatternFail";
				UE_LOG(LogTemp, Error, TEXT("Iris %s trigger"), *patternResult);
			}

			instance->GetChartManager()->PushFrameDataToArray(frame->frameData);
			if (instance->GetIsVideoRecording())
			{
				instance->GetVideoRecorder()->EnqueueLastFrameAndCheck(*frame, TCHAR_TO_UTF8(*lumResult), TCHAR_TO_UTF8(*redResult), TCHAR_TO_UTF8(*patternResult));
			}
			instance->GetFramesToAnalyse()->Pop();
		}
	}
	//Analysis completed, reset Iris parameters
	instance->IrisReset();

	return 0;
}

void AsyncAnalysis::Stop()
{
}
