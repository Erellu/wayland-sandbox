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

#ifndef FUBUKI_IO_PLATFORM_LINUX_WAYLAND_SEAT_HPP
#define FUBUKI_IO_PLATFORM_LINUX_WAYLAND_SEAT_HPP

#include "keyboard.hpp"
#include "pointer.hpp"

#include <wayland-client.h>

namespace fubuki::io::platform::linux_bsd::wayland
{

class seat
{
    struct token
    {
    };

public:

    class components
    {
    public:
        wayland::keyboard keyboard;
        wayland::pointer  mouse;

        components(wayland::keyboard k, wayland::pointer p) noexcept : keyboard{std::move(k)}, mouse{std::move(p)} {}

        components(const components&)            = delete;
        components& operator=(const components&) = delete;

        components(components&& other) noexcept : keyboard{std::move(other.keyboard)}, mouse{std::move(other.mouse)} {}

        ~components() noexcept = default;

        void swap(components& other) noexcept
        {
            keyboard.swap(other.keyboard);
            mouse.swap(other.mouse);
        }

        friend void swap(components& a, components& b) noexcept { a.swap(b); }
    };

    struct any_call_info
    {
    };

    seat(display& parent);

    seat(const seat&)            = delete;
    seat& operator=(const seat&) = delete;

    seat(seat&& other) noexcept : m_components{std::move(other.m_components)} {}

    seat& operator=(seat&& other) noexcept
    {
        swap(other);
        return *this;
    }

    ~seat() noexcept = default;

    [[nodiscard]] static std::expected<seat, any_call_info> make(display& parent) noexcept
    {
        auto kb = keyboard::make(parent);

        if(not kb)
        {
            return std::unexpected{any_call_info{}};
        }

        auto p = pointer::make(parent);

        if(not kb)
        {
            return std::unexpected{any_call_info{}};
        }

        return seat{token{}, *std::move(kb), *std::move(p)};
    }

    [[nodiscard]] auto&       parts() noexcept { return m_components; }
    [[nodiscard]] const auto& parts() const noexcept { return m_components; }

    void swap(seat& other) noexcept { m_components.swap(other.m_components); }

    friend void swap(seat& a, seat& b) noexcept { a.swap(b); }

private:

    seat(token, keyboard k, pointer p) noexcept : m_components{std::move(k), std::move(p)} {}

    components m_components;
};

} // namespace fubuki::io::platform::linux_bsd::wayland

#endif // FUBUKI_IO_PLATFORM_LINUX_WAYLAND_SEAT_HPP
