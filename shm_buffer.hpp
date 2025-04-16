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

#ifndef FUBUKI_IO_PLATFORM_LINUX_WAYLAND_SHM_BUFFER_HPP
#define FUBUKI_IO_PLATFORM_LINUX_WAYLAND_SHM_BUFFER_HPP

#include "shm_pool.hpp"

#include <cstddef>
#include <optional>

namespace fubuki::io::platform::linux_bsd::wayland
{

class shm_buffer
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

        /// Index of the image of the pool this buffer represents.
        std::size_t index = 0;

        /// Width of the buffer.
        ///  If not provided, uses the weight specified in the parent pool information.
        /// If provided, this value is adjusted to be at least 1.
        std::optional<std::size_t> width = {};

        /// Height of the buffer.
        /// If not provided, uses the height specified in the parent pool information.
        /// If provided, this value is adjusted to be at least 1.
        std::optional<std::size_t> height = {};
    };

    shm_buffer(shm_pool& parent, information i) : shm_buffer{token{}, parent, i}
    {
        if(const auto error = create(parent))
        {
            throw std::runtime_error("");
        }
    }

    shm_buffer(const shm_buffer&)            = delete;
    shm_buffer& operator=(const shm_buffer&) = delete;

    shm_buffer(shm_buffer&& other) noexcept :
          m_info{std::exchange(other.m_info, information{})},
          m_handle{std::exchange(other.m_handle, nullptr)},
          m_memory{std::exchange(other.m_memory, std::span<std::byte>{})}
    {
    }

    shm_buffer& operator=(shm_buffer&& other) noexcept
    {
        swap(other);
        return *this;
    }

    ~shm_buffer() noexcept
    {
        if(m_handle != nullptr)
        {
            wl_buffer_destroy(m_handle);
        }
    }

    [[nodiscard]] static std::expected<shm_buffer, any_call_info> make(shm_pool& parent, information i) noexcept
    {
        shm_buffer result = {token{}, parent, i};

        if(const auto error = result.create(parent))
        {
            return std::unexpected{*error};
        }

        return result;
    }

    [[nodiscard]] const auto& info() const noexcept { return m_info; }

    [[nodiscard]] auto index() const noexcept{return m_info.index;}
    [[nodiscard]] auto width() const noexcept { return m_info.width.value_or(1); }
    [[nodiscard]] auto height() const noexcept { return m_info.height.value_or(1); }

    [[nodiscard]] auto size_bytes() const noexcept
    {
        constexpr std::size_t format_stride = 4; // 32-bit, 4 bytes

        return width() * height() * format_stride;
    }

    [[nodiscard]] auto*       handle() noexcept { return m_handle; }
    [[nodiscard]] const auto* handle() const noexcept { return m_handle; }

    [[nodiscard]] std::span<std::byte> memory() noexcept { return m_memory; }

    [[nodiscard]] std::span<const std::byte> memory() const noexcept { return {m_memory}; }

    void swap(shm_buffer& other) noexcept
    {
        std::swap(m_info, other.m_info);
        std::swap(m_handle, other.m_handle);
        std::swap(m_memory, other.m_memory);
    }

    friend void swap(shm_buffer& a, shm_buffer& b) noexcept { a.swap(b); }

private:

    shm_buffer(token, shm_pool& parent, information i) noexcept : m_info{i}
    {
        m_info.index = std::min(m_info.index, parent.info().layers);

        if(m_info.width.has_value())
        {
            m_info.width = std::max(std::size_t{1}, *m_info.width);
        }
        else
        {
            m_info.width = parent.info().width;
        }

        if(m_info.height.has_value())
        {
            m_info.height = std::max(std::size_t{1}, *m_info.height);
        }
        else
        {
            m_info.height = parent.info().height;
        }
    }

    [[nodiscard]] std::optional<any_call_info> create(shm_pool& parent) noexcept;

    wl_buffer*                       m_handle = nullptr;
    information                      m_info   = {};
    std::span<std::byte>             m_memory = {};
};

} // namespace fubuki::io::platform::linux_bsd::wayland

#endif // FUBUKI_IO_PLATFORM_LINUX_WAYLAND_SHM_BUFFER_HPP
