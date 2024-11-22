//Copyright(c) 2024 Electronic Arts Inc.All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"

class IRISEA_API AsyncAnalysis : public FRunnable
{
public:
	bool Init() override;
	/// <summary>
	// Async thread, frames are analysed while irisActive and frameQueue is not empty
	/// </summary>
	uint32 Run() override;
	void Stop() override;

private:
	TArray<FString> resultString = { "Pass", "PassWithWarning", "ExtendedFail" ,"FlashFail" };
};
