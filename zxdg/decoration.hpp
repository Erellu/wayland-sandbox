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

#ifndef FUBUKI_IO_PLATFORM_LINUX_WAYLAND_ZXDG_DECORATION_HPP
#define FUBUKI_IO_PLATFORM_LINUX_WAYLAND_ZXDG_DECORATION_HPP

#include "../display.hpp"
#include "../xdg/toplevel.hpp"
#include "generated/decoration-client-protocol.hpp"

#include <optional>
#include <utility>

namespace fubuki::io::platform::linux_bsd::wayland::zxdg
{

class decoration
{
    struct token
    {
    };

public:

    struct any_call_info
    {
    };

    decoration(xdg::toplevel& parent)
    {
        if(const auto error = create(parent))
        {
            throw std::runtime_error("");
        }
    }

    decoration(const decoration&)            = delete;
    decoration& operator=(const decoration&) = delete;

    decoration(decoration&& other) noexcept : m_handle{std::exchange(other.m_handle, nullptr)} {}

    decoration& operator=(decoration&& other) noexcept
    {
        swap(other);
        return *this;
    }

    [[nodiscard]] static std::expected<decoration, any_call_info> make(xdg::toplevel& parent) noexcept
    {
        auto result = decoration{token{}};

        if(const auto error = result.create(parent))
        {
            return std::unexpected{any_call_info{}};
        }

        return result;
    }

    ~decoration() noexcept
    {
        if(m_handle != nullptr)
        {
            zxdg_toplevel_decoration_v1_destroy(m_handle);
        }
    }

    [[nodiscard]] auto*       handle() noexcept { return m_handle; }
    [[nodiscard]] const auto* handle() const noexcept { return m_handle; }

    void swap(decoration& other) noexcept { std::swap(m_handle, other.m_handle); }

    friend void swap(decoration& a, decoration& b) noexcept { a.swap(b); }

private:

    decoration(token) noexcept {}

    [[nodiscard]]
    std::optional<any_call_info> create(xdg::toplevel& parent) noexcept;

    zxdg_toplevel_decoration_v1* m_handle = nullptr;
};

} // namespace fubuki::io::platform::linux_bsd::wayland::zxdg

#endif // FUBUKI_IO_PLATFORM_LINUX_WAYLAND_ZXDG_DECORATION_HPP
