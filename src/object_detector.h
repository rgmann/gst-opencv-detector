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

#ifndef __OBJECT_DETECTOR_H__
#define __OBJECT_DETECTOR_H__

#include <gst/gst.h>
#include <gst/video/video.h>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>
#include "detections_list.h"


class ObjectDetector {
public:

    // Public attributes representing current caps
    gint width;
    gint height;
    GstVideoFormat format;

    float conf_threshold;
    float nms_threshold;

public:

    ObjectDetector();

    /**
     * Initialize the detector with the model definition, weights, and class names.
     * 
     * @param config Text file containing network configuration
     * @param weights Binary file containing trained weights
     * @return gboolean TRUE on success, FALSE on failure
     */
    gboolean initialize(const gchar* config, const gchar* weights, const gchar* class_names);

    /**
     * 
     */
    gboolean is_initialized() const;

    /**
     * Detects objects using loaded module and returns a list of detections.
     * 
     * @param image Input image. Image must be in BGR format.
     * @param detection_list List of Detections
     * @param annotate If true, image is annotated with a box arround each detection
     * @return gboolean  TRUE on success, FALSE on failure
     */
    gboolean get_objects(cv::Mat& image, DetectionList& detection_list, gboolean annotate);

private:

    gboolean parse_class_names(const gchar* filename, std::vector<std::string>& class_names) const;

    void annotate_detection(const Detection& detection, cv::Mat& image);

private:

    gboolean initialized_;

    std::unique_ptr<cv::dnn::DetectionModel> model_;

    std::vector<std::string> class_names_;
};

#endif // __OBJECT_DETECTOR_H__