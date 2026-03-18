//
// Created by mfrfo on 2/6/2026.
//

#ifndef EMBEDDEDCOROUTINES_EMBCOROUTINES_HPP
#define EMBEDDEDCOROUTINES_EMBCOROUTINES_HPP

#include <coroutine>
#include <cstdio>
#include <cstddef>
#include <new>


alignas(std::max_align_t) static uint8_t g_coroutine_buffer[1024];
static bool g_buffer_used = false;

struct StaticTask {
    // The Compiler looks for 'promise_type' inside the return type
    struct promise_type {

        // 1. INITIALIZATION
        StaticTask get_return_object() {
            return StaticTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend() { return {}; } // Start immediately? Yes.
        std::suspend_always final_suspend() noexcept { return {}; } // Don't destroy automatically

        void return_void() {}
        void unhandled_exception() { /* Trap error here */ }

        // --------------------------------------------------------
        // THE MAGIC: Custom Allocator
        // --------------------------------------------------------
        // When the compiler wants to create the Frame, it calls this.
        void* operator new(std::size_t size) {
            if (size > sizeof(g_coroutine_buffer)) {
                // Error: Static buffer too small!
                // In embedded, trigger a blinking LED or assert.
                return nullptr;
            }
            if (g_buffer_used) {
                // Error: Coroutine re-entrancy not supported in this simple example
                return nullptr;
            }

            g_buffer_used = true;
            printf("[System] Allocating %zu bytes from static buffer\n", size);
            return g_coroutine_buffer;
        }

        void operator delete(void* ptr, std::size_t size) {
            printf("[System] Freeing static buffer\n");
            g_buffer_used = false;
        }
    };

    std::coroutine_handle<promise_type> handle;

    // RAII to clean up
    ~StaticTask() {
        if (handle) handle.destroy();
    }
};



#endif //EMBEDDEDCOROUTINES_EMBCOROUTINES_HPP