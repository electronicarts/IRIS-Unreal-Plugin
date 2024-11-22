//Copyright(c) 2024 Electronic Arts Inc.All rights reserved.

#include "IrisEA.h"
#include "HAL/Platform.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "UObject/Object.h"
#include "Async/Async.h"
#include "Debug/DebugDrawService.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"


#if PLATFORM_WINDOWS
	#include "Windows/AllowWindowsPlatformTypes.h"
	#include "Windows/WindowsHWrapper.h"
#endif

THIRD_PARTY_INCLUDES_START
THIRD_PARTY_INCLUDES_END

#if PLATFORM_WINDOWS
	#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include <Kismet/GameplayStatics.h>

#define LOCTEXT_NAMESPACE "FIrisEAModule"

void FIrisEAModule::StartupModule()
{
	instance = this;
	IrisInit();
	//Unreal Engine console commands
	RegisterCommands();
}

void FIrisEAModule::ShutdownModule()
{
	IrisDeInit();
	UnregisterCommands();
}

void FIrisEAModule::IrisInit()
{
	log.Init(false, false);

	//Get appsettings.json path
	FString PluginBaseDir = IPluginManager::Get().FindPlugin("IrisEA")->GetBaseDir();
	FString CurrentDir = FPaths::Combine(*PluginBaseDir, TEXT("Source/ThirdParty/IrisLibrary/Win64/"));

	//Load configuration  
	configuration.Init(TCHAR_TO_UTF8(*CurrentDir));

	//VideoAnalyser
	vA = new iris::VideoAnalyser(&instance->configuration);

	frameCapturer = new FrameCapturerManager();
	videoRecorder = new VideoRecorder(configuration);
	irisAnalysis = new AsyncAnalysis();
}

void FIrisEAModule::IrisDeInit()
{
	//VideoAnalyser clean up
	vA->DeInit();
	delete vA;
	delete frameCapturer;
	delete videoRecorder;
	irisAnalysis->Stop();
	delete irisAnalysis;
}

void FIrisEAModule::IrisReset() 
{
	vA->DeInit();
	chartManager.Reset();
	videoRecorder->Reset();
}

DECLSPEC_NOINLINE bool FIrisEAModule::VideoAnalyserSetUp()
{
	if (!GEngine->GameViewport)
	{
		UE_LOG(LogTemp, Warning, TEXT("Iris.StartSession should not be used if the game is not running."));
		return false;
	}

	FViewport* Viewport = GEngine->GameViewport->Viewport;

	if (!Viewport)
	{
		UE_LOG(LogTemp, Error, TEXT("Current viewport not initialized."));
		return false;
	}
	
	int32 Width = Viewport->GetSizeXY().X * frameResizeProportion;
	int32 Height = Viewport->GetSizeXY().Y * frameResizeProportion;

	//VideoAnalyser Init
	frameSize = { Height , Width };
	vA->RealTimeInit(frameSize);
	return true;
}

void FIrisEAModule::ExecuteAllCommands()
{
	StartIrisSession();
	ToggleResultsGraph();
	ToggleTransitionsGraph();
}

void FIrisEAModule::StartIrisSession()
{
	if (bIrisActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("A session of Iris is currently running, use the 'Iris.EndSession' to end the current session."));
		return;
	}
	if (VideoAnalyserSetUp())
	{
		UE_LOG(LogTemp, Log, TEXT("Frame capture and Iris analysis activated"));
		bIrisActive = true;
		frameCapturer->Initialize();
		AsyncIrisGameThread();
		if (bVideoRecording)
		{
			videoRecorder->CreateDirectory();
		}
		preExitDelegateHandle = FCoreDelegates::OnPreExit.AddRaw(this, &FIrisEAModule::EndIrisSession);
		chartManager.SetChartValues(configuration.GetTransitionTrackerParams()->maxTransitions, configuration.GetTransitionTrackerParams()->warningTransitions);
		drawDelegateHandle = UDebugDrawService::Register(TEXT("Game"), FDebugDrawDelegate::CreateRaw(this, &FIrisEAModule::DrawGraph));
		asyncAnalysisThread = FRunnableThread::Create(irisAnalysis, TEXT("IrisAsyncAnalysisThread"));
#if !WITH_EDITOR
		bufferReadyDelegateHandle = FSlateApplication::Get().GetRenderer()->OnBackBufferReadyToPresent().AddRaw(this, &FIrisEAModule::OnBackBufferReady_RenderThread);
#endif
	}
}

void FIrisEAModule::EndIrisSession()
{
	if (!bIrisActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("The session has not yet started, use the 'Iris.StartSession' command to start an Iris analysis session."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Frame capture and Iris analysis deactivated"));
	bIrisActive = false;
	frameCapturer->EndSession();
	FString FilePath = FPaths::ProjectDir() / TEXT("IrisSessionMetrics.json");
	UDebugDrawService::Unregister(drawDelegateHandle);
	FCoreDelegates::OnPreExit.Remove(preExitDelegateHandle);
	asyncAnalysisThread->Kill();

#if !WITH_EDITOR
	FSlateApplication::Get().GetRenderer()->OnBackBufferReadyToPresent().Remove(bufferReadyDelegateHandle);
#endif
}

void FIrisEAModule::AsyncIrisGameThread()
{
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float DeltaTime)
		{
			frameCapturer->Tick(DeltaTime);
			return bIrisActive;
		}));
}

void FIrisEAModule::ToggleRecordEvents()
{
	if (bVideoRecording)
	{
		videoRecorder->Reset();
	}
	else if (bIrisActive)
	{
		videoRecorder->CreateDirectory();
	}
	bVideoRecording = !bVideoRecording;
}

void FIrisEAModule::DrawGraph(UCanvas* Canvas, APlayerController* PlayerController)
{
	if (!Canvas)
	{
		return;
	}

	chartManager.DrawResultsGraph(Canvas);
	chartManager.DrawTransitionsGraph(Canvas);
}

void FIrisEAModule::RegisterCommands()
{
	// Register the console commands
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Iris.ResultsGraph"),
		TEXT("Toggles the visibility of the iris results graph."),
		FConsoleCommandDelegate::CreateRaw(this, &FIrisEAModule::ToggleResultsGraph)
	);
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Iris.TransitionsGraph"),
		TEXT("Toggles the visibility of the iris transitions graph."),
		FConsoleCommandDelegate::CreateRaw(this, &FIrisEAModule::ToggleTransitionsGraph)
	);
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Iris.StartSession"),
		TEXT("Starts the frame capture and the Iris analysis."),
		FConsoleCommandDelegate::CreateRaw(this, &FIrisEAModule::StartIrisSession)
	);
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Iris.EndSession"),
		TEXT("Ends the frame capture and the Iris analysis."),
		FConsoleCommandDelegate::CreateRaw(this, &FIrisEAModule::EndIrisSession)
	);
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Iris.StartAll"),
		TEXT("Enable Iris analysis, the captured frame image, graphs visibility"),
		FConsoleCommandDelegate::CreateRaw(this, &FIrisEAModule::ExecuteAllCommands)
	);
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Iris.MoveChart"),
		TEXT("Move debug charts to given direction ( UP = 0, DOWN = 1, LEFT = 2, RIGHT = 3, RESET = 4)"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FIrisEAModule::MoveChart)
	);

#if DEBUG_FRAME_OPENCV 
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Iris.DebugFrame"),
		TEXT("Toggles the captured frame image."),
		FConsoleCommandDelegate::CreateRaw(this, &FIrisEAModule::ToggleDebugFrame)
	);
#endif // DEBUG_FRAME_OPENCV
#if LOCAL_SAVE_VIDEO
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Iris.RecordEventsOnVideo"),
		TEXT("Save the Iris fails triggers of the sessions as a .mp4 file (root/Saved/IrisSessions/Videos/)."),
		FConsoleCommandDelegate::CreateRaw(this, &FIrisEAModule::ToggleRecordEvents)
	);
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Iris.IncludeWarningsOnVideo"),
		TEXT("Includes the PassWithWarning events to the video recording feature."),
		FConsoleCommandDelegate::CreateRaw(this, &FIrisEAModule::ToggleRecordWarnings)
	);
#endif //LOCAL_SAVE_VIDEO
#if LOCAL_SAVE_FRAMES
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Iris.SaveFramesAsPNG"),
		TEXT("Save the Iris session frames as PNG (root/Saved/DebugFrames/)"),
		FConsoleCommandDelegate::CreateRaw(this, &FIrisEAModule::ToggleFramesSave)
	);
#endif //LOCAL_SAVE_FRAMES
}

void FIrisEAModule::MoveChart(const TArray<FString, FDefaultAllocator>& Args)
{
	if (Args.Num() <= 0)
		return;

	int32 Direction = FCString::Atoi(*Args[0]);
	chartManager.MoveChart(static_cast<DataChart::EDirections>(Direction));
}

void FIrisEAModule::UnregisterCommands()
{
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("Iris.ResultsGraph"), false);
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("Iris.TransitionsGraph"), false);
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("Iris.StartSession"), false);
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("Iris.EndSession"), false);
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("Iris.DebugFrame"), false);
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("Iris.StartAll"), false);
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("Iris.MoveChart"), false);
#if DEBUG_FRAME_OPENCV
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("Iris.DebugFrame"), false);
#endif
#if LOCAL_SAVE_VIDEO
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("Iris.RecordEventsOnVideo"), false);
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("Iris.IncludeWarningsOnVideo"), false);
#endif
#if LOCAL_SAVE_FRAMES
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("Iris.SaveFramesAsPNG"), false);
#endif
}
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FIrisEAModule, IrisEA)
