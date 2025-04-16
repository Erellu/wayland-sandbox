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

#include "window.hpp"

#include <iostream>
#include <ranges>

namespace fubuki::io::platform::linux_bsd::wayland
{

namespace
{

void apply_opacity(window::components& c)
{
    // TMP, debug code
    {
        const auto expected = static_cast<std::size_t>(c.info.size.width) * static_cast<std::size_t>(c.info.size.height) * 4;
        if(c.buffer.size_bytes() != expected)
        {
            std::cout << "buffer: " << c.buffer.size_bytes() << " expected: " << expected << "\n" << std::flush;
            assert(c.buffer.size_bytes() == expected);
        }
    }

    const auto alpha = static_cast<std::byte>(c.info.opacity * 255.f);

    auto alpha_bytes = c.buffer.memory() | std::views::enumerate
        | std::views::filter(
                           [](const auto& t) noexcept
                           {
                               const auto& [index, byte] = t;
                               std::ignore               = byte;
                               return (index % 4 == 3);
                           })
        | std::views::transform(
                           [](const auto& t) noexcept -> std::byte&
                           {
                               auto& [index, byte] = t;
                               std::ignore         = index;
                               return byte;
                           });

    std::ranges::fill(alpha_bytes, alpha);

    // constexpr auto fill = 0xFF'FF'00'00;
    // std::span<std::uint32_t> data = {reinterpret_cast<std::uint32_t*>(c.buffer.memory().data()), c.buffer.memory().size_bytes() /
    // sizeof(std::uint32_t)}; std::ranges::fill(data, fill);
}

namespace callback::xdg
{

void surface_configure(void* data, xdg_surface* xdg_surface, std::uint32_t serial)
{
    auto* w = static_cast<window::components*>(data);
    xdg_surface_ack_configure(xdg_surface, serial);

    apply_opacity(*w);
    wl_surface_attach(w->surface.handle(), w->buffer.handle(), 0, 0);

    xdg_surface_set_window_geometry(w->surface.xdg_handle(), w->info.coordinates.x, w->info.coordinates.y, w->info.size.width, w->info.size.height);

    xdg_toplevel_set_title(w->toplevel.handle(), w->info.title.c_str());

    wl_surface_commit(w->surface.handle());
}

} // namespace callback::xdg

namespace listener
{

constexpr xdg_surface_listener xdg_surface = {
    .configure = callback::xdg::surface_configure,
};

} // namespace listener

} // namespace

[[nodiscard]]
std::optional<window::any_call_info> window::create(display& parent) noexcept
{
    constexpr auto fill = 0x00;

    std::ranges::fill(m_components.buffer.memory(), std::byte{fill});

    xdg_surface_add_listener(m_components.surface.xdg_handle(), std::addressof(listener::xdg_surface), std::addressof(m_components));

    xdg_surface_set_window_geometry(m_components.surface.xdg_handle(),
                                    m_components.info.coordinates.x,
                                    m_components.info.coordinates.y,
                                    m_components.info.size.width,
                                    m_components.info.size.height);

    xdg_toplevel_set_title(m_components.toplevel.handle(), m_components.info.title.c_str());

    wl_surface_commit(m_components.surface.handle());
    wl_display_roundtrip(parent.handle());

    return {};
}

void window::show() noexcept {}
void window::hide() noexcept {}
void window::close() noexcept {}

void window::enable(bool) noexcept {}
void window::set_opacity(float o) noexcept
{
    m_components.info.opacity = std::clamp(o, 0.f, 1.f);

    apply_opacity(m_components);
}

void window::move(position2d p) noexcept
{
    // TODO: may be set in events... Check
    // TODO: may not be supported
    m_components.info.coordinates = p;

    xdg_surface_set_window_geometry(m_components.surface.xdg_handle(),
                                    m_components.info.coordinates.x,
                                    m_components.info.coordinates.y,
                                    m_components.info.size.width,
                                    m_components.info.size.height);
    wl_surface_commit(m_components.surface.handle());
}

void window::resize(dimension2d d) noexcept
{
    // TODO: may be set in events... Check

    if(d != m_components.info.size)
    {
        constexpr std::size_t format_stride = 4; // 32-bit, 4 bytes

        if(static_cast<std::size_t>(d.width * d.height) * format_stride > m_components.pool.size_bytes())
        {
            return; // return not supported
        }

        m_components.info.size = d;

        {
            auto new_buffer = shm_buffer::make(m_components.pool, {.index = 0, .width = d.width, .height = d.height});

            if(not new_buffer)
            {
                return; // return not supported
            }

            m_components.buffer = *std::move(new_buffer);
            constexpr auto fill = 0x00;

            // auto* const data = reinterpret_cast<std::uint32_t*>(c.buffers.front().memory().data());
            std::ranges::fill(m_components.buffer.memory(), std::byte{fill});

            apply_opacity(m_components);
        }

        wl_surface_attach(m_components.surface.handle(), m_components.buffer.handle(), 0, 0);

        xdg_surface_set_window_geometry(m_components.surface.xdg_handle(),
                                        m_components.info.coordinates.x,
                                        m_components.info.coordinates.y,
                                        m_components.info.size.width,
                                        m_components.info.size.height);
        apply_opacity(m_components);
        wl_surface_commit(m_components.surface.handle());
    }
}

void window::rename(std::string name)
{
    m_components.info.title = std::move(name);
    xdg_toplevel_set_title(m_components.toplevel.handle(), m_components.info.title.c_str());
    wl_surface_commit(m_components.surface.handle());
}

} // namespace fubuki::io::platform::linux_bsd::wayland
