#ifndef __DETECTIONS_LIST_SERVER_H__
#define __DETECTIONS_LIST_SERVER_H__

#include <boost/asio.hpp>
#include "detection_list.h"
#include "generated/detections_list_generated.h"

class tcp_server {
public:

    tcp_server(boost::asio::io_context& io_context)
        : io_context_(io_context)
        , acceptor_(io_context, tcp::endpoint(tcp::v4(), 9000))
    {
        start_accept();
    }

private:

    void start_accept()
    {
        // tcp_connection::pointer new_connection = tcp_connection::create(io_context_);

        // acceptor_.async_accept(new_connection->socket(),
        //     std::bind(&tcp_server::handle_accept, this, new_connection,
        //     boost::asio::placeholders::error));
    }

    // void handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error)
    // {
    //     if (!error)
    //     {
    //         new_connection->start();
    //     }

    //     start_accept();
    // }

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};

#endif // __DETECTIONS_LIST_SERVER_H__