#include <gst/gst.h>
#include <opencv2/opencv.hpp>

cv::Mat    gst_buffer_to_cv_mat(GstBuffer *buffer, gint width, gint height, gint channels);
GstBuffer* cv_mat_to_gst_buffer(const cv::Mat& mat);
