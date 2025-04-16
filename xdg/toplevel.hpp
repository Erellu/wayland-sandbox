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

#ifndef FUBUKI_IO_PLATFORM_LINUX_WAYLAND_XDG_TOPLEVEL_HPP
#define FUBUKI_IO_PLATFORM_LINUX_WAYLAND_XDG_TOPLEVEL_HPP

#include "surface.hpp"

namespace fubuki::io::platform::linux_bsd::wayland::xdg
{

class toplevel
{
    struct token
    {
    };

public:

    struct any_call_info
    {
    };


    toplevel(surface& parent)
    {
        if(const auto error = create(parent))
        {
            throw std::runtime_error("");
        }
    }

    toplevel(const toplevel&)            = delete;
    toplevel& operator=(const toplevel&) = delete;

    toplevel(toplevel&& other) noexcept : m_handle{std::exchange(other.m_handle, nullptr)} {}

    toplevel& operator=(toplevel&& other) noexcept
    {
        swap(other);
        return *this;
    }

    ~toplevel() noexcept
    {
        if(m_handle != nullptr)
        {
            xdg_toplevel_destroy(m_handle);
        }
    }

    [[nodiscard]] static std::expected<toplevel, any_call_info> make(surface& parent) noexcept
    {
        toplevel result{token{}};

        if(const auto error = result.create(parent))
        {
            return std::unexpected{*error};
        }

        return result;
    }

    [[nodiscard]] auto*       handle() noexcept { return m_handle; }
    [[nodiscard]] const auto* handle() const noexcept { return m_handle; }

    void swap(toplevel& other) noexcept
    {
        std::swap(m_handle, other.m_handle);
    }

    friend void swap(toplevel& a, toplevel& b) noexcept { a.swap(b); }


private:

    toplevel(token) noexcept {}

    [[nodiscard]]
    std::optional<any_call_info> create(surface& parent) noexcept;

    xdg_toplevel* m_handle = nullptr;
};

} // namespace fubuki::io::platform::linux_bsd::wayland::xdg

#endif // FUBUKI_IO_PLATFORM_LINUX_WAYLAND_XDG_TOPLEVEL_HPP
