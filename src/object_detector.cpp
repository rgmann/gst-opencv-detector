/*
 * OpenCV Detector Plugin
 * Copyright (C) 2024 Robert Vaughan <robert.glissmann@gmail.com>
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

#include <fstream>
#include "object_detector.h"

class Timer {
public:

    Timer(bool auto_start = true)
        : started_(false)
        , stopped_(false)
    {
        if (auto_start) start();
    }

    void start()
    {
        start_ = std::chrono::system_clock::now();
        started_ = true;
    }

    void stop()
    {
        end_ = std::chrono::system_clock::now();
        stopped_ = true;
    }

    uint64_t elapsed_ms(bool auto_stop = true)
    {
        using namespace std::chrono;

        if (auto_stop && !stopped_) stop();

        if (started_ && stopped_)
        {
            std::chrono::duration<double> elapsed = end_ - start_;
            auto elapsed_ms = duration_cast<milliseconds>(elapsed).count();
            return static_cast<uint64_t>(elapsed_ms);
        }

        return 0;
    }


private:

    std::chrono::time_point<std::chrono::system_clock> start_;
    bool started_;

    std::chrono::time_point<std::chrono::system_clock> end_;
    bool stopped_;

};

ObjectDetector::ObjectDetector()
    : initialized_(FALSE)
    , crop_size_(cv::Size(ObjectDetector::kDefaultCropWidth, ObjectDetector::kDefaultCropHeight))
    , conf_threshold_(ObjectDetector::kDefaultConfidenceThreshold)
    , nms_threshold_(ObjectDetector::kDefaultNmsThreshold)
    , annotation_enabled_(false)
{
}

gboolean ObjectDetector::initialize(
    const gchar* config_file,
    const gchar* weights_file, 
    const gchar* class_names_file,
    float        conf_threshold,
    float        nms_threshold,
    cv::Size     crop,
    float        input_scale,
    float        input_mean,
    bool         swap_rb)
{
    gboolean success = FALSE;

    if (config_file && weights_file && class_names_file)
    {
        g_print("Creating detection model.\n");

        success = parse_class_names(class_names_file, class_names_);

        if (success)
        {
            model_ = std::make_unique<cv::dnn::DetectionModel>(
                weights_file,
                config_file
            );

            conf_threshold_ = conf_threshold;
            nms_threshold_ = nms_threshold;

            // Override the input size. If the image size and the input size
            // are not equal, OpenCV performs a crop from the center of the
            // input image.
            crop_size_ = crop;
            model_->setInputSize(crop);

            model_->setInputScale(input_scale);
            model_->setInputMean(input_mean);
            model_->setInputSwapRB(swap_rb);

            initialized_ = TRUE;
        }
        else
        {
            success = false;
        }
    }

    return success;
}

gboolean ObjectDetector::is_initialized() const
{
    return initialized_;
}

void ObjectDetector::set_annotate(bool enable_annotation)
{
    annotation_enabled_ = enable_annotation;
}

gboolean ObjectDetector::parse_class_names(const gchar* filename, std::vector<std::string>& class_names) const
{
    std::ifstream file(filename);

    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            class_names.push_back(line);
        }

        return true;
    }
    else
    {
        return false;
    }
}

uint64_t ObjectDetector::create_timestamp()
{
    using namespace std::chrono;

    // Get the current time point
    auto now = system_clock::now();

    // Convert to milliseconds since epoch
    auto millisecondsSinceEpoch = duration_cast<milliseconds>(now.time_since_epoch()).count();

    // Return as uint64_t
    return static_cast<uint64_t>(millisecondsSinceEpoch);
}

gboolean ObjectDetector::get_objects(cv::Mat& image, DetectionList& detection_list)
{
    gboolean success = FALSE;

    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    if (is_initialized())
    {
        Timer timer;

        detection_list.info.timestamp = create_timestamp();
        detection_list.info.image_width = image.size().width;
        detection_list.info.image_height = image.size().height;
        detection_list.info.crop_width = crop_size_.width;
        detection_list.info.crop_height = crop_size_.height;
        
        model_->detect(image, class_ids, confidences, boxes, conf_threshold_, nms_threshold_);

        detection_list.detections.reserve(class_ids.size());

        for (std::size_t index = 0; index < class_ids.size(); ++index)
        {
            Detection detection;

            detection.class_id = class_ids[index];

            int class_name_index = detection.class_id - 1;
            if ((class_name_index >= 0) && (class_name_index < static_cast<int>(class_names_.size())))
            {
                detection.class_name = class_names_[static_cast<std::size_t>(class_name_index)];
            }

            detection.box = boxes[index];
            detection.confidence = confidences[index];

            detection_list.detections.push_back(detection);

            if (annotation_enabled_)
            {
                annotate_detection(detection, image);
            }
        }

        timer.stop();
        detection_list.info.elapsed_time_ms = timer.elapsed_ms();

        success = TRUE;
    }

    return success;
}

void ObjectDetector::annotate_detection(const Detection& detection, cv::Mat& image)
{
    static const int thickness = 2;
    cv::Scalar color(0, 255, 0);

    cv::rectangle(image, detection.box, color, thickness);

    cv::Point class_name_location(detection.box.x + 10, detection.box.y + 30);
    cv::putText(image, detection.class_name, class_name_location, cv::FONT_HERSHEY_COMPLEX, 1, color);

    cv::Point confidence_location(detection.box.x + 200, detection.box.y + 30);
    cv::putText(image, std::to_string(detection.confidence), confidence_location, cv::FONT_HERSHEY_COMPLEX, 1, color);
}
