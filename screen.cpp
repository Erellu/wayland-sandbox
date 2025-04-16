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
#include "screen.hpp"

#include <mutex>

namespace fubuki::io::platform::linux_bsd::wayland::screen
{

namespace
{

namespace globals
{

auto& monitors()
{
    struct result
    {
        std::vector<properties> r   = {properties{}}; // At least one monitor is expected
        std::vector<wl_output*> out = {nullptr};
    };

    static result r = {};
    return r;
}

auto& sync()
{
    static std::mutex m = {};
    return m;
}

} // namespace globals

namespace callbacks
{

void geometry(void* /*data*/,
              wl_output* /*wl_output*/,
              int32_t x,
              int32_t y,
              int32_t /*physical_width*/,
              int32_t /*physical_height*/,
              int32_t     subpixel,
              const char* make,
              const char* model,
              int32_t     transform) noexcept
{
    auto& next = globals::monitors().r.back();

    next.area.offset.x = x;
    next.area.offset.y = y;
    next.name          = std::string{make} + "-" + model;
}

void pixel_contexts(void* /*data*/, wl_output* wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh) noexcept
{
    auto& next = globals::monitors().r.back();

    next.area.extent.width  = width;
    next.area.extent.height = height;
    next.refresh_rate       = static_cast<std::uint32_t>(refresh);
}

void done(void* /*data*/, wl_output* wl_output) noexcept
{
    auto& next = globals::monitors().r.back();

    globals::monitors().out.back() = wl_output;
    globals::monitors().r.push_back({});
    globals::monitors().out.push_back({});
}

void scale(void* /*data*/, wl_output* /*wl_output*/, int32_t factor) noexcept { /*globals::monitors().r.back().scale = factor;*/ }

void name(void* /*data*/, wl_output* /*wl_output*/, const char* name) noexcept { globals::monitors().r.back().name = name; }

void description(void* /*data*/, wl_output* /*wl_output*/, const char* /*description*/) noexcept {}

constexpr wl_output_listener listener = {geometry, pixel_contexts, done, scale, name, description};

void interface_added(void* data, wl_registry* registry, std::uint32_t name, const char* interface, std::uint32_t version) noexcept
{
    if(std::string_view{interface} == std::string_view{wl_output_interface.name})
    {
        wl_output* const out = static_cast<wl_output*>(wl_registry_bind(registry, name, &wl_output_interface, version));
        wl_output_add_listener(out, &listener, nullptr);
    }
}

constexpr wl_registry_listener registry_listener = {
    .global = interface_added, .global_remove = nullptr
};

} // namespace callbacks

} // namespace

[[nodiscard]] std::vector<properties> enumerate(display& p)
{
    const std::scoped_lock<std::mutex> lock{globals::sync()};

    registry display_registry {p};

    if(not display_registry)
    {
        return {};
    }

    wl_registry_add_listener(display_registry.handle(), &callbacks::registry_listener, nullptr);
    wl_display_roundtrip(p.handle());

    while(wl_display_dispatch(p.handle()) == -1)
    {
    }

    // The last (added upon call to callback::done) is an invalid entry
    globals::monitors().r.pop_back();
    globals::monitors().out.pop_back();

    for(const auto out : globals::monitors().out)
    {
        wl_output_release(out);
    }

    auto copy           = globals::monitors().r;
    globals::monitors() = {};

    return copy;
}

} // namespace fubuki::io::platform::linux_bsd::wayland::screen
