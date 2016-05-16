/*
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-protocol.
 *
 * libbitcoin-protocol is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) 
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <bitcoin/protocol/zmq/poller.hpp>

#include <cstdint>
#include <czmq.h>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/protocol/zmq/socket.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

poller::poller()
  : expired_(false),
    terminated_(false)
{
}

poller::~poller()
{
}

void poller::add(socket& socket)
{
    zmq_pollitem_t item;

    // zmq socket.
    item.socket = socket.self();

    // non-zmq socket (unused when socket is set).
    item.fd = 0;

    // flags.
    item.events = ZMQ_POLLIN;

    // return events.
    item.revents = 0;

    pollers_.push_back(item);
}

socket poller::wait(int timeout)
{
    const auto size = pollers_.size();
    BITCOIN_ASSERT(size <= max_int32);

    const auto size32 = static_cast<int32_t>(size);
    auto signaled = zmq_poll(pollers_.data(), size32, timeout);

    if (signaled < 0)
    {
        terminated_ = true;
        return nullptr;
    }

    if (signaled == 0)
    {
        expired_ = true;
        return nullptr;
    }

    for (const auto& poller: pollers_)
        if ((poller.revents & ZMQ_POLLIN) != 0)
            return poller.socket;

    return nullptr;
}

bool poller::expired() const
{
    return expired_;
}

bool poller::terminated() const
{
    return terminated_;
}

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin
