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

#ifndef FUBUKI_IO_PLATFORM_LINUX_WAYLAND_WINDOW_HPP
#define FUBUKI_IO_PLATFORM_LINUX_WAYLAND_WINDOW_HPP

#include "decoration.hpp"
#include "display.hpp"
#include "registry.hpp"
#include "screen.hpp"
#include "seat.hpp"
#include "shm_buffer.hpp"
#include "window_info.hpp"
#include "xdg/surface.hpp"
#include "xdg/toplevel.hpp"
#include "xdg/wm_base.hpp"

#include <optional>
#include <utility>

#include <wayland-client.h>

namespace fubuki::io::platform::linux_bsd::wayland
{

class window
{
    struct token
    {
    };


public:

    class components
    {
        [[nodiscard]] static auto construct_pool(display& parent)
        {
            const auto available_screens = screen::enumerate(parent);

            // if(not available_screens){throw call_exception{available_screens.error()};}

            std::size_t width  = 0;
            std::size_t height = 0;

            for(const auto& s : available_screens)
            {
                width += s.area.extent.width;
                height += s.area.extent.height;
            }

            width  = std::max(std::size_t{1}, width);
            height = std::max(std::size_t{1}, height);

            return shm_pool{
                parent, {.width = width, .height = height, .layers = 1}
            };
        }

        [[nodiscard]] auto construct_buffer(const window_info& i)
        {
            return shm_buffer{
                pool,
                {
                  .index  = 0,
                  .width  = i.size.width,
                  .height = i.size.height,
                  }
            };
        }

        [[nodiscard]] xdg::surface construct_surface() { return xdg::surface{wm_base}; }

        [[nodiscard]] xdg::toplevel construct_toplevel() { return xdg::toplevel{surface}; }

        [[nodiscard]] std::optional<decoration> construct_decoration(display& parent, const window_info& i)
        {
            if(i.style == window_style::borderless)
            {
                return std::nullopt;
            }

            // Though Wayland windows are borderless by default (especially on platforms that don't support server-side decorations),
            // the general agreement on desktop is that windows should have at least a close button

            // Not supported on GNOME as of the time of writing
            if(parent.globals().decoration_manager != nullptr)
            {
                return std::optional<decoration>{std::in_place, decoration{decoration::server_side{toplevel}}};
            }

            return decoration{decoration::client_side{}};
        }

    public:

        struct event_state
        {
            struct seat
            {
                struct kb
                {

                    [[nodiscard]] friend constexpr bool operator==(const kb& a, const kb& b) noexcept  = default;
                    [[nodiscard]] friend constexpr bool operator!=(const kb& a, const kb& b) noexcept  = default;
                    [[nodiscard]] friend constexpr auto operator<=>(const kb& a, const kb& b) noexcept = default;
                };

                struct pointer
                {
                    struct button
                    {
                        std::optional<std::uint32_t> last_click_timestamp = {}; ///< Used to determine if the user double-clicked.

                        [[nodiscard]] friend constexpr bool operator==(const button& a, const button& b) noexcept  = default;
                        [[nodiscard]] friend constexpr bool operator!=(const button& a, const button& b) noexcept  = default;
                        [[nodiscard]] friend constexpr auto operator<=>(const button& a, const button& b) noexcept = default;
                    };

                    button left   = {};
                    button centre = {};
                    button right  = {};

                    [[nodiscard]] friend constexpr bool operator==(const pointer& a, const pointer& b) noexcept  = default;
                    [[nodiscard]] friend constexpr bool operator!=(const pointer& a, const pointer& b) noexcept  = default;
                    [[nodiscard]] friend constexpr auto operator<=>(const pointer& a, const pointer& b) noexcept = default;
                };

                kb      keyboard = {};
                pointer mouse    = {};

                [[nodiscard]] friend constexpr bool operator==(const seat& a, const seat& b) noexcept  = default;
                [[nodiscard]] friend constexpr bool operator!=(const seat& a, const seat& b) noexcept  = default;
                [[nodiscard]] friend constexpr auto operator<=>(const seat& a, const seat& b) noexcept = default;
            };

            seat inputs = {};
        };

        shm_pool                  pool;
        shm_buffer                buffer;
        xdg::wm_base              wm_base;
        xdg::surface              surface;
        xdg::toplevel             toplevel;
        seat                      inputs;
        std::optional<decoration> deco;
        window_info               info;
        window_state              state;
        event_state               internal_state;

        components(display& parent, window_info i)
            : pool{construct_pool(parent)},
              buffer{construct_buffer(i)},
              wm_base{parent},
              surface{construct_surface()},
              toplevel{construct_toplevel()},
              inputs{parent},
              deco{construct_decoration(parent, i)},
              info{std::move(i)},
              state{},
              internal_state{}
        {
        }

        components(shm_pool                  p,
                   shm_buffer                buf,
                   xdg::wm_base              xm,
                   xdg::surface              surf,
                   xdg::toplevel             top,
                   seat                      in,
                   std::optional<decoration> dec,
                   window_info               i) noexcept
            : pool{std::move(p)},
              buffer{std::move(buf)},
              wm_base{std::move(xm)},
              surface{std::move(surf)},
              toplevel{std::move(top)},
              inputs{std::move(in)},
              deco{std::move(dec)},
              info{std::move(i)},
              state{},
              internal_state{}
        {
        }

        components(components&& other) noexcept
            : pool{std::move(other.pool)},
              buffer{std::move(other.buffer)},
              wm_base{std::move(other.wm_base)},
              surface{std::move(other.surface)},
              toplevel{std::move(other.toplevel)},
              inputs{std::move(other.inputs)},
              deco{std::move(other.deco)},
              info{std::move(other.info)},
              state{std::exchange(other.state, window_state{})},
              internal_state{std::exchange(other.internal_state, event_state{})}
        {
            xdg_surface_set_user_data(surface.xdg_handle(), this);
            xdg_toplevel_set_user_data(toplevel.handle(), this);
            wl_pointer_set_user_data(inputs.parts().mouse.handle(), this);
            wl_keyboard_set_user_data(inputs.parts().keyboard.handle(), this);
        }

        components& operator=(components&& other) noexcept
        {
            swap(other);
            return *this;
        }

        components(const components&)            = delete;
        components& operator=(const components&) = delete;

        ~components() noexcept = default;

        void swap(components& other) noexcept
        {
            pool.swap(other.pool);
            buffer.swap(other.buffer);
            wm_base.swap(other.wm_base);
            surface.swap(other.surface);
            toplevel.swap(other.toplevel);
            inputs.swap(other.inputs);
            deco.swap(other.deco);
            info.swap(other.info);
            state.swap(other.state);
            std::swap(internal_state, other.internal_state);

            xdg_surface_set_user_data(surface.xdg_handle(), this);
            xdg_surface_set_user_data(other.surface.xdg_handle(), std::addressof(other));

            xdg_toplevel_set_user_data(toplevel.handle(), this);
            xdg_toplevel_set_user_data(other.toplevel.handle(), std::addressof(other));

            wl_pointer_set_user_data(inputs.parts().mouse.handle(), this);
            wl_pointer_set_user_data(other.inputs.parts().mouse.handle(), std::addressof(other));

            wl_keyboard_set_user_data(other.inputs.parts().keyboard.handle(), std::addressof(other));
        }

        friend void swap(components& a, components& b) noexcept { a.swap(b); }
    };

    struct any_call_info
    {
    };

    window(display& parent, window_info i) : m_registry{parent}, m_components{parent, std::move(i)}
    {
        if(const auto error = create(parent))
        {
            throw std::runtime_error("Wayland surface creation failed");
        }
    }

    window(const window&)            = delete;
    window& operator=(const window&) = delete;

    window(window&& other) noexcept : m_registry{std::move(other.m_registry)}, m_components{std::move(other.m_components)}
    {
        xdg_surface_set_user_data(m_components.surface.xdg_handle(), std::addressof(m_components));
    }

    window& operator=(window&& other) noexcept
    {
        swap(other);
        return *this;
    }

    ~window() noexcept = default;

    [[nodiscard]] static std::expected<window, any_call_info> make(display& parent, window_info i) noexcept
    {
        auto r = registry::make(parent);

        if(not r)
        {
            return std::unexpected{any_call_info{}};
        }

        auto pool = shm_pool::make(parent,
                                   {.width = static_cast<std::size_t>(i.size.width), .height = static_cast<std::size_t>(i.size.height), .layers = 1});

        if(not pool)
        {
            return std::unexpected{any_call_info{}};
        }

        auto buffer = shm_buffer::make(*pool, {.index = 0, .width = i.size.width, .height = i.size.height});

        if(not buffer)
        {
            return std::unexpected{any_call_info{}};
        }

        auto wm_base = xdg::wm_base::make(parent);

        if(not wm_base)
        {
            return std::unexpected{any_call_info{}};
        }

        auto surface = xdg::surface::make(*wm_base);

        if(not surface)
        {
            return std::unexpected{any_call_info{}};
        }

        auto toplevel = xdg::toplevel::make(*surface);

        if(not toplevel)
        {
            return std::unexpected{any_call_info{}};
        }

        auto inputs = seat::make(parent);

        if(not inputs)
        {
            return std::unexpected{any_call_info{}};
        }

        std::optional<decoration> deco = {};

        // Though Wayland windows are borderless by default (especially on platforms that don't support server-side decorations),
        // the general agreement on desktop is that windows should have at least a close button

        // Not supported on GNOME as of the time of writing
        if(parent.globals().decoration_manager != nullptr)
        {
            auto ssd = decoration::server_side::make(*toplevel);

            if(not ssd)
            {
                return std::unexpected{any_call_info{}};
            }

            deco = std::optional<decoration>{std::in_place, decoration{*std::move(ssd)}};
        }

        else
        {
            deco = decoration{decoration::client_side{}};
        }

        auto result = window{token{},
                             *std::move(r),
                             *std::move(pool),
                             *std::move(buffer),
                             *std::move(wm_base),
                             *std::move(surface),
                             *std::move(toplevel),
                             *std::move(inputs),
                             std::move(deco),
                             std::move(i)};

        if(const auto error = result.create(parent))
        {
            return std::unexpected{any_call_info{}};
        }

        return result;
    }

    [[nodiscard]] auto*       handle() noexcept { return m_components.surface.handle(); }
    [[nodiscard]] const auto* handle() const noexcept { return m_components.surface.handle(); }
    [[nodiscard]] auto*       xdg_handle() noexcept { return m_components.surface.xdg_handle(); }
    [[nodiscard]] const auto* xdg_handle() const noexcept { return m_components.surface.xdg_handle(); }

    [[nodiscard]] const auto& pool() const noexcept { return m_components.pool; }
    [[nodiscard]] const auto& buffer() const noexcept { return m_components.buffer; }
    [[nodiscard]] const auto& wm_base() const noexcept { return m_components.wm_base; }
    [[nodiscard]] const auto& surface() const noexcept { return m_components.surface; }
    [[nodiscard]] const auto& toplevel() const noexcept { return m_components.toplevel; }

    void show() noexcept;
    void hide() noexcept;
    void close() noexcept;

    void enable(bool) noexcept;
    void set_opacity(float o) noexcept;
    void move(position2d p) noexcept;
    void resize(dimension2d d) noexcept;
    void rename(std::string name);

    void swap(window& other) noexcept
    {
        m_registry.swap(other.m_registry);
        m_components.swap(other.m_components);

        xdg_surface_set_user_data(m_components.surface.xdg_handle(), std::addressof(m_components));
        xdg_surface_set_user_data(other.m_components.surface.xdg_handle(), std::addressof(other.m_components));
    }

    friend void swap(window& a, window& b) noexcept { a.swap(b); }

private:

    window(token,
           registry                  r,
           shm_pool                  pool,
           shm_buffer                buffer,
           xdg::wm_base              wm_base,
           xdg::surface              surface,
           xdg::toplevel             toplevel,
           seat                      inputs,
           std::optional<decoration> deco,
           window_info               i) noexcept
        : m_registry{std::move(r)},
          m_components{
              std::move(pool),
              std::move(buffer),
              std::move(wm_base),
              std::move(surface),
              std::move(toplevel),
              std::move(inputs),
              std::move(deco),
              std::move(i),
          }
    {
    }

    [[nodiscard]]
    std::optional<any_call_info> create(display& parent) noexcept;

    registry   m_registry;
    components m_components;
};

} // namespace fubuki::io::platform::linux_bsd::wayland
#endif // FUBUKI_IO_PLATFORM_LINUX_WAYLAND_WINDOW_HPP
