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

#ifndef FUBUKI_IO_PLATFORM_LINUX_WAYLAND_DECOR_FRAME_HPP
#define FUBUKI_IO_PLATFORM_LINUX_WAYLAND_DECOR_FRAME_HPP

#include "../xdg/surface.hpp"
#include "context.hpp"

#include <functional>

struct libdecor_frame;

namespace fubuki::io::platform::linux_bsd::wayland::decor
{

class frame
{
    struct token
    {
    };

public:

    struct any_call_info
    {
    };

    struct task
    {
        std::function<void()> configure     = []() {};
        std::function<void()> close         = []() {};
        std::function<void()> commit        = []() {};
        std::function<void()> dismiss_popup = []() {};

        void swap(task& other) noexcept
        {
            configure.swap(other.configure);
            close.swap(other.close);
            commit.swap(other.commit);
            dismiss_popup.swap(other.dismiss_popup);
        }

        friend void swap(task& a, task& b) noexcept { a.swap(b); }
    };

    frame(display& parent, xdg::surface& target) : m_context{parent}
    {
        if(const auto error = create(parent, target))
        {
            throw std::runtime_error("");
        }
    }

    frame(const frame&)            = delete;
    frame& operator=(const frame&) = delete;

    frame(frame&& other) noexcept;

    frame& operator=(frame&& other) noexcept
    {
        swap(other);
        return *this;
    }

    ~frame() noexcept;

    [[nodiscard]] static std::expected<frame, any_call_info> make(display& parent, xdg::surface& target, task t) noexcept
    {
        auto ctx = context::make(parent);

        if(not ctx)
        {
            return std::unexpected{any_call_info{}};
        }

        auto result = frame{token{}, *std::move(ctx), std::move(t)};

        if(const auto error = result.create(parent, target))
        {
            return std::unexpected{any_call_info{}};
        }

        return result;
    }

    [[nodiscard]] auto*       handle() noexcept { return m_handle; }
    [[nodiscard]] const auto* handle() const noexcept { return m_handle; }

    void        swap(frame& other) noexcept;
    friend void swap(frame& a, frame& b) noexcept { a.swap(b); }

private:

    frame(token, context ctx, task t) noexcept : m_context{std::move(ctx)}, m_task{std::move(t)} {}

    [[nodiscard]] std::optional<any_call_info> create(display& parent, xdg::surface& target) noexcept;

    context         m_context;
    task            m_task   = {};
    libdecor_frame* m_handle = nullptr;
};

} // namespace fubuki::io::platform::linux_bsd::wayland::decor

#endif // FUBUKI_IO_PLATFORM_LINUX_WAYLAND_DECOR_FRAME_HPP
