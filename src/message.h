#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <memory>
#include <cstring>
#include <cstdio>

class message {
public:

    typedef std::shared_ptr<message> ptr;

    static constexpr size_t HEADER_LENGTH = 4;
    static constexpr size_t MAX_BODY_LENGTH = 1024;

    message() : body_length_(0)
    {
        std::memset(data_, 0, sizeof(data_));
    }

    static message::ptr encode(const void* raw, size_t length)
    {
        message::ptr encoded_message;

        if (length < MAX_BODY_LENGTH)
        {
            encoded_message = std::make_shared<message>();

            encoded_message->body_length_ = length;

            char header[HEADER_LENGTH + 1] = "";
            std::snprintf(header, sizeof(header), "%4d", static_cast<int>(length));
            std::memcpy(encoded_message->data_, raw, HEADER_LENGTH);

            std::memcpy(encoded_message->data_ + HEADER_LENGTH, raw, length);
        }

        return encoded_message;
    }

    char* data()
    {
        return data_;
    }

    char* body()
    {
        return data() + HEADER_LENGTH;
    }

    size_t body_length()
    {
        return body_length_;
    }

    size_t size() {
        return body_length() + HEADER_LENGTH;
    }

private:

    char data_[HEADER_LENGTH + MAX_BODY_LENGTH];
    size_t body_length_;
};

#endif // __MESSAGE_H__