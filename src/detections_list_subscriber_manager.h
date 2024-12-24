#ifndef __DETECTIONS_LIST_SUBSCRIBER_MANAGER_H__
#define __DETECTIONS_LIST_SUBSCRIBER_MANAGER_H__

#include <set>
#include <cstdint>
#include "detections_list.h"
#include "detections_list_subscriber.h"

class detections_list_subscriber_manager {
public:

    // detections_list_subscriber_manager();

    // detections_list_subscriber_manager(const detections_list_subscriber_manager&) = delete;
    // detections_list_subscriber_manager& operator=(const tcp_connection&) = delete;

    void publish(const DetectionList& detection_list);

    void join(detections_list_subscriber_ptr subscriber);

    void leave(detections_list_subscriber_ptr subscriber);

    void stop_all();

private:

    static uint64_t create_timestamp();

    static message::ptr build_message(const DetectionList& detection_list);

private:

    std::set<detections_list_subscriber_ptr> subscribers_;
};

#endif // __DETECTIONS_LIST_SUBSCRIBER_MANAGER_H__