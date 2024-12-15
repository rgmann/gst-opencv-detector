#include <fstream>
#include "object_detector.h"

ObjectDetector::ObjectDetector()
    : width(-1)
    , height(-1)
    , conf_threshold(0.45)
    , nms_threshold(0.2)
    , initialized_(FALSE)
{
}

gboolean ObjectDetector::initialize(const gchar* config_file, const gchar* weights_file, const gchar* class_names_file)
{
    gboolean success = FALSE;

    if (config_file && weights_file && class_names_file && (width > 0) && (height > 0))
    {
        g_print("Creating detection model.\n");

        success = parse_class_names(class_names_file, class_names_);

        if (success)
        {
            model_ = std::make_unique<cv::dnn::DetectionModel>(
                weights_file,
                config_file
            );

            cv::Size input_size(320, 320);
            // cv::Size input_size(width, height);
            model_->setInputSize(input_size);

            // cv::Scalar input_scale = 1.0 / 127.5;
            model_->setInputScale(1.0 / 127.5);

            cv::Scalar input_mean = 127.5;
            model_->setInputMean(input_mean);

            model_->setInputSwapRB(true);

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

gboolean ObjectDetector::get_objects(cv::Mat& image, std::vector<Detection>& detections, gboolean annotate)
{
    gboolean success = FALSE;

    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    if (is_initialized())
    {
        model_->detect(image, class_ids, confidences, boxes, conf_threshold, nms_threshold);

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

            // print_detection(detection);

            detections.push_back(detection);

            if (annotate)
            {
                annotate_detection(detection, image);
            }
        }

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
