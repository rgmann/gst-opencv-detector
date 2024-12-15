/*
 * GstOpenCVServer Utils
 * Copyright (C) 2024 Robert Vaughan <<user@hostname.org>>
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

#include <gst/gst.h>
#include <opencv2/opencv.hpp>
#include <gst/video/video.h>

/**
 * Helper class to create a buffer map scope that is automatically destroyed
 * when the object goes out of scope.
 */
class ScopedBufferMap {
public:

    /**
     * Creates a scoped buffer info map from the supplied buffer and attributes.
     * @param buffer Input buffer
     * @param width Width of image represented in buffer
     * @param height Height of image represented in buffer
     * @param format Format of the image represented in buffer
     */
    ScopedBufferMap(GstBuffer* buffer, gint width, gint height, GstVideoFormat format);

    /**
     * Destructor
     */
    ~ScopedBufferMap();

    /**
     * Reference to OpenCV matrix.
     * 
     * @return OpenCV matrix
     */
    cv::Mat& frame();


private:

    GstBuffer* buffer_;
    GstMapInfo map_;
    cv::Mat    frame_;
};

/**
 * Utility for checking whether a filepath is valid (and points to a regular file).
 * 
 * @param path File path
 * @return TRUE if filepath exists and points to a regular file, FALSE otherwise
 */
gboolean valid_file_path(const gchar* path);

/**
 * Utility method for creating a new GstBuffer from a cv::Mat
 * 
 * @param mat Image matrix
 * @return GstBuffer (caller is responsible for freeing)
 */
GstBuffer* cv_mat_to_gst_buffer(const cv::Mat& mat);
