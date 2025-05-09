/*******************************************************************************
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
With this sample, you can set the "Depth Range" parameter.
*/

#include "area_scan_3d_camera/parameters/Scanning3D.h"
#include "area_scan_3d_camera/Camera.h"
#include "area_scan_3d_camera/api_util.h"

int main()
{
    mmind::eye::Camera camera;
    if (!findAndConnect(camera))
        return -1;

    mmind::eye::UserSet& currentUserSet = camera.currentUserSet();

    auto* parameter = currentUserSet.getParameter(mmind::eye::scanning3d_setting::DepthRange::name);
    if (!parameter)
        return -1;
    auto* depthRange = dynamic_cast<mmind::eye::RangeParameter*>(parameter);
    if (!depthRange)
        return -1;

    // Set the range of depth values to 100–1000 mm.
    mmind::eye::Range<int> range(100, 1000);
    showError(depthRange->setValue(range));
    mmind::eye::Range<int> curDepthRange;
    showError(depthRange->getValue(curDepthRange));
    std::cout << "Depth lower limit: " << curDepthRange.min
              << " mm, depth upper limit: " << curDepthRange.max << " mm." << std::endl;

    camera.disconnect();
    std::cout << "Disconnected from the camera successfully." << std::endl;
    return 0;
}
