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

#include <vector>
#include <chrono>
#include "generated/detections_list_generated.h"
#include "detections_list_subscriber_manager.h"

void detections_list_subscriber_manager::publish(const DetectionList& detections_list)
{
    message::ptr message = build_message(detections_list);

    for ( auto subscriber : subscribers_ )
    {
        subscriber->publish(message);
    }
}

void detections_list_subscriber_manager::join(detections_list_subscriber_ptr subscriber)
{
    subscribers_.insert(subscriber);
}

void detections_list_subscriber_manager::leave(detections_list_subscriber_ptr subscriber)
{
    subscribers_.erase(subscriber);
    subscriber->close();
}

void detections_list_subscriber_manager::stop_all()
{
    subscribers_.clear();
}

uint64_t detections_list_subscriber_manager::create_timestamp()
{
    using namespace std::chrono;

    // Get the current time point
    auto now = system_clock::now();

    // Convert to milliseconds since epoch
    auto millisecondsSinceEpoch = duration_cast<milliseconds>(now.time_since_epoch()).count();

    // Return as uint64_t
    return static_cast<uint64_t>(millisecondsSinceEpoch);
}

message::ptr detections_list_subscriber_manager::build_message(const DetectionList& detections_list)
{
    flatbuffers::FlatBufferBuilder builder(1024);

    std::vector<flatbuffers::Offset<gst_opencv_detector::Detection>> detections;
    for (auto detection : detections_list.detections)
    {
        auto box = gst_opencv_detector::Rect(
            detection.box.x,
            detection.box.y,
            detection.box.width,
            detection.box.height
        );

        detections.push_back(gst_opencv_detector::CreateDetection(
            builder,
            detection.class_id,
            builder.CreateString(detection.class_name),
            &box,
            detection.confidence
        ));
    }

    auto detections_vector = builder.CreateVector(detections);

    auto detection_list = gst_opencv_detector::CreateDetectionList(
        builder,
        create_timestamp(),
        detections_list.width,
        detections_list.height,
        detections_vector
    );

    builder.Finish(detection_list);

    return message::encode(builder.GetBufferPointer(), builder.GetSize());
}