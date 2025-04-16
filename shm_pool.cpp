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

#include "file_descriptor.hpp"
#include "scoped_mmap.hpp"
#include "shm_pool.hpp"

#include <cstdint>
#include <iostream>
#include <memory>
#include <string_view>

#include <fcntl.h>
#include <unistd.h>
#include <wayland-client-protocol.h>

namespace fubuki::io::platform::linux_bsd::wayland
{

namespace
{

template<typename T>
struct free_c_storage
{
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory)
    void operator()(T* ptr) const { free(ptr); }
};

template<typename T>
using unique_c_ptr = std::unique_ptr<T, free_c_storage<T>>;

constexpr auto format = WL_SHM_FORMAT_ARGB8888;

namespace callback::registry
{

void global(void* data, wl_registry* registry, std::uint32_t name, const char* c_interface, std::uint32_t /*version*/) noexcept
{
    auto* const pool = static_cast<shm_pool::global*>(data);

    const std::string_view interface = c_interface;

    if(interface == "wl_shm")
    {
        pool->shm = static_cast<wl_shm*>(wl_registry_bind(registry, name, &wl_shm_interface, 1));
    }
}

void global_remove(void* data, wl_registry* registry, std::uint32_t name) noexcept {}

} // namespace callback::registry

namespace listener
{
constexpr wl_registry_listener registry{.global = callback::registry::global, .global_remove = callback::registry::global_remove};
} // namespace listener

} // anonymous namespace

[[nodiscard]]
std::optional<shm_pool::any_call_info> shm_pool::create(display& parent) noexcept
{
    wl_registry_add_listener(m_registry.handle(), std::addressof(listener::registry), std::addressof(m_globals));

    // Sync to retrieve all registry objects
    wl_display_roundtrip(parent.handle());

    if(m_globals.shm == nullptr)
    {
        std::cerr << "Failed to get shm handle\n" << std::flush;
        return any_call_info{};
    }

    const unique_c_ptr<char> current_dir{get_current_dir_name()};

    if(not current_dir)
    {
        std::cerr << "Failed to get current directory\n" << std::flush;
        return any_call_info{};
    }

    {
        auto fd = file_descriptor::make_anonymous(size_bytes(), std::string_view{current_dir.get()}, "fubuki-io-wayland-shm_pool");

        if(not fd)
        {
            std::cerr << "Failed to create anonymous fd\n" << std::flush;
            return any_call_info{};
        }

        m_fd = *std::move(fd);
    }

    {
        auto mmap_scope = scoped_mmap::make(nullptr, size_bytes(), PROT_READ | PROT_WRITE, MAP_SHARED, m_fd.get().value, 0);

        if(not mmap_scope)
        {
            std::cerr << "Failed to mmap\n" << std::flush;
            return any_call_info{};
        }

        m_memory = *std::move(mmap_scope);
    }

    m_handle = wl_shm_create_pool(m_globals.shm, m_fd.get().value, static_cast<std::int32_t>(size_bytes()));

    if(m_handle == nullptr)
    {
        return any_call_info{};
    }

    return {};
}

} // namespace fubuki::io::platform::linux_bsd::wayland
