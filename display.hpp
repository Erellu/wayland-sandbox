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

#ifndef FUBUKI_IO_PLATFORM_LINUX_WAYLAND_DISPLAY_HPP
#define FUBUKI_IO_PLATFORM_LINUX_WAYLAND_DISPLAY_HPP

#include <expected>
#include <optional>
#include <stdexcept>
#include <utility>

#include <wayland-client.h>

struct xdg_wm_base;
struct zxdg_decoration_manager_v1;

namespace fubuki::io::platform::linux_bsd::wayland
{

class display
{
    struct token
    {
    };

public:
    struct any_call_info
    {
    };

    struct global
    {
        wl_compositor*    compositor    = nullptr;
        wl_subcompositor* subcompositor = nullptr;
        wl_shm*           shm           = nullptr;

        xdg_wm_base*                wm_base            = nullptr;
        zxdg_decoration_manager_v1* decoration_manager = nullptr;

        void swap(global& other) noexcept
        {
            std::swap(compositor, other.compositor);
            std::swap(subcompositor, other.subcompositor);
            std::swap(shm, other.shm);
            std::swap(wm_base, other.wm_base);
            std::swap(decoration_manager, other.decoration_manager);
        }

        friend void swap(global& a, global& b) noexcept { a.swap(b); }
    };

    display(const char* name = nullptr) : m_handle{wl_display_connect(name)}
    {
        if(m_handle == nullptr)
        {
            throw std::runtime_error{"Failed to connect to Wayland display"};
        }

        if(const auto error = create())
        {
            throw std::runtime_error{"Failed to retrieve display globals"};
        }
    }

    display(const display&)            = delete;
    display& operator=(const display&) = delete;
    display(display&& other) noexcept { swap(other); }
    display& operator=(display&& other) noexcept
    {
        swap(other);
        return *this;
    }

    [[nodiscard]] static std::expected<display, any_call_info> make(const char* name = nullptr) noexcept
    {
        display result{token{}};

        result.m_handle = wl_display_connect(name);

        if(not result)
        {
            return std::unexpected{any_call_info{}};
        }

        if(const auto error = result.create())
        {
            return std::unexpected{any_call_info{}};
        }

        return result;
    }

    ~display() noexcept
    {
        if(m_handle != nullptr)
        {
            wl_display_disconnect(m_handle);
        }
    }

    [[nodiscard]] explicit operator bool() const noexcept { return m_handle != nullptr; }

    [[nodiscard]] auto*       handle() noexcept { return m_handle; }
    [[nodiscard]] const auto* handle() const noexcept { return m_handle; }

    [[nodiscard]] const auto& globals() const noexcept { return m_globals; }

    void swap(display& other) noexcept
    {
        std::swap(m_handle, other.m_handle);
        m_globals.swap(other.m_globals);
    }

    friend void swap(display& a, display& b) noexcept { a.swap(b); }

private:
    display(token) noexcept {}

    [[nodiscard]]
    std::optional<any_call_info> create() noexcept;

    wl_display* m_handle  = nullptr;
    global      m_globals = {};
};

} // namespace fubuki::io::platform::linux_bsd::wayland

#endif // FUBUKI_IO_PLATFORM_LINUX_WAYLAND_DISPLAY_HPP
