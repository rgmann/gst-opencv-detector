#pragma once

#include <gst/gst.h>
#include <gst/video/video.h>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>

struct Detection {
    int class_id;
    std::string class_name;
    cv::Rect box;
    gfloat confidence;

    std::string to_string() const;
};

struct DetectionList {
    int width;
    int height;
    std::vector<Detection> detections;
};

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

    gboolean initialize(const gchar* config, const gchar* weights, const gchar* class_names);

    gboolean is_initialized() const;

    /**
     * Detects objects using loaded module and returns a list of detections.
     * 
     * @param image Input image. Image must be in BGR format.
     * @param detections List of Detections
     * @param annotate If true, image is annotated with a box arround each detection
     * @return gboolean  TRUE on success, FALSE on failure
     */
    gboolean get_objects(cv::Mat& image, std::vector<Detection>& detections, gboolean annotate);

private:

    gboolean parse_class_names(const gchar* filename, std::vector<std::string>& class_names) const;

    void annotate_detection(const Detection& detection, cv::Mat& image);

private:

    gboolean initialized_;

    std::unique_ptr<cv::dnn::DetectionModel> model_;

    std::vector<std::string> class_names_;
};
