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

    detections_list_server(int port)
        : acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
        , runner_(&detections_list_server::run, this)
    {
        start_accept();
    }

    ~detections_list_server()
    {
        io_context_.stop();
    }

    void publish(const DetectionList& detections) {
        manager_.publish(detections);
    }

    void run() {
        io_context_.run();
    }

private:

    void start_accept()
    {
        acceptor_.async_accept([this](const boost::system::error_code& error, boost::asio::ip::tcp::socket new_connection) {
            if (!error) {
                std::make_shared<detections_list_subscriber>(std::move(new_connection), manager_)->start();
            }

            start_accept();
        });
    }

private:

    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::thread runner_;

    detections_list_subscriber_manager manager_;

};

#endif // __DETECTIONS_LIST_SERVER_H__