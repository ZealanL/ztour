#pragma once
#include <ztour/base.h>

// A struct for tracking how many calls are active, and enabling waiting for all calls to cease
// (Vaguely resembles Rust's `std::sync::Barrier`... sort of)
struct AtomicCallGate {
private:
    std::atomic<size_t> num_active_calls = 0;
    std::mutex cond_mutex = {};
    std::condition_variable cond_var = {};
public:
    AtomicCallGate() = default;

    void on_enter_call() {
        num_active_calls.fetch_add(1, std::memory_order_relaxed);
    }

    void on_exit_call() {
        size_t remaining = num_active_calls.fetch_sub(1, std::memory_order_acq_rel) - 1;
        if (remaining == 0) {
            auto lock = std::unique_lock(cond_mutex);
            cond_var.notify_all();
        }
    }

    // Wait until there are no active calls
    void wait() {
        auto lock = std::unique_lock(cond_mutex);
        cond_var.wait(lock, [this] {
            return num_active_calls.load(std::memory_order_acquire) == 0;
        });
    }

    size_t num_active() const {
        return num_active_calls.load();
    }

    ~AtomicCallGate() {
        ZT_ASSERT(num_active_calls.load() == 0)
    }
};