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

namespace fubuki::io::platform::linux_bsd::wayland
{

namespace
{

void draw_frame(window::components& c)
{
    // TMP, debug code
    constexpr std::size_t width  = 640;
    constexpr std::size_t height = 480;

    assert(c.buffers.front().size_bytes() == width * height * 4);

    auto* const data = reinterpret_cast<std::uint32_t*>(c.buffers.front().memory().data());

    for (std::size_t y = 0; y < height; ++y) {
        for (std::size_t x = 0; x < width; ++x) {
            if ((x + y / 8 * 8) % 16 < 8)
                data[y * width + x] = 0xFF666666;
            else
                data[y * width + x] = 0xFFEEEEEE;
        }
    }
}

namespace callback::xdg
{

void surface_configure(void* data, xdg_surface* xdg_surface, std::uint32_t serial)
{
    auto* w = static_cast<window::components*>(data);
    xdg_surface_ack_configure(xdg_surface, serial);

    // struct wl_buffer *buffer = draw_frame(state);
    draw_frame(*w);
    wl_surface_attach(w->surface.handle(), w->buffers.front().handle(), 0, 0);
    wl_surface_commit(w->surface.handle());

    std::cout << "hi\n" << std::flush;
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
    move(m_info.coordinates);
    resize(m_info.size);
    rename(m_info.title);

    xdg_surface_add_listener(m_components.surface.xdg_handle(), std::addressof(listener::xdg_surface), std::addressof(m_components));

    wl_surface_commit(m_components.surface.handle());
    wl_display_roundtrip(parent.handle());

    return {};
}

void window::show() noexcept {}
void window::hide() noexcept {}
void window::close() noexcept {}

void window::enable(bool) noexcept {}
void window::set_opacity(float /*o*/) noexcept {}

void window::move(position2d p) noexcept
{
    // TODO: may be set in events... Check
    m_info.coordinates = p;
    xdg_surface_set_window_geometry(
        m_components.surface.xdg_handle(), m_info.coordinates.x, m_info.coordinates.y, m_info.size.width, m_info.size.height);
}

void window::resize(dimension2d d) noexcept
{
    // TODO: may be set in events... Check
    m_info.size = d;
    xdg_surface_set_window_geometry(
        m_components.surface.xdg_handle(), m_info.coordinates.x, m_info.coordinates.y, m_info.size.width, m_info.size.height);
}

void window::rename(std::string name)
{
    m_info.title = std::move(name);
    xdg_toplevel_set_title(m_components.toplevel.handle(), m_info.title.c_str());
}

void window::swap_xdg_surface_listener_data(window& other) noexcept
{
    xdg_surface_set_user_data(m_components.surface.xdg_handle(), std::addressof(m_components));

    // May happen after move
    if(other.m_components.surface.xdg_handle() != nullptr)
    {
        xdg_surface_set_user_data(other.m_components.surface.xdg_handle(), std::addressof(other.m_components));
    }
}

} // namespace fubuki::io::platform::linux_bsd::wayland
