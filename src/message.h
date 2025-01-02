/*
 * GstOpencvDetector Utils
 * Copyright (C) 2024 Robert Vaughan <<robert.glissmann@gmail.com>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

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
            std::memcpy(encoded_message->data_, header, HEADER_LENGTH);

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