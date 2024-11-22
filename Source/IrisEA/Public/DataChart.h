//Copyright(c) 2024 Electronic Arts Inc.All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Debug/DebugDrawService.h"
#include "Engine/Canvas.h"
#include <FrameStruct.h>

/**
 * 
 */
class IRISEA_API DataChart
{
public:

	DataChart();
	~DataChart();

	void Reset() { dataArray.Empty(); }
	void DrawTransitionsGraph(UCanvas* Canvas);
	void DrawResultsGraph(UCanvas* Canvas);
	void PushFrameDataToArray(const iris::FrameData& NewFrameData);

	/// <summary>
	//Toggle results graph visibility
	/// </summary>
	void ToggleResultsGraph() { bShowResultsGraph = !bShowResultsGraph; }

	/// <summary>
	//Toggle transitions graph visibility
	/// </summary>
	void ToggleTransitionsGraph() { bShowTransitionsGraph = !bShowTransitionsGraph; }

	enum EDirections { UP = 0, DOWN, LEFT, RIGHT, RESET };

	/// <summary>
	//Move the charts in a given direction 50 units
	/// </summary>
	void MoveChart(EDirections MoveTo);

	void SetChartValues(int failTransitions, int warningTransitions);

private:

	//Values used to draw the charts

	const float RESULTS_POS{ 50.f };

	float startX = RESULTS_POS;
	float startY = RESULTS_POS;
	const float chartWidth{ 300.f };
	const float chartHeight{ 120.f };
	const float lineHeight{ 30.f };
	const float lineThickness{ 2.f };

	const float TRANSITIONS_POS = RESULTS_POS + chartHeight + 25.f;
	
	float transitionsStartY = TRANSITIONS_POS;
	const int maxTransitions{ 12 };
	const float transitionsLineHeight = chartHeight / maxTransitions;

	const FLinearColor purpleColor{ 0.4f, 0.27f, 1.f, 1.f };
	const FLinearColor yellowColor{ 1.f, 0.8f, 0.f, 1.f };
	const FLinearColor backgroundColor{ 0.f, 0.f, 0.f, 0.7f };

	TArray<int> LineValues = { 7, 4 };

	const TArray<FLinearColor> resultsLineColors = { FLinearColor::Red, FLinearColor::Red, yellowColor, FLinearColor::Green };
	const TArray<FString> resultsLineTexts = { TEXT("FLASH FAIL"), TEXT("EXTENDED FAIL"), TEXT("WARNING"), TEXT("PASS") };

	const TArray<FLinearColor> transitionsLineColors = { FLinearColor::Red, yellowColor };
	TArray<FString> transitionsLineTexts = { TEXT("7-FAIL"), TEXT("4-WARNING") };

	FVector2D chartSize = FVector2D(chartWidth, chartHeight);

	//Last 300 FrameData to draw the result and transition values
	TArray<iris::FrameData> dataArray;

	bool bShowResultsGraph = false;
	
	bool bShowTransitionsGraph = false;
};
