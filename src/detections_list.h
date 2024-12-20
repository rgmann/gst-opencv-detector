#ifndef __DETECTIONS_LIST_H__
#define __DETECTIONS_LIST_H__

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

struct Detection {

    // Classification returned by OpenCV 
    int class_id;

    // Class name associated with the ID
    std::string class_name;

    // Bounding rectangle
    cv::Rect box;

    // Classification confidence score
    float confidence;

    std::string to_string() const;
};

struct DetectionList {
    int width;
    int height;
    std::vector<Detection> detections;
};

#endif // __DETECTIONS_LIST_H__