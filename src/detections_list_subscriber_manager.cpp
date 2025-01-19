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

detections_list_subscriber_manager::detections_list_subscriber_manager(
    boost::asio::io_context& context,
    int port,
    size_t max_subscribers
)
    : acceptor_(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    , max_subscribers_(max_subscribers)
    , accepting_connections_(true)
{
    start_accept();
}

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

    if (subscribers_.size() >= max_subscribers_)
    {
        accepting_connections_ = false;
    }
}

void detections_list_subscriber_manager::leave(detections_list_subscriber_ptr subscriber)
{
    subscribers_.erase(subscriber);
    subscriber->close();

    if (!accepting_connections_)
    {
        accepting_connections_ = true;
        start_accept();
    }
}

void detections_list_subscriber_manager::stop_all()
{
    accepting_connections_ = false;
    subscribers_.clear();
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

    auto meta_info = gst_opencv_detector::Meta(
        detections_list.info.timestamp,
        detections_list.info.image_width,
        detections_list.info.image_height,
        detections_list.info.crop_width,
        detections_list.info.crop_height,
        detections_list.info.elapsed_time_ms
    );

    auto detection_list = gst_opencv_detector::CreateDetectionList(
        builder,
        &meta_info,
        detections_vector
    );

    builder.Finish(detection_list);

    return message::encode(builder.GetBufferPointer(), builder.GetSize());
}

void detections_list_subscriber_manager::start_accept()
{
    acceptor_.async_accept(
        [this](const boost::system::error_code& error, boost::asio::ip::tcp::socket new_connection)
        {
            if (!error)
            {
                std::make_shared<detections_list_subscriber>(std::move(new_connection), *this)->start();
            }

            if (accepting_connections_)
            {
                start_accept();
            }
        }
    );
}