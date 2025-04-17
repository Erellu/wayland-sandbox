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

#include "frame.hpp"
#include "../libdecor/libdecor/src/libdecor.h"

namespace fubuki::io::platform::linux_bsd::wayland::decor
{

namespace
{

namespace callback
{

void configure(struct libdecor_frame* frame, struct libdecor_configuration* configuration, void* user_data)
{
    auto* task = static_cast<frame::task*>(user_data);

    task->configure();
}

void close(struct libdecor_frame* frame, void* user_data)
{
    auto* task = static_cast<frame::task*>(user_data);

    task->close();
}

void commit(struct libdecor_frame* frame, void* user_data)
{
    auto* task = static_cast<frame::task*>(user_data);

    task->commit();
}

void dismiss_popup(struct libdecor_frame* frame, const char* seat_name, void* user_data)
{
    auto* task = static_cast<frame::task*>(user_data);

    task->dismiss_popup();
}

} // namespace callback

namespace listener
{

constexpr libdecor_frame_interface libdecor{
    .configure     = callback::configure,
    .close         = callback::close,
    .commit        = callback::commit,
    .dismiss_popup = callback::dismiss_popup,
};

} // namespace listener

} // anonymousnamespace

[[nodiscard]] auto frame::create(display& parent, xdg::surface& target) noexcept -> std::optional<any_call_info>
{
    m_handle = libdecor_decorate(m_context.handle(), target.handle(), std::addressof(listener::libdecor), std::addressof(m_task));

    if(m_handle == nullptr)
    {
        return any_call_info{};
    }

    return {};
}

frame::frame(frame&& other) noexcept : m_context{std::move(other.m_context)}, m_handle{std::exchange(other.m_handle, nullptr)}
{
    libdecor_frame_set_user_data(m_handle, std::addressof(m_task));
}

frame::~frame() noexcept
{
    if(m_handle != nullptr)
    {
        libdecor_frame_unref(m_handle);
    }
}

void frame::swap(frame& other) noexcept
{
    m_context.swap(other.m_context);
    std::swap(m_handle, other.m_handle);
    m_task.swap(other.m_task);
    libdecor_frame_set_user_data(m_handle, std::addressof(m_task));
    libdecor_frame_set_user_data(other.m_handle, std::addressof(other.m_task));
}

} // namespace fubuki::io::platform::linux_bsd::wayland::decor
