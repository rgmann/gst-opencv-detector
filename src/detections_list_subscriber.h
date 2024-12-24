#ifndef __TCP_CONNECTION_H__
#define __TCP_CONNECTION_H__

#include <deque>
#include <memory>
#include <boost/asio.hpp>
#include "detections_list.h"
#include "message.h"

class detections_list_subscriber_manager;

class detections_list_subscriber : public std::enable_shared_from_this<detections_list_subscriber> {
public:

    explicit detections_list_subscriber(
        boost::asio::ip::tcp::socket socket,
        detections_list_subscriber_manager& manager);

    void publish(const message::ptr message);

    void start();

    void close();

private:

    void do_write();

private:

    /// Socket for the connection.
    boost::asio::ip::tcp::socket socket_;

    /// The manager for this connection.
    detections_list_subscriber_manager& subscriber_manager_;

    std::deque<message::ptr> messages_;
};

typedef std::shared_ptr<detections_list_subscriber> detections_list_subscriber_ptr;

#endif // __TCP_CONNECTION_H__