//Copyright(c) 2024 Electronic Arts Inc.All rights reserved.

#pragma once

THIRD_PARTY_INCLUDES_START
#include "src/FrameManager.h"
#include "iris/FrameData.h"
THIRD_PARTY_INCLUDES_END
#include "FrameStruct.h"
#include <unordered_set>
#include <string>

class IRISEA_API VideoRecorder
{
public:
	VideoRecorder(iris::Configuration& config);
	~VideoRecorder() {};

	//Enqueues last frame and writes to videofile when needed
	void EnqueueLastFrameAndCheck(FIrisFrame irisFrame, std::string lumResultStr, std::string redResultStr, std::string patternResultStr);
	
	//When a new sessions starts, a directory is created with its local date and time (/Saved/IrisSessions/Videos/Date&Time)
	void CreateDirectory();

	//Resets the VideoRecorder parameters when a session ends
	void Reset();

	void ToggleWarningSaving() { bWarningSaving = !bWarningSaving; }

private:

	void CreateAndOpenVideoFile(iris::FrameData frameData, cv::Size frameSize);

	void SaveAllFramesToVideofile(int framesToSubstract);

	std::string ConvertLongToTimeString(long milliseconds);

	void RemoveExceedance();
	bool CheckFrameDataPass(iris::FrameData frameData);
	bool CheckFrameData(iris::FrameData frameData);

	void RenameVideo();
	void RegisterEventType(const iris::FrameData& frameData, const std::string& lumResultStr, const std::string& redResultStr, const std::string& patternResultStr);
 
	const std::string folderPath = "/Saved/IrisSessions/Videos/";
	const std::string fileExtension = ".mp4";
	std::string finalFolderPath;

	cv::VideoWriter videoWriter;

	int frameManagerIndex;

	int fourcc;
	int sessionFPS = 60;
	int extraSecondsToRecord = 2;
	int remainingFramesToFill = sessionFPS * extraSecondsToRecord;

	bool bWarningSaving = false;
	//Last extraSecondsToRecord of frames
	TQueue<cv::Mat> lastFramesQueue;
	int framesInsideQueue = 0; 

	std::string tempVideoFile = "";
	std::string finalVideoFile = "";

	std::unordered_set<std::string> eventTypes;
};