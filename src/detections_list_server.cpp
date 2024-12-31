#include "detections_list_server.h"

detections_list_server::detections_list_server(int port)
    : acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    , runner_(&detections_list_server::run, this)
{
    start_accept();
}

detections_list_server::~detections_list_server()
{
    io_context_.stop();
    runner_.join();
}

void detections_list_server::publish(const DetectionList& detections)
{
    manager_.publish(detections);
}

void detections_list_server::run()
{
    auto work = boost::asio::make_work_guard(io_context_);
    io_context_.run();
}

void detections_list_server::start_accept()
{
    acceptor_.async_accept([this](const boost::system::error_code& error, boost::asio::ip::tcp::socket new_connection) {
        if (!error) {
            std::make_shared<detections_list_subscriber>(std::move(new_connection), manager_)->start();
        }

        start_accept();
    });
}