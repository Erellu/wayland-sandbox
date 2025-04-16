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

#include "shm_buffer.hpp"

#include <cassert>
#include <cstdint>

namespace fubuki::io::platform::linux_bsd::wayland
{

[[nodiscard]] std::optional<shm_buffer::any_call_info> shm_buffer::create(shm_pool& parent) noexcept
{
    constexpr std::size_t format_stride = 4; // 32-bit, 4 bytes
    const auto            stride        = static_cast<std::int32_t>(width() * format_stride);
    const auto            offset        = height() * width() * index() * format_stride;

    m_handle = wl_shm_pool_create_buffer(parent.handle(),
                                         static_cast<std::int32_t>(offset),
                                         static_cast<std::int32_t>(width()),
                                         static_cast<std::int32_t>(height()),
                                         stride,
                                         information::format);

    if(m_handle == nullptr)
    {
        return any_call_info{};
    }

    m_memory = parent.memory().subspan(offset, size_bytes());

    return {};
}

} // namespace fubuki::io::platform::linux_bsd::wayland
