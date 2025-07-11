/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2025, Erwan DUHAMEL
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "generated/shell-client-protocol.hpp"
#include "wm_base.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>

namespace fubuki::io::platform::linux_bsd::wayland::xdg
{

namespace
{

namespace callback
{

void pong(void* /*user*/, struct xdg_wm_base* handle, std::uint32_t serial) { xdg_wm_base_pong(handle, serial); }

} // namespace callback

namespace listener
{

constexpr xdg_wm_base_listener xdg = {
    .ping = callback::pong,
};

} // namespace listener

} // namespace

[[nodiscard]]
auto wm_base::create() noexcept -> std::optional<any_call_info>
{
    xdg_wm_base_add_listener(m_globals.wm_base, std::addressof(listener::xdg), nullptr);

    if(m_globals.wm_base == nullptr)
    {
        std::cerr << "Parent display globals().wm_base was nullptr\n" << std::flush;
        return any_call_info{};
    }

    m_handle = m_globals.wm_base;

    return {};
}

} // namespace fubuki::io::platform::linux_bsd::wayland::xdg
