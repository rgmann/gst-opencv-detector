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

#ifndef __DETECTIONS_LIST_SUBSCRIBER_H__
#define __DETECTIONS_LIST_SUBSCRIBER_H__

#include <deque>
#include <memory>
#include <boost/asio.hpp>
#include "detections_list.h"
#include "message.h"

class detections_list_subscriber_manager;

class detections_list_subscriber : public std::enable_shared_from_this<detections_list_subscriber> {
public:

    /**
     * Constructor
     *
     * @param socket TCP socket for accepted connection
     * @param manager Reference to the subscription pool manager
     */
    explicit detections_list_subscriber(
        boost::asio::ip::tcp::socket socket,
        detections_list_subscriber_manager& manager);

    /**
     * Copying is not permitted
     */
    detections_list_subscriber(const detections_list_subscriber&) = delete;
    detections_list_subscriber& operator= (const detections_list_subscriber&) = delete;

    /**
     * Add message to the subscriber's transmission queue.
     *
     * @param message Message to send
     * @return void
     */
    void publish(const message::ptr message);

    /**
     * Join the subscription pool to begin receiving detection lists.
     *
     * @return void
     */
    void start();

    /**
     * Leave the pool and close the socket.
     *
     * @return void
     */
    void close();

private:

    /**
     * Start an asynchronous write operation.
     *
     * @return void
     */
    void start_write();


private:

    /// Socket for the connection.
    boost::asio::ip::tcp::socket socket_;

    /// The manager for this connection.
    detections_list_subscriber_manager& subscriber_manager_;

    std::deque<message::ptr> messages_;
};

typedef std::shared_ptr<detections_list_subscriber> detections_list_subscriber_ptr;

#endif // __DETECTIONS_LIST_SUBSCRIBER_H__