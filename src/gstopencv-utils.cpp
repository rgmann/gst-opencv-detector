#include <map>
#include <filesystem>
#include "gstopencv-utils.h"

static const std::map<GstVideoFormat,int> FORMAT_TO_CHANNELS = {
    { GST_VIDEO_FORMAT_BGR, 3 },
    { GST_VIDEO_FORMAT_UYVY, 2 },
    { GST_VIDEO_FORMAT_VYUY, 2 },
    { GST_VIDEO_FORMAT_BGRx, 4 },
    { GST_VIDEO_FORMAT_RGBx, 4 },
    { GST_VIDEO_FORMAT_RGB, 3 },
    { GST_VIDEO_FORMAT_RGB16, 3 },
    { GST_VIDEO_FORMAT_YUY2, 3 }
};

ScopedBufferMap::ScopedBufferMap(GstBuffer* buffer, gint width, gint height, GstVideoFormat format)
: buffer_(buffer)
{
    gst_buffer_map(buffer_, &map_, GST_MAP_READ);

    int num_channels = -1;
    
    if (FORMAT_TO_CHANNELS.count(format))
    {
        num_channels = FORMAT_TO_CHANNELS.at(format);
        cv::Mat temp(cv::Size(width, height), CV_8UC(num_channels), map_.data, cv::Mat::AUTO_STEP);

        if (format == GST_VIDEO_FORMAT_BGR)
        {
            frame_ = temp.clone();
        }
        else
        {
            // cv:cvtColor(temp, frame_, )
        }
    }
}

cv::Mat& ScopedBufferMap::frame()
{
    return frame_;
}

ScopedBufferMap::~ScopedBufferMap()
{
    gst_buffer_unmap(buffer_, &map_);
}

gboolean valid_file_path(const gchar* path)
{
    if (!path) return FALSE;

    std::error_code error;
    std::filesystem::file_status status = std::filesystem::status(path, error);

    if (!error)
    {
        return std::filesystem::is_regular_file(status) ? TRUE : FALSE;
    }

    return FALSE;
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
