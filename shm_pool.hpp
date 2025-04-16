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

#ifndef FUBUKI_IO_PLATFORM_LINUX_WAYLAND_SHM_POOL_HPP
#define FUBUKI_IO_PLATFORM_LINUX_WAYLAND_SHM_POOL_HPP

#include "display.hpp"
#include "file_descriptor.hpp"
#include "scoped_mmap.hpp"

#include <cstddef>
#include <optional>
#include <utility>

#include <wayland-client.h>

namespace fubuki::io::platform::linux_bsd::wayland
{

class shm_pool
{
    struct token
    {
    };

public:

    struct any_call_info
    {
    };

    struct information
    {
        static constexpr auto format = WL_SHM_FORMAT_ARGB8888;

        std::size_t width  = 1;
        std::size_t height = 1;
        std::size_t layers = 2;
        // Only format supported: ARGB8888
    };

    shm_pool(display& parent, information i) : m_globals{parent.globals()}, m_info{i}
    {
        if(const auto error = create())
        {
            throw std::runtime_error("");
        }
    }

    shm_pool(const shm_pool&)            = delete;
    shm_pool& operator=(const shm_pool&) = delete;

    shm_pool(shm_pool&& other) noexcept
        : m_fd{std::move(other.m_fd)},
          m_memory{std::move(other.m_memory)},
          m_info{other.m_info},
          m_globals{other.m_globals},
          m_handle{std::exchange(other.m_handle, nullptr)}
    {
    }

    shm_pool& operator=(shm_pool&& other) noexcept
    {
        swap(other);
        return *this;
    }

    ~shm_pool() noexcept
    {
        if(m_handle != nullptr)
        {
            wl_shm_pool_destroy(m_handle);
        }
    }

    [[nodiscard]] static std::expected<shm_pool, any_call_info> make(display& parent, information i) noexcept
    {
        auto result = shm_pool{token{}, parent.globals(), i};

        if(const auto error = result.create())
        {
            return std::unexpected{any_call_info{}};
        }

        return result;
    }

    [[nodiscard]] const auto& info() const noexcept { return m_info; }

    [[nodiscard]] auto size_bytes() const noexcept
    {
        constexpr std::size_t format_stride = 4; // 32-bit, 4 bytes

        return m_info.width * m_info.height * format_stride * m_info.layers;
    }

    [[nodiscard]] auto*       handle() noexcept { return m_handle; }
    [[nodiscard]] const auto* handle() const noexcept { return m_handle; }

    [[nodiscard]] const auto& memory() const noexcept { return m_memory; }

    [[nodiscard]] const auto& globals() const noexcept { return m_globals; }

    void swap(shm_pool& other) noexcept
    {
        m_fd.swap(other.m_fd);
        m_memory.swap(other.m_memory);
        m_globals.swap(other.m_globals);
        std::swap(m_info, other.m_info);
        std::swap(m_handle, other.m_handle);
    }

    friend void swap(shm_pool& a, shm_pool& b) noexcept { a.swap(b); }

private:

    shm_pool(token, display::global g, information i) noexcept : m_globals{g}, m_info{i}
    {
        m_info.width  = std::max(std::size_t{1}, m_info.width);
        m_info.height = std::max(std::size_t{1}, m_info.height);
        m_info.layers = std::max(std::size_t{1}, m_info.layers);
    }

    [[nodiscard]]
    std::optional<any_call_info> create() noexcept;

    file_descriptor m_fd      = {};
    scoped_mmap     m_memory  = {};
    display::global m_globals = {};
    information     m_info    = {};
    wl_shm_pool*    m_handle  = nullptr;
};

} // namespace fubuki::io::platform::linux_bsd::wayland

#endif // FUBUKI_IO_PLATFORM_LINUX_WAYLAND_SHM_POOL_HPP
