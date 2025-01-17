/*
 * GstOpencvDetector Utils
 * Copyright (C) 2024 Robert Vaughan <<robert.glissmann@gmail.com>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __DETECTIONS_LIST_H__
#define __DETECTIONS_LIST_H__

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

struct Detection {

    // Classification returned by OpenCV 
    int class_id = 0;

    // Class name associated with the ID
    std::string class_name;

    // Bounding rectangle
    cv::Rect box;

    // Classification confidence score
    float confidence = 0.0;
};

struct MetaInfo {
    uint64_t timestamp = 0;

    // Image width
    uint32_t image_width = 0;

    // Image height
    uint32_t image_height = 0;

    // Image width
    uint32_t crop_width = 0;

    // Image height
    uint32_t crop_height = 0;

    // Elapsed time to perform detection in milliseconds
    uint32_t elapsed_time_ms = 0;
};

struct DetectionList {

    MetaInfo info;

    // List of detections associated with this image
    std::vector<Detection> detections;
};

#endif // __DETECTIONS_LIST_H__