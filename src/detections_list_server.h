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

    // Maximum number of connections that the server can accept
    static constexpr size_t DEFAULT_MAX_SUBCRIBERS = 5;

    /**
     * Constructor
     *
     * @param port Connection endpoint port number
     * @param max_subscribers Maximum number of subscribers that may be in the pool at any point in time
     */
    detections_list_server(int port, size_t max_subscribers = DEFAULT_MAX_SUBCRIBERS);

    /**
     * The destructor stops the asio context and waits for the runner thread to exit
     */
    ~detections_list_server();

    /**
     * Publish a list of detections to all subscribed clients. This is a no-op
     * if there are no subscribers. Caller is never blocked.
     *
     * @param detections List of detections
     * @return void
     */
    void publish(const DetectionList& detections);

    /**
     * Server thread entry point.
     *
     * @return void
     */
    void run();


private:

    boost::asio::io_context io_context_;

    // Subscription manager
    detections_list_subscriber_manager manager_;

    // The server runs the asio event loop with a separate thread context
    std::thread runner_;

};

#endif // __DETECTIONS_LIST_SERVER_H__