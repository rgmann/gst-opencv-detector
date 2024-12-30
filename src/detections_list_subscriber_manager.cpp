#include <vector>
#include <chrono>
#include "generated/detections_list_generated.h"
#include "detections_list_subscriber_manager.h"

void detections_list_subscriber_manager::publish(const DetectionList& detections_list)
{
    message::ptr message = build_message(detections_list);

    for ( auto subscriber : subscribers_ ) {
        subscriber->publish(message);
    }
}

void detections_list_subscriber_manager::join(detections_list_subscriber_ptr subscriber) {
    subscribers_.insert(subscriber);
}

void detections_list_subscriber_manager::leave(detections_list_subscriber_ptr subscriber) {
    subscribers_.erase(subscriber);
    subscriber->close();
}

void detections_list_subscriber_manager::stop_all()
{
    subscribers_.clear();
}

uint64_t detections_list_subscriber_manager::create_timestamp() {
    using namespace std::chrono;

    // Get the current time point
    auto now = system_clock::now();

    // Convert to milliseconds since epoch
    auto millisecondsSinceEpoch = duration_cast<milliseconds>(now.time_since_epoch()).count();

    // Return as uint64_t
    return static_cast<uint64_t>(millisecondsSinceEpoch);
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

    auto detection_list = gst_opencv_detector::CreateDetectionList(
        builder,
        create_timestamp(),
        detections_list.width,
        detections_list.height,
        detections_vector
    );

    builder.Finish(detection_list);

    return message::encode(builder.GetBufferPointer(), builder.GetSize());
}