//Copyright(c) 2024 Electronic Arts Inc.All rights reserved.


#include "DataChart.h"

DataChart::DataChart()
{
}

DataChart::~DataChart()
{
}

void DataChart::DrawResultsGraph(UCanvas* Canvas)
{
	if (bShowResultsGraph)
	{
		// Draw the filled background box using FCanvasTileItem
		FVector2D TopLeft = FVector2D(startX-1, startY);
		FCanvasTileItem TileItem(TopLeft, chartSize, backgroundColor);
		TileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(TileItem);

		//Draw horizontal lines
		for (int32 i = 0; i < resultsLineColors.Num(); ++i)
		{
			FVector2D LineStart = FVector2D(startX, startY + (i + 1) * lineHeight);
			FVector2D LineEnd = FVector2D(startX + chartWidth, startY + (i + 1) * lineHeight);

			// Draw line
			Canvas->K2_DrawLine(LineStart, LineEnd, lineThickness/2, resultsLineColors[i]);

			// Draw text next to the line
			FVector2D TextPosition = FVector2D(startX + chartWidth + 10.f, startY + (i + 1) * lineHeight - 5.f); // Adjust Y position for centering text
			Canvas->K2_DrawText(GEngine->GetLargeFont(), resultsLineTexts[i], TextPosition, FVector2D(1.f, 1.f), resultsLineColors[i]);
		}

		if (dataArray.IsEmpty())
		{
			return;
		}
		//Draw FrameData transitions
		FVector2D PreviousPointLuminance;
		FVector2D PreviousPointRedSat;
		for (int i = 0; i < dataArray.Num(); i++)
		{
			float xPos = i + 0.5;
			int lumPos = static_cast<int>(dataArray[i].luminanceFrameResult);
			int redPos = static_cast<int>(dataArray[i].redFrameResult);

			FVector2D LuminancePointPosition = FVector2D(startX + xPos, startY + (4 - lumPos) * lineHeight);
			FVector2D RedSatPointPosition = FVector2D(startX + xPos, startY + (4 - redPos) * lineHeight);

			if (i > 0)
			{
				Canvas->K2_DrawLine(PreviousPointRedSat, RedSatPointPosition, lineThickness, purpleColor);
				Canvas->K2_DrawLine(PreviousPointLuminance, LuminancePointPosition, lineThickness, FLinearColor::White);
			}
			PreviousPointLuminance = LuminancePointPosition;
			PreviousPointRedSat = RedSatPointPosition;
		}
	}
}

void DataChart::DrawTransitionsGraph(UCanvas* Canvas)
{
	if (bShowTransitionsGraph)
	{
		// Draw the filled background box using FCanvasTileItem
		FVector2D TransTopLeft = FVector2D(startX-1, transitionsStartY);
		FCanvasTileItem TransTileItem(TransTopLeft, chartSize, backgroundColor);
		TransTileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(TransTileItem);

		//Draw horizontal lines
		for (int32 i = transitionsLineTexts.Num() - 1; i >= 0; i--)
		{
			FVector2D LineStart = FVector2D(startX, transitionsStartY + chartHeight - (LineValues[i]) * transitionsLineHeight);
			FVector2D LineEnd = FVector2D(startX + chartWidth, transitionsStartY + chartHeight - (LineValues[i]) * transitionsLineHeight);

			// Draw line
			Canvas->K2_DrawLine(LineStart, LineEnd, lineThickness / 2, transitionsLineColors[i]);

			// Draw text next to the line
			FVector2D TextPosition = FVector2D(startX + chartWidth + 10.f, transitionsStartY + chartHeight - (LineValues[i]) * transitionsLineHeight - 5.f); // Adjust Y position for centering text
			Canvas->K2_DrawText(GEngine->GetLargeFont(), transitionsLineTexts[i], TextPosition, FVector2D(1.f, 1.f), transitionsLineColors[i]);
		}

		if (dataArray.IsEmpty())
		{
			return;
		}

		//Draw FrameData results
		FVector2D PreviousPointLumTrans;
		FVector2D PreviousPointRedTrans;
		for (int i = 0; i < dataArray.Num(); i++)
		{
			float xPos = i+0.5;
			int lumPos = static_cast<int>(dataArray[i].LuminanceTransitions);
			if (lumPos > maxTransitions)lumPos = maxTransitions;
			int redPos = static_cast<int>(dataArray[i].RedTransitions);
			if (redPos > maxTransitions)redPos = maxTransitions;

			FVector2D LuminancePointPosition = FVector2D(startX + xPos, transitionsStartY + chartHeight - (lumPos * transitionsLineHeight));
			FVector2D RedSatPointPosition = FVector2D(startX + xPos, transitionsStartY + chartHeight - (redPos * transitionsLineHeight));

			if (i > 0)
			{
				Canvas->K2_DrawLine(PreviousPointRedTrans, RedSatPointPosition, lineThickness, purpleColor);
				Canvas->K2_DrawLine(PreviousPointLumTrans, LuminancePointPosition, lineThickness, FLinearColor::White);
			}
			PreviousPointLumTrans = LuminancePointPosition;
			PreviousPointRedTrans = RedSatPointPosition;
		}
	}
}


void DataChart::PushFrameDataToArray(const iris::FrameData &NewFrameData)
{
	dataArray.Push(NewFrameData);

	while (dataArray.Num() >= chartWidth-1)
	{
		dataArray.RemoveAt(0);
	}
}

void DataChart::MoveChart(EDirections MoveTo)
{
	switch (MoveTo)
	{
	case DataChart::UP:
		startY -= 50.f;
		transitionsStartY -= 50.f;
		break;
	case DataChart::DOWN:
		startY += 50.f;
		transitionsStartY += 50.f;
		break;
	case DataChart::LEFT:
		startX -= 50.f;
		break;
	case DataChart::RIGHT:
		startX += 50.f;
		break;
	case DataChart::RESET:
		startX = RESULTS_POS;
		startY = RESULTS_POS;
		transitionsStartY = TRANSITIONS_POS;
		break;
	default:
		break;
	}
}

void DataChart::SetChartValues(int failTransitions, int warningTransitions)
{
	LineValues[0] = failTransitions + 1;
	LineValues[1] = warningTransitions;

	transitionsLineTexts[0] = FString::Printf(TEXT("%d-FAIL"), LineValues[0]);
	transitionsLineTexts[1] = FString::Printf(TEXT("%d-WARNING"), LineValues[1]);
}
