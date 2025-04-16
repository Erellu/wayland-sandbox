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

namespace callback::registry
{

void global(void* data, wl_registry* registry, std::uint32_t name, const char* c_interface, std::uint32_t /*version*/) noexcept
{
    auto* const wm = static_cast<wm_base::global*>(data);

    const std::string_view interface = c_interface;

    if(interface == "wl_compositor")
    {
        wm->compositor = static_cast<wl_compositor*>(wl_registry_bind(registry, name, &wl_compositor_interface, 4));
    }
    else if(interface == "wl_subcompositor")
    {
        wm->subcompositor = static_cast<wl_subcompositor*>(wl_registry_bind(registry, name, &wl_subcompositor_interface, 1));
    }

    if(interface == "xdg_wm_base")
    {
        wm->wm = static_cast<xdg_wm_base*>(wl_registry_bind(registry, name, &xdg_wm_base_interface, 1));

        xdg_wm_base_add_listener(wm->wm, std::addressof(listener::xdg), nullptr);
    }
}

void global_remove(void* /*data*/, wl_registry* /*registry*/, std::uint32_t /*name*/) noexcept {}

} // namespace callback::registry

namespace listener
{

constexpr wl_registry_listener registry{.global = callback::registry::global, .global_remove = callback::registry::global_remove};

} // namespace listener

} // namespace

[[nodiscard]]
auto wm_base::create(display& parent) noexcept -> std::optional<any_call_info>
{
    wl_registry_add_listener(m_registry.handle(), std::addressof(listener::registry), std::addressof(m_globals));

    // Sync to retrieve all registry objects
    wl_display_roundtrip(parent.handle());

    if(m_globals.wm == nullptr)
    {
        std::cerr << "Failed to get shm handle\n" << std::flush;
        return any_call_info{};
    }

    m_handle = m_globals.wm;

    assert(m_handle);

    return {};
}

void wm_base::swap(wm_base& other) noexcept
{
    m_registry.swap(other.m_registry);
    m_globals.swap(other.m_globals);
    std::swap(m_handle, other.m_handle);

    wl_registry_set_user_data(m_registry.handle(), std::addressof(m_globals));
    wl_registry_set_user_data(other.m_registry.handle(), std::addressof(other.m_globals));
}

} // namespace fubuki::io::platform::linux_bsd::wayland::xdg
