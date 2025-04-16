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

#ifndef FUBUKI_IO_PLATFORM_LINUX_SCOPED_MMAP_HPP
#define FUBUKI_IO_PLATFORM_LINUX_SCOPED_MMAP_HPP

#include <algorithm>
#include <cassert>
#include <expected>
#include <span>
#include <stdexcept>
#include <utility>

#include <sys/mman.h>

namespace fubuki::io::platform::linux_bsd
{

/**
 * A RAII object that calls mmap upon construction, and munmap upon destruction.
 */
class scoped_mmap
{
    struct token
    {
    };

public:

    using range_type       = std::span<std::byte>;
    using element_type     = typename range_type::element_type;
    using value_type       = typename range_type::value_type;
    using size_type        = typename range_type::size_type;
    using difference_type  = typename range_type::difference_type;
    using pointer          = typename range_type::pointer;
    using const_pointer    = typename range_type::const_pointer;
    using reference        = typename range_type::reference;
    using const_reference  = typename range_type::const_reference;
    using iterator         = typename range_type::iterator;
    using reverse_iterator = typename range_type::reverse_iterator;

    struct error
    {
    };

    /**
     * Constructor.
     * @throws call_exception This constructor throws an instance of call_exception if a call failed.
     * @see https://man7.org/linux/man-pages/man2/mmap.2.html
     */
    scoped_mmap(void* addr, std::size_t len, int prot, int flags, int fd, ::off_t offset = 0)
    {
        auto* const begin = static_cast<std::byte*>(mmap(addr, len, prot, flags, fd, offset));

        if(begin == MAP_FAILED)
        {
            throw std::runtime_error{"mmap failed"};
        }

        assert(begin);

        m_contents = {begin, len};
    }

    /// Default constructor. Does not map a region.
    scoped_mmap() noexcept = default;

    scoped_mmap(const scoped_mmap&)            = delete;
    scoped_mmap& operator=(const scoped_mmap&) = delete;

    /// Move constructor.
    scoped_mmap(scoped_mmap&& other) noexcept : m_contents{std::exchange(other.m_contents, std::span<std::byte>{})} {}

    /// Move assignment operator.
    scoped_mmap& operator=(scoped_mmap&& other) noexcept
    {
        swap(other);
        return *this;
    }

    /// Destructor. Unmaps the region.
    ~scoped_mmap() noexcept
    {
        if(m_contents.data() != nullptr)
        {
            std::ignore = munmap(m_contents.data(), m_contents.size());
        }
    }

    /**
     * Equivalent of a noexcept constructor.
     * @returns A valid instance of scoped_mmap, or an instance of any_call_info describing the call that failed.
     * @see https://man7.org/linux/man-pages/man2/mmap.2.html
     */
    [[nodiscard]] static std::expected<scoped_mmap, error> make(void* addr, std::size_t len, int prot, int flags, int fd, ::off_t offset = 0) noexcept
    {
        auto* ptr = static_cast<std::byte*>(mmap(addr, len, prot, flags, fd, offset));
        assert(ptr);

        if(ptr == MAP_FAILED)
        {
            return std::unexpected{error{}};
        }

        return scoped_mmap{token{}, ptr, len};
    }

    [[nodiscard]] constexpr iterator         begin() const noexcept { return m_contents.begin(); }
    [[nodiscard]] constexpr iterator         end() const noexcept { return m_contents.end(); }
    [[nodiscard]] constexpr reverse_iterator rbegin() const noexcept { return m_contents.rbegin(); }
    [[nodiscard]] constexpr reverse_iterator rend() const noexcept { return m_contents.rend(); }

    [[nodiscard]] constexpr reference     front() const { return m_contents.front(); }
    [[nodiscard]] constexpr reference     back() const { return m_contents.back(); }
    [[nodiscard]] constexpr reference     operator[](size_type idx) const { return m_contents[idx]; }
    [[nodiscard]] constexpr const_pointer data() const noexcept { return m_contents.data(); }
    [[nodiscard]] constexpr pointer       data() noexcept { return m_contents.data(); }

    [[nodiscard]] constexpr size_type size() const noexcept { return m_contents.size(); }
    [[nodiscard]] constexpr size_type size_bytes() const noexcept { return m_contents.size_bytes(); }
    [[nodiscard]] constexpr bool      empty() const noexcept { return m_contents.empty(); }

    template<std::size_t count>
    [[nodiscard]] constexpr std::span<element_type, count> first() const noexcept
    {
        return m_contents.first<count>();
    }

    [[nodiscard]] constexpr std::span<element_type, std::dynamic_extent> first(size_type count) const noexcept { return m_contents.first(count); }
    template<std::size_t count>
    [[nodiscard]] constexpr std::span<element_type, count> last() const noexcept
    {
        return m_contents.last<count>();
    }

    [[nodiscard]] constexpr std::span<element_type, std::dynamic_extent> last(size_type count) const noexcept { return m_contents.last(count); }

    template<std::size_t offset, std::size_t count = std::dynamic_extent>
    [[nodiscard]] constexpr auto subspan() const
    {
        return m_contents.subspan<offset, count>();
    }
    [[nodiscard]] constexpr std::span<element_type, std::dynamic_extent> subspan(size_type offset, size_type count = std::dynamic_extent) const
    {
        return m_contents.subspan(offset, count);
    }

    [[nodiscard]] constexpr operator std::span<std::byte>() noexcept { return m_contents; }
    [[nodiscard]] constexpr operator std::span<const std::byte>() const noexcept { return {m_contents.data(), m_contents.size()}; }

    [[nodiscard]] friend constexpr bool operator==(const scoped_mmap& a, const scoped_mmap& b) noexcept
    {
        return std::ranges::equal(a.m_contents, b.m_contents);
    }
    [[nodiscard]] friend constexpr bool operator!=(const scoped_mmap& a, const scoped_mmap& b) noexcept { return not(a == b); }

    [[nodiscard]] friend constexpr auto operator<=>(const scoped_mmap& a, const scoped_mmap& b) noexcept
    {
        return std::lexicographical_compare_three_way(a.m_contents.begin(), a.m_contents.end(), b.m_contents.begin(), b.m_contents.end());
    }

    void swap(scoped_mmap& other) noexcept { std::swap(m_contents, other.m_contents); }

    friend void swap(scoped_mmap& a, scoped_mmap& b) noexcept { a.swap(b); }

private:

    scoped_mmap(token, std::byte* begin, std::size_t size) noexcept : m_contents{begin, size} {}

    std::span<std::byte> m_contents = {};
};

} // namespace fubuki::io::platform::linux_bsd

#endif // FUBUKI_IO_PLATFORM_LINUX_SCOPED_MMAP_HPP
