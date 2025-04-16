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

#include "display.hpp"
#include "registry.hpp"
#include "xdg/generated/shell-client-protocol.hpp"
#include "zxdg/generated/decoration-client-protocol.hpp"

#include <cstdint>
#include <iostream>

namespace fubuki::io::platform::linux_bsd::wayland
{

namespace
{

namespace callback::registry
{

void global(void* data, wl_registry* registry, std::uint32_t name, const char* c_interface, std::uint32_t /*version*/) noexcept
{
    auto* const dp = static_cast<display::global*>(data);

    const std::string_view interface = c_interface;

    std::cout << "interface: " << interface << "\n" << std::flush;

    if(interface == wl_compositor_interface.name)
    {
        dp->compositor = static_cast<wl_compositor*>(wl_registry_bind(registry, name, &wl_compositor_interface, 4));
    }

    else if(interface == wl_subcompositor_interface.name)
    {
        dp->subcompositor = static_cast<wl_subcompositor*>(wl_registry_bind(registry, name, &wl_subcompositor_interface, 1));
    }

    else if(interface == wl_shm_interface.name)
    {
        dp->shm = static_cast<wl_shm*>(wl_registry_bind(registry, name, &wl_shm_interface, 1));
    }

    else if(interface == xdg_wm_base_interface.name)
    {
        dp->wm_base = static_cast<xdg_wm_base*>(wl_registry_bind(registry, name, &xdg_wm_base_interface, 1));
    }

    else if(interface == zxdg_decoration_manager_v1_interface.name)
    {
        dp->decoration_manager = static_cast<zxdg_decoration_manager_v1*>(wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1));
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
auto display::create() noexcept -> std::optional<any_call_info>
{
    auto r = registry::make(m_handle);

    if(not r)
    {
        return any_call_info{};
    }

    wl_registry_add_listener(r->handle(), std::addressof(listener::registry), std::addressof(m_globals));
    wl_display_roundtrip(m_handle);

    return {};
}

} // namespace fubuki::io::platform::linux_bsd::wayland
