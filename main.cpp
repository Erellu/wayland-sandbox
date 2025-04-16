#include "file_descriptor.hpp"
#include "test.hpp"

#include <expected>
#include <functional>
#include <iomanip>
#include <iostream>
#include <optional>
#include <thread>
#include <type_traits>

#include <wayland-client.h>

// TODO:
// - https://wayland-book.com/xdg-shell-basics/example-code.html
// - https://wayland-book.com/seat/example.html

template<typename func, typename... arguments>
requires(std::invocable<func, arguments...> and std::is_same_v<int, std::invoke_result_t<func, arguments...>>)
[[nodiscard]] std::optional<int> run(std::string_view label, func&& f, arguments&&... args) noexcept(std::is_nothrow_invocable_v<func, arguments...>)
{
    if(const auto x = std::invoke(f, std::forward<arguments>(args)...); x != 0)
    {
        std::cout << "/-------------------------------------------/"
                  << "Error during test " << std::quoted(label) << ", exited with code " << x << "/-------------------------------------------/\n"
                  << std::flush;
        return x;
    }

    std::cout << "/-------------------------------------------/"
              << "Test " << std::quoted(label) << " succeeded " << "/-------------------------------------------/\n"
              << std::flush;

    return {};
}

int main()
{
    // if(const auto x = run("test connection", sandbox::wayland::test_connection, /*name=*/ nullptr))
    // {
    //     return *x;
    // }

    // if(const auto x = run("enumerate_screens", sandbox::wayland::enumerate_screens))
    // {
    //     return *x;
    // }

    if(const auto x = run("shm_buffer", sandbox::wayland::shm_buffer))
    {
        return *x;
    }

    if(const auto x = run("window", sandbox::wayland::window))
    {
        return *x;
    }

    //

    // using fd = fubuki::io::platform::linux_bsd::file_descriptor;

    // auto x = fd::make_anonymous(4096, "/home/erel/", "fubuki-io-linux-wayland", fd::anonymous_hint::preference::provided);

    // while (wl_display_dispatch(display.underlying()) != -1) {
    //     /* This space deliberately left blank */
    //     // See also wl_display_get_fd
    //     // wl_display_flush
    // }

    return 0;
}
