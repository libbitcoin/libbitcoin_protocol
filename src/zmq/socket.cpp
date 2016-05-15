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
#include <bitcoin/protocol/zmq/socket.hpp>

#include <czmq.h>
#include <bitcoin/bitcoin.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

socket::socket()
  : self_(nullptr)
{
}

socket::socket(void* self)
  : self_(self)
{
}

socket::socket(socket&& other)
{
    BITCOIN_ASSERT(self_ == nullptr);
    self_ = other.self_;
    other.self_ = nullptr;
}

socket::socket(context& context, int type)
{
    self_ = zsocket_new(context.self(), type);
}

socket::operator const bool() const
{
    return self_ != nullptr;
}

bool socket::operator==(const socket& other) const
{
    return self_ == other.self_;
}

bool socket::operator!=(const socket& other) const
{
    return !(*this == other);
}

void* socket::self()
{
    return self_;
}

void* socket::self() const
{
    return self_;
}

void socket::destroy(context& context)
{
    BITCOIN_ASSERT(self_);
    zsocket_destroy(context.self(), self_);
}

// format-security: format not a string literal and no format arguments.
int socket::bind(const std::string& address)
{
    return zsocket_bind(self_, address.c_str());
}

// format-security: format not a string literal and no format arguments.
int socket::connect(const std::string& address)
{
    static constexpr int zmq_no_linger = 0;

    zsocket_set_linger(self_, zmq_no_linger);
    return zsocket_connect(self_, address.c_str());
}

void socket::set_curve_server()
{
    zsocket_set_curve_server(self_, 1);
}

void socket::set_curve_serverkey(const std::string& key)
{
    zsocket_set_curve_serverkey(self_, key.c_str());
}

void socket::set_zap_domain(const std::string& domain)
{
    zsocket_set_zap_domain(self_, domain.c_str());
}

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin