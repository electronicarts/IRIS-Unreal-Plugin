//Copyright(c) 2024 Electronic Arts Inc.All rights reserved.

#include "VideoRecorder.h"

VideoRecorder::VideoRecorder(iris::Configuration& config)
{
    fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    FrameManager::GetInstance()->Init(&config);
    frameManagerIndex = FrameManager::GetInstance()->RegisterNewElem(1.0f, 60);
}

void VideoRecorder::CreateDirectory()
{
    FString dateString = FDateTime::Now().ToString();
    finalFolderPath = folderPath + TCHAR_TO_UTF8(*dateString) + "/";
    FString FfinalPath = FPaths::ProjectDir() + UTF8_TO_TCHAR(finalFolderPath.c_str());
 
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(FfinalPath), true);
}

void VideoRecorder::Reset()
{
    if (videoWriter.isOpened())
    {
        videoWriter.release();
        RenameVideo();
    }
    FrameManager::GetInstance()->FrameTsReset(frameManagerIndex);
    lastFramesQueue.Empty();
    framesInsideQueue = 0;
}

void VideoRecorder::EnqueueLastFrameAndCheck(FIrisFrame irisFrame, std::string lumResultStr, std::string redResultStr, std::string patternResultStr)
{
    //Update FrameManager
    FrameManager::GetInstance()->NewFrameEntry(irisFrame.frameData);
    sessionFPS = FrameManager::GetInstance()->GetFrameNumInWindow(frameManagerIndex);

    lastFramesQueue.Enqueue(irisFrame.frameMatrix);
    framesInsideQueue++;
    int framesToSubstract = 0;

    //1st check event type
    RegisterEventType(irisFrame.frameData, lumResultStr, redResultStr, patternResultStr);

    //2nd step: Remove exceedance
    RemoveExceedance();

    //3rd step: if there is a video file open, the frames are dumped into the file
    if (videoWriter.isOpened())
    {
        framesToSubstract = CheckFrameDataPass(irisFrame.frameData);
        SaveAllFramesToVideofile(framesToSubstract);
        return;
    }
    //4th step: check if a fail/warning(optional) has ocurred
    if (CheckFrameData(irisFrame.frameData))
    {
        //On first fail creates and opens the video file
        CreateAndOpenVideoFile(irisFrame.frameData, irisFrame.frameMatrix.size());
    }
}

void VideoRecorder::CreateAndOpenVideoFile(iris::FrameData frameData, cv::Size frameSize)
{
    std::string videofileName = ConvertLongToTimeString(frameData.TimeStampVal);

    tempVideoFile = TCHAR_TO_UTF8(*FPaths::ProjectDir()) + finalFolderPath + videofileName;
    std::string videoPath = finalFolderPath + videofileName + fileExtension;
    
    FString FvideosPath = FPaths::ProjectDir() + UTF8_TO_TCHAR(videoPath.c_str());
    std::string TempFile = TCHAR_TO_UTF8(*FvideosPath);

    remainingFramesToFill = sessionFPS * extraSecondsToRecord;
    videoWriter = cv::VideoWriter(TempFile, fourcc, sessionFPS, frameSize);
}

void VideoRecorder::SaveAllFramesToVideofile(int framesToSubstract)
{
    while (!lastFramesQueue.IsEmpty())
    {
        videoWriter.write(*lastFramesQueue.Peek());
        remainingFramesToFill-= framesToSubstract;
        lastFramesQueue.Pop();
    }
    framesInsideQueue = 0;
    if (remainingFramesToFill <= 0)
    {
        Reset();
    }
}

std::string VideoRecorder::ConvertLongToTimeString(long milliseconds)
{
    long totalSeconds = milliseconds / 1000;
    std::string hours = std::to_string(totalSeconds / 3600);
    std::string minutes = std::to_string((totalSeconds % 3600) / 60);
    std::string seconds = std::to_string(totalSeconds % 60);

    std::string timeInString = hours + "h-" + minutes + "m-" + seconds + "s";

    return timeInString;
}

void VideoRecorder::RemoveExceedance()
{
    while (framesInsideQueue > sessionFPS * extraSecondsToRecord)
    {
        lastFramesQueue.Pop();
        framesInsideQueue--;
    }
}

bool VideoRecorder::CheckFrameData(iris::FrameData frameData)
{
    int lumResult = static_cast<int>(frameData.luminanceFrameResult);
    int redResult = static_cast<int>(frameData.redFrameResult);
    int pattResult = static_cast<int>(frameData.patternFrameResult);

    if (bWarningSaving)
    {
        return lumResult == 1 || redResult == 1;
    }
    
    return lumResult > 1 || redResult > 1 || pattResult > 1;
}

bool VideoRecorder::CheckFrameDataPass(iris::FrameData frameData)
{
    return  frameData.luminanceFrameResult == iris::FlashResult::Pass &&
            frameData.redFrameResult == iris::FlashResult::Pass &&
            frameData.patternFrameResult == iris::PatternResult::Pass;
}

void VideoRecorder::RegisterEventType(const iris::FrameData& frameData, const std::string& lumResultStr, const std::string& redResultStr, const std::string& patternResultStr)
{
    if (bWarningSaving && lumResultStr.empty() && redResultStr.empty() && patternResultStr.empty())
    {
        if (frameData.luminanceFrameResult == iris::FlashResult::PassWithWarning)
        {
            eventTypes.insert("_LuminancePassWithWarning");
        }
        else if (frameData.redFrameResult == iris::FlashResult::PassWithWarning) 
        {
            eventTypes.insert("_RedPassWithWarning");
        }
    }
    if (!lumResultStr.empty())
    {
        eventTypes.insert("_" + lumResultStr);
    }
    if (!redResultStr.empty())
    {
        eventTypes.insert("_" + redResultStr);
    }
    if (!patternResultStr.empty())
    {
        eventTypes.insert("_" + patternResultStr);
    }
}

void VideoRecorder::RenameVideo()
{
    finalVideoFile = tempVideoFile;

    if (bWarningSaving && eventTypes.size() > 1)
    {
        eventTypes.erase(eventTypes.begin());
    }
    
    for (const std::string& strEvent : eventTypes) 
    {
        finalVideoFile += strEvent;
    }

    tempVideoFile += ".mp4";
    finalVideoFile += ".mp4";
    IFileManager::Get().Move(UTF8_TO_TCHAR(finalVideoFile.c_str()), UTF8_TO_TCHAR(tempVideoFile.c_str()));

    eventTypes.clear();
    tempVideoFile = "";
    finalVideoFile = "";
}


