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

#include "display.hpp"
#include "screen.hpp"
#include "shm_buffer.hpp"
#include "shm_pool.hpp"
#include "test.hpp"
#include "window.hpp"

#include <iomanip>
#include <iostream>
#include <thread>

namespace sandbox::wayland
{

[[nodiscard]] int test_connection(const char* name)
{
    // https://wayland-book.com/wayland-display/creation.html

    if(auto display = fbk_wl::display::make(name))
    {

        try
        {
            fbk_wl::display display_1{name};
        }
        catch(const std::runtime_error& e)
        {
            std::cerr << "Failed to connect to display " << std::quoted(name) << "(display::ctor): " << e.what() << std::flush;
            return 1;
        }
    }
    else
    {
        std::cerr << "Failed to connect to display " << std::quoted(name) << "(display::make)" << std::flush;
        return 1;
    }

    return 0;
}

[[nodiscard]] int enumerate_screens()
{
    std::cout << "screens:\n";
    for(const auto& p : fbk_wl::screen::enumerate())
    {
        std::cout << p << "\n******************************\n";
    }
    std::cout << "\n" << std::flush;

    return 0;
}

[[nodiscard]] int shm_buffer()
{
    auto display = fbk_wl::display::make();

    if(not display)
    {
        return 1;
    }

    auto pool = fbk_wl::shm_pool::make(*display,
                                       fbk_wl::shm_pool::information{
                                           .width  = 512,
                                           .height = 512,
                                           .layers = 2,
                                       });
    if(not pool)
    {
        return 2;
    }

    auto buffer = fbk_wl::shm_buffer::make(*pool, fbk_wl::shm_buffer::information{});

    if(not buffer)
    {
        return 3;
    }

    return 0;
}

[[nodiscard]] int window()
{
    auto display = fbk_wl::display::make();

    if(not display)
    {
        return 1;
    }

    auto window = fbk_wl::window::make(*display,
                                       fubuki::io::platform::window_info{
                                           .title       = "Wayland window",
                                           .size        = {640, 480},
                                           .coordinates = {1408, 568},
                                           .opacity     = 1.f,
                                           .style       = {},
    });

    if(not window)
    {
        return 2;
    }

    // window->show();

    while (wl_display_dispatch(display->handle())) {
        /* This space deliberately left blank */
    }


    // for(int i = 0; i < 3; ++i)
    // {
    //     wl_display_dispatch_pending(display->handle());
    //     std::this_thread::sleep_for(std::chrono::seconds{1});
    // }

    return 0;
}

} // namespace sandbox::wayland
