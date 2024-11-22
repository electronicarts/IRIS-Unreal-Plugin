//Copyright(c) 2024 Electronic Arts Inc.All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Platform.h"
#include "Modules/ModuleManager.h"

THIRD_PARTY_INCLUDES_START
#include "iris/Configuration.h"
#include "iris/VideoAnalyser.h"
#include "iris/FrameData.h"
#include "iris/Log.h"
#include "VideoRecorder.h"
THIRD_PARTY_INCLUDES_END
#include <FrameStruct.h>
#include <DataChart.h>
#include "FrameCapturerManager.h"
#include "AsyncAnalysis.h"

#define LOCAL_SAVE_VIDEO 1
#define DEBUG_FRAME_OPENCV 1
#define LOCAL_SAVE_FRAMES 0 //WIP

class FIrisEAModule : public IModuleInterface
{
public:

	FIrisEAModule(){};

	//Singleton
	inline static FIrisEAModule* GetInstance() 
	{
		if (!instance) {
			instance = new FIrisEAModule();
		}
		return instance;
	}

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;


	bool IsIrisActive() const { return bIrisActive;}

	bool IsDebugFrameActive() const { return bDebugCapturedFrame;}

	bool IsSaveFramesActive() const { return bSaveFramesAsPNGs; }

	void StartIrisSession();

	void EndIrisSession();

	/// <summary>
	/// Enqueues frames to be analysed
	/// </summary>
	/// <param name="frame">captured frame to analyse</param>
	void EnqueueIrisFrame(const FIrisFrame& frame) { framesToAnalyse.Enqueue(frame); };

	float GetFrameResizeProportion() const { return frameResizeProportion; }

	FTextureRHIRef GetFrameBuffer() const { return gameBuffer; }
	
	/// <summary>
	//Reset the parameters to be ready for the next Iris session
	/// </summary>
	void IrisReset();

	bool GetIsVideoRecording() { return bVideoRecording; }

	VideoRecorder* GetVideoRecorder() { return videoRecorder; }

	TQueue<FIrisFrame>* GetFramesToAnalyse() { return &framesToAnalyse; }

	iris::VideoAnalyser* GetVideoAnalyser() { return vA; }

	DataChart* GetChartManager() { return &chartManager; }

private:

	/// <summary>
	//Iris setup
	/// </summary>
	void IrisInit();

	/// <summary>
	/// Release Iris VideoAnalyser
	/// </summary>
	void IrisDeInit();

	/// <summary>
	//Toggle resutls graph visibility
	/// </summary>
	void ToggleResultsGraph() { chartManager.ToggleResultsGraph(); }

	/// <summary>
	//Toggle transitions graph visibility
	/// </summary>
	void ToggleTransitionsGraph() { chartManager.ToggleTransitionsGraph(); }

	/// <summary>
	//Toggle last rendered frame visibility (OpenCV)
	/// </summary>
	void ToggleDebugFrame() { bDebugCapturedFrame = !bDebugCapturedFrame; }

	/// <summary>
	//Toggle local frame saving as .png
	/// </summary>
	void ToggleFramesSave() { bSaveFramesAsPNGs = !bSaveFramesAsPNGs; }

	/// <summary>
	//Function called by the drawDelegateHandle
	/// </summary>
	void DrawGraph(UCanvas* Canvas, APlayerController* PlayerController);

	/// <summary>
	//Iris VideoAnalyser initialization
	/// </summary>
	bool VideoAnalyserSetUp();

	/// <summary>
	//Executes several commands to save time (debug only)
	/// </summary>
	void ExecuteAllCommands();

	/// <summary>
	//Function called when the Unreal BackBuffer is ready to be read
	/// </summary>
	void OnBackBufferReady_RenderThread(SWindow& SlateWindow, const FTextureRHIRef& BackBuffer) {	gameBuffer = BackBuffer; }

	/// <summary>
	//Asynchronous function that analyzes the captured frames using the Iris library
	/// </summary>
	void AsyncIrisGameThread();

	void RegisterCommands();

	void UnregisterCommands();

	void MoveChart(const TArray<FString, FDefaultAllocator>& Args);

	void ToggleRecordEvents();

	void ToggleRecordWarnings() { videoRecorder->ToggleWarningSaving();	}

	const float frameResizeProportion{ 0.2f }; //Resize proportion of the captured frame for IRIS analysis
	cv::Size frameSize; //Size of the captured frame (resize proportion applied)
	
	iris::Configuration configuration;
	
	iris::Log log;

	bool bIrisActive = false;

	bool bDebugCapturedFrame = false;

	bool bWarningRecording = false;

	bool bSaveFramesAsPNGs = false;

	inline static FIrisEAModule* instance = nullptr;

	FrameCapturerManager* frameCapturer = nullptr;

	TQueue<FIrisFrame> framesToAnalyse;

	//When this delegate is called, the DrawGraph function  is executed
	FDelegateHandle drawDelegateHandle;	

	FDelegateHandle preExitDelegateHandle;

	FDelegateHandle bufferReadyDelegateHandle;

	float irisSessionStartTime{ 0.f };

	iris::VideoAnalyser* vA = nullptr;

	bool bVideoRecording = false;

	//Manages how to draw the debug charts
	DataChart chartManager;

	VideoRecorder* videoRecorder;

	AsyncAnalysis* irisAnalysis = nullptr;
	FRunnableThread* asyncAnalysisThread = nullptr;

	//Unreal Engine BackBuffer
	FTextureRHIRef gameBuffer;
};