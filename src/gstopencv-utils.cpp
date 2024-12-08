#include "gstopencv-utils.h"

cv::Mat gst_buffer_to_cv_mat(GstBuffer* buffer, gint width, gint height, gint channels) {
    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_READ);

    // Create OpenCV Mat
    cv::Mat frame(cv::Size(width, height), CV_8UC(channels), map.data, cv::Mat::AUTO_STEP);

    // Unmap the buffer
    gst_buffer_unmap(buffer, &map);

    return frame;
}

GstBuffer* cv_mat_to_gst_buffer(const cv::Mat& mat)
{
    GstBuffer* buffer = gst_buffer_new_allocate(NULL, mat.total() * mat.elemSize(), NULL);
    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_WRITE);
    memcpy(map.data, mat.data, mat.total() * mat.elemSize());
    gst_buffer_unmap(buffer, &map);

    return buffer;
}