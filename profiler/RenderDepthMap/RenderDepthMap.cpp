﻿/*******************************************************************************
 *BSD 3-Clause License
 *
 *Copyright (c) 2016-2025, Mech-Mind Robotics
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *
 *1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *2. Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 *3. Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

/*
With this sample, you can obtain and save the depth map rendered with the jet color scheme.
*/

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <thread>
#include <cstdio>
#include <cmath>
#include <chrono>
#include <mutex>
#include "profiler/Profiler.h"
#include "profiler/api_util.h"
#include "profiler/parameters/ProfileProcessingParameters.h"
#include "profiler/parameters/ProfileExtractionParameters.h"
#include "profiler/parameters/RawImageParameters.h"
#include "profiler/parameters/ScanParameters.h"

namespace {
std::mutex kMutex;

// Define the callback function for retrieving the profile data
void callbackFunc(const mmind::eye::ProfileBatch& batch, void* pUser)
{
    std::unique_lock<std::mutex> lock(kMutex);
    if (!batch.getErrorStatus().isOK()) {
        std::cout << "Error occurred during data acquisition." << std::endl;
        showError(batch.getErrorStatus());
    }
    auto* outPutBatch = static_cast<mmind::eye::ProfileBatch*>(pUser);
    outPutBatch->append(batch);
}

bool acquireProfileDataUsingCallback(mmind::eye::Profiler& profiler,
                                     mmind::eye::ProfileBatch& profileBatch, bool isSoftwareTrigger)
{
    profileBatch.clear();

    // Set a large value for CallbackRetrievalTimeout
    showError(profiler.currentUserSet().setIntValue(
        mmind::eye::scan_settings::CallbackRetrievalTimeout::name, 60000));

    // Register the callback function
    auto status = profiler.registerAcquisitionCallback(callbackFunc, &profileBatch);
    if (!status.isOK()) {
        showError(status);
        return false;
    }

    // Call startAcquisition() to enter the laser profiler into the acquisition ready status
    std::cout << "Start data acquisition." << std::endl;
    status = profiler.startAcquisition();
    if (!status.isOK()) {
        showError(status);
        return false;
    }

    // Call triggerSoftware() to start the data acquisition
    if (isSoftwareTrigger) {
        status = profiler.triggerSoftware();
        if (!status.isOK()) {
            showError(status);
            return false;
        }
    }

    while (true) {
        std::unique_lock<std::mutex> lock(kMutex);
        if (profileBatch.isEmpty()) {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } else
            break;
    }

    std::cout << "Stop data acquisition." << std::endl;
    status = profiler.stopAcquisition();
    if (!status.isOK())
        showError(status);
    return status.isOK();
}

void saveDepthMap(const mmind::eye::ProfileBatch& batch, int lineCount, int width,
                  const std::string& path)
{
    if (batch.isEmpty()) {
        std::cout
            << "The depth map cannot be saved because the batch does not contain any profile data."
            << std::endl;
        return;
    }
    cv::imwrite(path, cv::Mat(lineCount, width, CV_32FC1, batch.getDepthMap().data()));
}

inline bool isApprox0(double d) { return std::fabs(d) <= DBL_EPSILON; }

cv::Mat renderDepthData(const cv::Mat& depth)
{
    if (depth.empty())
        return cv::Mat();

    cv::Mat mask = cv::Mat(depth == depth);
    double minDepthValue, maxDepthValue;
    cv::minMaxLoc(depth, &minDepthValue, &maxDepthValue, nullptr, nullptr, mask);

    cv::Mat depth8U;
    isApprox0(maxDepthValue - minDepthValue)
        ? depth.convertTo(depth8U, CV_8UC1)
        : depth.convertTo(depth8U, CV_8UC1, (255.0 / (minDepthValue - maxDepthValue)),
                          (((maxDepthValue * 255.0) / (maxDepthValue - minDepthValue)) + 1));

    if (depth8U.empty())
        return cv::Mat();

    cv::Mat coloredDepth;
    cv::applyColorMap(depth8U, coloredDepth, cv::COLORMAP_JET);
    coloredDepth.forEach<cv::Vec3b>([&](auto& val, const int* pos) {
        if (!depth8U.ptr<uchar>(pos[0])[pos[1]]) {
            val[0] = 0;
            val[1] = 0;
            val[2] = 0;
        }
    });
    return coloredDepth;
}

void saveRenderedDepthMap(const mmind::eye::ProfileBatch& batch, int lineCount, int width,
                          const std::string& path)
{
    if (batch.isEmpty()) {
        std::cout
            << "The depth map cannot be saved because the batch does not contain any profile data."
            << std::endl;
        return;
    }

    cv::Mat depth = cv::Mat(lineCount, width, CV_32FC1, batch.getDepthMap().data());
    cv::Mat renderedDepth = renderDepthData(depth);

    cv::imwrite(path, renderedDepth);
}

void saveIntensityImage(const mmind::eye::ProfileBatch& batch, int lineCount, int width,
                        const std::string& path)
{
    if (batch.isEmpty()) {
        std::cout << "The intensity image cannot be saved because the batch does not contain any "
                     "profile data."
                  << std::endl;
        return;
    }
    cv::imwrite(path, cv::Mat(lineCount, width, CV_8UC1, batch.getIntensityImage().data()));
}
} // namespace

int main()
{
    mmind::eye::Profiler profiler;
    if (!findAndConnect(profiler))
        return -1;

    if (!confirmCapture()) {
        profiler.disconnect();
        return -1;
    }

    mmind::eye::UserSet userSet = profiler.currentUserSet();

    int dataWidth = 0;
    // Get the number of data points in each profile
    showError(
        userSet.getIntValue(mmind::eye::scan_settings::DataPointsPerProfile::name, dataWidth));
    int captureLineCount = 0;
    // Get the current value of the "Scan Line Count" parameter
    userSet.getIntValue(mmind::eye::scan_settings::ScanLineCount::name, captureLineCount);

    // Define a ProfileBatch object to store the profile data
    mmind::eye::ProfileBatch profileBatch(dataWidth);

    int dataAcquisitionTriggerSource{};
    showError(userSet.getEnumValue(mmind::eye::trigger_settings::DataAcquisitionTriggerSource::name,
                                   dataAcquisitionTriggerSource));
    bool isSoftwareTrigger =
        dataAcquisitionTriggerSource ==
        static_cast<int>(
            mmind::eye::trigger_settings::DataAcquisitionTriggerSource::Value::Software);

    // Acquire the profile data using the callback function
    if (!acquireProfileDataUsingCallback(profiler, profileBatch, isSoftwareTrigger))
        return -1;

    std::cout << "Save the depth map and intensity image." << std::endl;
    saveDepthMap(profileBatch, captureLineCount, dataWidth, "DepthMap.tiff");
    saveRenderedDepthMap(profileBatch, captureLineCount, dataWidth, "RenderedDepthMap.tiff");
    saveIntensityImage(profileBatch, captureLineCount, dataWidth, "IntensityImage.png");

    // Disconnect from the laser profiler
    profiler.disconnect();
    return 0;
}
