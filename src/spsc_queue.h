#pragma once
#ifndef SPSC_QUEUE_H_INCLUDED
#define SPSC_QUEUE_H_INCLUDED

#include <atomic>
#include <cstddef>
#include <new>
#include <type_traits>

template <typename T, size_t Capacity = 256>
class SPSCQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");
    static_assert(Capacity >= 2, "Capacity must be at least 2");

public:
    SPSCQueue() : m_head(0), m_tail(0) {}

    SPSCQueue(const SPSCQueue&) = delete;
    SPSCQueue& operator=(const SPSCQueue&) = delete;
    SPSCQueue(SPSCQueue&&) = delete;
    SPSCQueue& operator=(SPSCQueue&&) = delete;

    bool push(const T& item) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        const size_t head = m_head.load(std::memory_order_relaxed);
        const size_t tail = m_tail.load(std::memory_order_acquire);

        if ((head - tail) >= Capacity) {
            return false;
        }

        m_buffer[head & Mask] = item;
        m_head.store(head + 1, std::memory_order_release);
        return true;
    }

    bool pop(T& item) noexcept(std::is_nothrow_move_assignable_v<T> || std::is_nothrow_copy_assignable_v<T>) {
        const size_t tail = m_tail.load(std::memory_order_relaxed);
        const size_t head = m_head.load(std::memory_order_acquire);

        if (head == tail) {
            return false;
        }

        item = m_buffer[tail & Mask];
        m_tail.store(tail + 1, std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool empty() const noexcept {
        return m_head.load(std::memory_order_relaxed) == m_tail.load(std::memory_order_relaxed);
    }

    [[nodiscard]] size_t size() const noexcept {
        const size_t head = m_head.load(std::memory_order_relaxed);
        const size_t tail = m_tail.load(std::memory_order_relaxed);
        return (head >= tail) ? (head - tail) : 0;
    }

    [[nodiscard]] static constexpr size_t capacity() noexcept {
        return Capacity;
    }

    void clear() noexcept {
        m_head.store(0, std::memory_order_relaxed);
        m_tail.store(0, std::memory_order_relaxed);
    }

private:
    static constexpr size_t Mask = Capacity - 1;

    // Separate head, tail, and buffer onto distinct 64-byte cache lines to eliminate false sharing
    alignas(64) std::atomic<size_t> m_head;
    alignas(64) std::atomic<size_t> m_tail;
    alignas(64) T                  m_buffer[Capacity];
};

#endif // SPSC_QUEUE_H_INCLUDED
