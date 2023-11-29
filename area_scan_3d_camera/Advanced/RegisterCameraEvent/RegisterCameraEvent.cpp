﻿/*******************************************************************************
 *BSD 3-Clause License
 *
 *Copyright (c) 2016-2023, Mech-Mind Robotics
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
With this sample, you can define and register the callback function for monitoring the camera connection status.
*/

#include <thread>
#include <chrono>
#include "area_scan_3d_camera/CameraEvent.h"
#include "area_scan_3d_camera/Camera.h"
#include "area_scan_3d_camera/api_util.h"

int main()
{
    mmind::eye::Camera camera;
    if (!findAndConnect(camera))
        return -1;

    // Define the callback function of event
    mmind::eye::CameraEvent::EventCallback callback = [](mmind::eye::CameraEvent::Event event,
                                                         void* pUser) {
        std::cout << "A camera event has occurred. The event ID is " << event << "." << std::endl;
    };

    // Set the heartbeat interval to 2 seconds
    camera.setHeartbeatInterval(2000);
    std::cout << "Register the callback function for camera disconnection events." << std::endl;
    // Register the callback function, and the type of event is CAMERA_EVENT_DISCONNECTED
    showError(mmind::eye::CameraEvent::registerCameraEventCallback(
        camera, callback, nullptr, mmind::eye::CameraEvent::CAMERA_EVENT_DISCONNECTED));
    // Let the program sleep for 20 seconds. During this period, if the camera is disconnected, the
    // callback function will detect and report the disconnection.
    std::this_thread::sleep_for(std::chrono::milliseconds(20000));

    camera.disconnect();
    std::cout << "Disconnected from the camera successfully." << std::endl;
    return 0;
}
