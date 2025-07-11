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

#ifndef FUBUKI_IO_PLATFORM_WAYLAND_REGISTRY_HPP
#define FUBUKI_IO_PLATFORM_WAYLAND_REGISTRY_HPP

#include "display.hpp"

#include <expected>

#include <wayland-client.h>

namespace fubuki::io::platform::linux_bsd::wayland
{

class registry
{
    struct token
    {
    };

public:
    struct connection_error
    {
    };

    registry(wl_display* d) : m_handle{wl_display_get_registry(d)}
    {
        if(m_handle == nullptr)
        {
            throw std::runtime_error{"Failed to connect to Wayland display"};
        }
    }

    registry(display& d) : registry{d.handle()} {}

    registry(const registry&)            = delete;
    registry& operator=(const registry&) = delete;
    registry(registry&& other) noexcept { swap(other); }
    registry& operator=(registry&& other) noexcept
    {
        swap(other);
        return *this;
    }

    [[nodiscard]] static std::expected<registry, connection_error> make(wl_display* d) noexcept
    {
        registry result{token{}};

        result.m_handle = wl_display_get_registry(d);

        if(not result)
        {
            return std::unexpected{connection_error{}};
        }

        return result;
    }

    [[nodiscard]] static std::expected<registry, connection_error> make(display& d) noexcept { return make(d.handle()); }

    ~registry() noexcept
    {
        if(m_handle != nullptr)
        {
            wl_registry_destroy(m_handle);
        }
    }

    [[nodiscard]] explicit operator bool() const noexcept { return m_handle != nullptr; }

    [[nodiscard]] auto* handle() noexcept { return m_handle; }

    void        swap(registry& other) noexcept { std::swap(m_handle, other.m_handle); }
    friend void swap(registry& a, registry& b) noexcept { a.swap(b); }

private:
    registry(token) noexcept {}

    wl_registry* m_handle = nullptr;
};

} // namespace fubuki::io::platform::linux_bsd::wayland

#endif // FUBUKI_IO_PLATFORM_WAYLAND_REGISTRY_HPP
