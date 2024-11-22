//Copyright (C) 2023 Electronic Arts, Inc.  All rights reserved.
#pragma once
#include "iris/Configuration.h"
#include "ConfigurationParams.h"
#include "iris/FrameData.h"

class FrameManager 
{
public:

	//Singleton
	inline static FrameManager* GetInstance() {
		if (!instance) {
			instance = new FrameManager();
		}
		return instance;
	}

	void Init(iris::Configuration* configuration);
	void DeInit();

	/// <summary>
	/// Creates a new element in the vectors, then returns the index to access the new elements.
	/// </summary>
	/// <param name="timeBarrier:"> max time in window (seconds) </param>
	/// <param name="maxFrames:"> max frames in window </param>
	int RegisterNewElem(float timeBarrier, const int& maxFrames);

	/// <summary>
	/// Real-time only. m_frameTimeStamps and m_framesToRemove vectors are updated.
	/// </summary>
	void NewFrameEntry(const iris::FrameData& data);

	/// <summary>
	/// Returns the number of frames that need to be removed from the counter vectors.
	/// </summary>
	/// <param name="index:"> Integer to access the desired element in the vectors. </param>
	int	GetFramesToRemove(const int& index, const int& currentFrames) const;

	/// <summary>
	/// Returns the number of frames in the window.
	/// </summary>
	/// <param name="index:"> Integer to access the desired element in the vectors. </param>
	int GetFrameNumInWindow(const int& index) const;

	/// <summary>
	/// Real-time only. m_frameTimeStamps and m_framesToRemove are reset.
	/// </summary>
	/// <param name="index:"> Integer to access the desired element in the vectors. </param>
	void FrameTsReset(const int& index);
	
private:

	struct FrameTimeStamps
	{
		FrameTimeStamps(float timeBarrier, int maxFrames)
		{
			SetTimeBarrier(timeBarrier);
			frameTimeStamp.reserve(maxFrames);
			timesBetweenPairOfFrames.reserve(maxFrames);
		}
		
		std::vector<long> frameTimeStamp; //vector of current frame entry times frames in the window

		std::vector<long> timesBetweenPairOfFrames; //vector of current times between frames in the window

		long timesSum = 0; //total sum of times between each pair of Frames 

		/// <summary>
		/// Set time barrier in milliseconds 
		/// </summary>
		/// <param name="seconds:"> time barrier in seconds </param>
		void SetTimeBarrier(float seconds) {
			timeBarrier = seconds * 1000.0;
		}

		/// <summary>
		/// Check if when adding a new frame it is needed to update frameTimeStamp and timesBetweenPairOfFrames vectors (timeBarrier has been surpassed),
		/// then add newFrameTime to frameTimeStamp and timesBetweenPairOfFrames vectors.
		/// This function returns an integer (number of frames to remove from the count vectors)
		/// </summary>
		/// <param name="newFrameTime:"> new frame MS </param>
		int GetFrameNumToRemove(const unsigned long& newFrameTime)
		{
			int framesToRemove = 0; //number of frames to that are no longer inside the time window
			long newTimeBetFrames = 0.0;//time between frame to add and last recorded frame time stamp

			if (!frameTimeStamp.empty())
			{
				newTimeBetFrames = newFrameTime - frameTimeStamp[frameTimeStamp.size() - 1];

				//Remove frames until new frame fits inside the window
				while (timesSum + newTimeBetFrames >= timeBarrier && frameTimeStamp.size() > 0)
				{
					frameTimeStamp.erase(frameTimeStamp.begin());
					if (!timesBetweenPairOfFrames.empty())
					{
						timesSum -= timesBetweenPairOfFrames[0];
						timesBetweenPairOfFrames.erase(timesBetweenPairOfFrames.begin());
					}
					framesToRemove++;
				}
				//If the newTimeBetFrames is bigger than the time window, timeSum must be reset
				if (newTimeBetFrames >= timeBarrier)
				{
					timesSum = 0;
				}
			}

			AddNewFrame(newFrameTime, newTimeBetFrames);

			return framesToRemove;
		}

		/// <summary>
		/// Add the new frame time in the frameTimeStamp and the timesBetweenPairOfFrames vectors
		/// also update the timesSum (times between frames) variable.
		/// </summary>
		/// <param name="newTimeBetFrames:"> time between frame to add and last recorded frame time stamp </param>
		void AddNewFrame(const unsigned long& newFrameTime, const unsigned long& newTimeBetFrames)
		{
			if (!frameTimeStamp.empty())
			{
				timesSum += newTimeBetFrames;
				timesBetweenPairOfFrames.emplace_back(newTimeBetFrames);
			}
			frameTimeStamp.emplace_back(newFrameTime);
		}
		/// <summary>
		/// The current info is reset, the last frame will become the first one.
		/// </summary>
		void Reset() 
		{
			if (!frameTimeStamp.empty())
			{
				long lastFrameTime = frameTimeStamp[frameTimeStamp.size() - 1];
				frameTimeStamp.clear();
				timesBetweenPairOfFrames.clear();
				AddNewFrame(lastFrameTime, 0);
				timesSum = 0;
			}
		}
	private:
		//Time barrier measured in milliseconds
		//Used to know when timesBetweenPairOfFrames vector sum has surpased the desired second window and vectors needs to be updated
		float timeBarrier = 1000.0;
	};

	static inline FrameManager* instance = nullptr;

	iris::Configuration* m_config = nullptr;

	bool m_realTimeUse = false; //will be set to 'true' if the analysis by time is enabled in the 'appsettings.json' file

	//The vectors work in parallel, each element(Flash, PatternDetection, TransitionTracker...) has an index to access the data in the vectors.
	std::vector<FrameTimeStamps> m_frameTimeStamps; //Analysis by time
	std::vector<int> m_framesToRemove; //Analysis by time
	std::vector<int> m_frameCapacity; //Analysis by FPS/Time
};