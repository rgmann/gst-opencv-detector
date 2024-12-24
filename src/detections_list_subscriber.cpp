#include "detections_list_subscriber_manager.h"
#include "detections_list_subscriber.h"

detections_list_subscriber::detections_list_subscriber(
    boost::asio::ip::tcp::socket socket,
    detections_list_subscriber_manager& manager
)
    : socket_(std::move(socket))
    , subscriber_manager_(manager)
{
}

void detections_list_subscriber::start()
{
    subscriber_manager_.join(shared_from_this());
}

void detections_list_subscriber::publish(const message::ptr message)
{
    bool write_in_progress = !messages_.empty();
    messages_.push_back(message);

    if (!write_in_progress)
    {
        do_write();
    }
}

void detections_list_subscriber::close()
{
    socket_.close();
}

void detections_list_subscriber::do_write()
{
    auto self(shared_from_this());

    if (!messages_.empty())
    {
        message::ptr front = messages_.front();

        boost::asio::async_write(
            socket_,
            boost::asio::buffer(front->data(), front->size()),
            [this, self](const boost::system::error_code& error, size_t bytes_written)
            {
                (void)bytes_written;

                if (!error)
                {
                    messages_.pop_front();

                    if (!messages_.empty())
                    {
                        do_write();
                    }
                }
                else
                {
                    subscriber_manager_.leave(shared_from_this());
                }
            }
        );
    }
}