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

#ifndef __DETECTIONS_LIST_SUBSCRIBER_MANAGER_H__
#define __DETECTIONS_LIST_SUBSCRIBER_MANAGER_H__

#include <set>
#include <cstdint>
#include <boost/asio.hpp>
#include "detections_list.h"
#include "detections_list_subscriber.h"

class detections_list_subscriber_manager {
public:

    /**
     * Constructor
     *
     * @param context Async IO context
     * @param port Connection endpoint port number
     * @param max_subscribers Maximum number of subscribers that may be in the pool at any point in time
     */
    detections_list_subscriber_manager(
        boost::asio::io_context& context,
        int port,
        size_t max_subscribers);

    /**
     * Copying is not permitted
     */
    detections_list_subscriber_manager(const detections_list_subscriber_manager&) = delete;
    detections_list_subscriber_manager& operator= (const detections_list_subscriber_manager&) = delete;

    /**
     * Publish a list of detections to all subscribed clients. This is a no-op
     * if there are no subscribers. Caller is never blocked.
     *
     * @param detection_list List of detections
     * @return void
     */
    void publish(const DetectionList& detection_list);

    /**
     * Add the subscriber to the subscription pool.
     *
     * @param subscriber Shared pointer to subscriber instance
     * @return void
     */
    void join(detections_list_subscriber_ptr subscriber);

    /**
     * Remove the subscriber from the subscription pool. If the pool was
     * full before subscriber was removed, new subscriber acceptance will
     * be triggered once the subscriber has been removed.
     *
     * @param subscriber Shared pointer to subscriber instance
     * @return void
     */
    void leave(detections_list_subscriber_ptr subscriber);

    /**
     * Stop and remove all subscribers from the pool.
     */
    void stop_all();


private:

    /**
     * Initiate asynchronous socket acceptor.
     * 
     * @return void
     */
    void start_accept();

    /**
     * Build a transmittable message from the specified detection list.
     *
     * @param detection_list Detection list to pack
     * @return Shared pointer to transmittable message
     */
    static message::ptr build_message(const DetectionList& detection_list);


private:

    boost::asio::ip::tcp::acceptor acceptor_;

    size_t max_subscribers_;

    std::set<detections_list_subscriber_ptr> subscribers_;

    bool accepting_connections_;
};

#endif // __DETECTIONS_LIST_SUBSCRIBER_MANAGER_H__