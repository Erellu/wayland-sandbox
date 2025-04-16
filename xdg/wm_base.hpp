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

#ifndef FUBUKI_IO_PLATFORM_LINUX_WAYLAND_XDG_BASE_HPP
#define FUBUKI_IO_PLATFORM_LINUX_WAYLAND_XDG_BASE_HPP

#include "../display.hpp"
#include "../registry.hpp"
#include "generated/shell-client-protocol.hpp"

#include <optional>
#include <utility>

namespace fubuki::io::platform::linux_bsd::wayland::xdg
{

class wm_base
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
        xdg_wm_base*      wm            = nullptr;

        global() noexcept                = default;
        global(const global&)            = delete;
        global& operator=(const global&) = delete;
        global(global&& other) noexcept
            : compositor{std::exchange(other.compositor, nullptr)},
              subcompositor{std::exchange(other.subcompositor, nullptr)},
              wm{std::exchange(other.wm, nullptr)}
        {
        }
        global& operator=(global&& other) noexcept
        {
            swap(other);
            return *this;
        }

        void swap(global& other) noexcept { std::swap(wm, other.wm); }

        friend void swap(global& a, global& b) noexcept { a.swap(b); }
    };

    wm_base(display& parent) : m_registry{parent}
    {
        if(const auto error = create(parent))
        {
            throw std::runtime_error("");
        }
    }

    wm_base(const wm_base&)            = delete;
    wm_base& operator=(const wm_base&) = delete;

    wm_base(wm_base&& other) noexcept
        : m_registry{std::move(other.m_registry)},
          m_globals{std::move(other.m_globals)},
          m_handle{std::exchange(other.m_handle, nullptr)}
    {
    }

    wm_base& operator=(wm_base&& other) noexcept
    {
        swap(other);
        return *this;
    }

    ~wm_base() noexcept
    {
        if(m_handle != nullptr)
        {
            xdg_wm_base_destroy(m_handle);
        }
    }

    [[nodiscard]] static std::expected<wm_base, any_call_info> make(display& parent) noexcept
    {
        auto r = registry::make(parent);

        if(not r)
        {
            return std::unexpected{any_call_info{}};
        }

        auto result = wm_base{token{}, *std::move(r)};

        if(const auto error = result.create(parent))
        {
            return std::unexpected{any_call_info{}};
        }

        return result;
    }

    [[nodiscard]] auto*       handle() noexcept { return m_handle; }
    [[nodiscard]] const auto* handle() const noexcept { return m_handle; }

    [[nodiscard]] const auto& globals() const noexcept { return m_globals; }

    void swap(wm_base& other) noexcept;

    friend void swap(wm_base& a, wm_base& b) noexcept { a.swap(b); }

private:

    wm_base(token, registry r) noexcept : m_registry{std::move(r)} {}

    [[nodiscard]]
    std::optional<any_call_info> create(display& parent) noexcept;

    registry     m_registry;
    global       m_globals = {};
    xdg_wm_base* m_handle  = nullptr;
};

} // namespace fubuki::io::platform::linux_bsd::wayland::xdg

#endif // FUBUKI_IO_PLATFORM_LINUX_WAYLAND_XDG_BASE_HPP
