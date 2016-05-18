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
#include <bitcoin/protocol/zmq/frame.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <zmq.h>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/protocol/zmq/socket.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

static constexpr auto zmq_fail = -1;

// Use for receiving.
frame::frame()
  : more_(false), valid_(initialize(message_, {}))
{
}

// Use for sending.
frame::frame(const data_chunk& data)
  : more_(false), valid_(initialize(message_, data))
{
}

frame::~frame()
{
    DEBUG_ONLY(const auto result =) destroy();
    BITCOIN_ASSERT(result);
}

// static
bool frame::initialize(zmq_msg& message, const data_chunk& data)
{
    const auto buffer = reinterpret_cast<zmq_msg_t*>(&message);

    if (data.empty())
        return (zmq_msg_init(buffer) != zmq_fail);

    if (zmq_msg_init_size(buffer, data.size()) == zmq_fail)
        return false;

    std::memcpy(zmq_msg_data(buffer), data.data(), data.size());
    return true;
}

frame::operator const bool() const
{
    return valid_;
}

bool frame::more() const
{
    return more_;
}

// private
bool frame::set_more(socket& socket)
{
    int more;
    auto length = static_cast<size_t>(sizeof(int));

    if (zmq_getsockopt(socket.self(), ZMQ_RCVMORE, &more, &length) == zmq_fail)
        return false;

    more_ = (more != 0);
    return true;
}

data_chunk frame::payload()
{
    const auto buffer = reinterpret_cast<zmq_msg_t*>(&message_);
    const auto size = zmq_msg_size(buffer);
    const auto data = zmq_msg_data(buffer);
    const auto begin = static_cast<uint8_t*>(data);
    return{ begin, begin + size };
}

bool frame::receive(socket& socket)
{
    if (!valid_)
        return false;

    const auto buffer = reinterpret_cast<zmq_msg_t*>(&message_);
    return zmq_recvmsg(socket.self(), buffer, 0) != zmq_fail &&
        set_more(socket);
}

bool frame::send(socket& socket, bool last)
{
    if (!valid_)
        return false;

    const int flags = last ? 0 : ZMQ_SNDMORE;
    const auto buffer = reinterpret_cast<zmq_msg_t*>(&message_);
    return zmq_sendmsg(socket.self(), buffer, flags) != zmq_fail;
}

// private
bool frame::destroy()
{
    const auto buffer = reinterpret_cast<zmq_msg_t*>(&message_);
    return valid_ && (zmq_msg_close(buffer) != zmq_fail);
}

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin