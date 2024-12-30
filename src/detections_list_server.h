#ifndef __DETECTIONS_LIST_SERVER_H__
#define __DETECTIONS_LIST_SERVER_H__

#include <deque>
#include <thread>
#include <boost/asio.hpp>
#include "detections_list.h"
#include "generated/detections_list_generated.h"
#include "detections_list_subscriber_manager.h"


class detections_list_server {
public:

    detections_list_server(int port);
    ~detections_list_server();

    void publish(const DetectionList& detections);

    void run();

private:

    void start_accept();

private:

    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::thread runner_;

    detections_list_subscriber_manager manager_;

};

#endif // __DETECTIONS_LIST_SERVER_H__