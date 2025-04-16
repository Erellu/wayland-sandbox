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

#include "display.hpp"
#include "registry.hpp"
#include "shm_buffer.hpp"
#include "window_info.hpp"
#include "xdg/surface.hpp"
#include "xdg/toplevel.hpp"
#include "xdg/wm_base.hpp"

#include <optional>
#include <ranges>
#include <utility>
#include <vector>

#include <wayland-client.h>

namespace fubuki::io::platform::linux_bsd::wayland
{

class window
{
    struct token
    {
    };

    static constexpr std::size_t shm_pool_layers = 2;



public:

    class components
    {
        [[nodiscard]] auto construct_buffers()
        {
            std::vector<shm_buffer> buffers = {};
            buffers.reserve(shm_pool_layers);

            buffers.emplace_back(shm_buffer{pool, {.index = 0}});
            buffers.emplace_back(shm_buffer{pool, {.index = 1}});
            return buffers;
        }

        [[nodiscard]] xdg::surface construct_surface() { return xdg::surface{wm_base}; }

        [[nodiscard]] xdg::toplevel construct_toplevel() { return xdg::toplevel{surface}; }

    public:
        shm_pool                pool;
        std::vector<shm_buffer> buffers;
        xdg::wm_base            wm_base;
        xdg::surface            surface;
        xdg::toplevel           toplevel;

        components(display& parent, const window_info& i) :
                                                     pool{parent, {.width = static_cast<std::size_t>(i.size.width), .height = static_cast<std::size_t>(i.size.height), .layers = shm_pool_layers}},
                                                     buffers{construct_buffers()},
                                                     wm_base{parent},
                                                     surface{construct_surface()},
                                                     toplevel{construct_toplevel()}
        {
        }

        components(shm_pool pool, std::vector<shm_buffer> buffers, xdg::wm_base wm_base, xdg::surface surface, xdg::toplevel toplevel) noexcept
            : pool{std::move(pool)},
              buffers{std::move(buffers)},
              wm_base{std::move(wm_base)},
              surface{std::move(surface)},
              toplevel{std::move(toplevel)}

        {
        }

        components(components&& other) noexcept
            : pool{std::move(other.pool)},
              buffers{std::move(other.buffers)},
              wm_base{std::move(other.wm_base)},
              surface{std::move(other.surface)},
              toplevel{std::move(other.toplevel)}
        {
        }

        components& operator=(components&& other) noexcept
        {
            swap(other);
            return *this;
        }

        components(const components&)            = delete;
        components& operator=(const components&) = delete;

        void swap(components& other) noexcept
        {
            pool.swap(other.pool);
            buffers.swap(other.buffers);
            wm_base.swap(other.wm_base);
            surface.swap(other.surface);
            toplevel.swap(other.toplevel);
        }

        friend void swap(components& a, components& b) noexcept { a.swap(b); }
    };

    struct any_call_info
    {
    };

    window(display& parent, window_info i) : m_registry{parent}, m_components{parent, i}, m_info{std::move(i)}
    {
        if(const auto error = create(parent))
        {
            throw std::runtime_error("Wayland surface creation failed");
        }
    }

    window(const window&)            = delete;
    window& operator=(const window&) = delete;

    window(window&& other) noexcept
        : m_registry{std::move(other.m_registry)},
          m_components{std::move(other.m_components)},
          m_info{std::move(other.m_info)}
    {
        swap_xdg_surface_listener_data(other);
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

        auto pool = shm_pool::make(
            parent, {.width = static_cast<std::size_t>(i.size.width), .height = static_cast<std::size_t>(i.size.height), .layers = shm_pool_layers});

        if(not pool)
        {
            return std::unexpected{any_call_info{}};
        }

        std::vector<shm_buffer> buffers = {};
        buffers.reserve(shm_pool_layers);

        for(const auto i : std::views::iota(0u, shm_pool_layers))
        {
            auto buffer = shm_buffer::make(*pool, {.index = i});

            if(not buffer)
            {
                return std::unexpected{any_call_info{}};
            }

            buffers.emplace_back(*std::move(buffer));
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

        auto result = window{token{},
                             *std::move(r),
                             *std::move(pool),
                             std::move(buffers),
                             *std::move(wm_base),
                             *std::move(surface),
                             *std::move(toplevel),
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
    [[nodiscard]] const auto& buffers() const noexcept { return m_components.buffers; }
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
        m_info.swap(other.m_info);

        swap_xdg_surface_listener_data(other);
    }

    friend void swap(window& a, window& b) noexcept { a.swap(b); }

private:

    window(token,
           registry                r,
           shm_pool                pool,
           std::vector<shm_buffer> buffers,
           xdg::wm_base            wm_base,
           xdg::surface            surface,
           xdg::toplevel           toplevel,
           window_info             i) noexcept
        : m_registry{std::move(r)},
          m_components{
              std::move(pool),
              std::move(buffers),
              std::move(wm_base),
              std::move(surface),
              std::move(toplevel),
          },

          m_info{std::move(i)}
    {
    }

    [[nodiscard]]
    std::optional<any_call_info> create(display& parent) noexcept;

    void swap_xdg_surface_listener_data(window& other) noexcept;

    registry   m_registry;
    components m_components;

    window_info m_info = {};
};

} // namespace fubuki::io::platform::linux_bsd::wayland
#endif // FUBUKI_IO_PLATFORM_LINUX_WAYLAND_WINDOW_HPP
