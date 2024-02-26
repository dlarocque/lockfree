#include "lib/spsc.h"
#include <gtest/gtest.h>
#include <iostream>
#include <thread>
#include <vector>
#include <optional>

using lockfree::spsc::Queue;

TEST(SpscQueueTest, PushPop) {
    Queue<int> q(1);
    bool pushed = q.push_back(1);

    ASSERT_TRUE(pushed);

    auto popped_elem = q.pop_front();

    ASSERT_TRUE(popped_elem.has_value()); // Ensure there is a value
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(1, popped_elem.value());
}

#if 1
TEST(SpscQueueTest, MultiplePushPop) {
    Queue<int> q(2);
    q.push_back(1);
    q.push_back(2);

    auto first_popped_elem = q.pop_front();
    ASSERT_TRUE(first_popped_elem.has_value()); // Ensure there is a value
    EXPECT_FALSE(q.empty());

    auto second_popped_elem = q.pop_front();
    ASSERT_TRUE(second_popped_elem.has_value()); // Ensure there is a value

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(1, first_popped_elem.value());
    EXPECT_EQ(2, second_popped_elem.value());
}

TEST(SpscQueueTest, EmptyQueuePop) {
    Queue<int> q(0);
    auto popped_elem = q.pop_front();

    EXPECT_TRUE(q.empty());
    EXPECT_FALSE(popped_elem.has_value()); // Expect no value
}

TEST(SpscQueueTest, FullQueuePush) {
    Queue<int> q(2);
    q.push_back(1);
    q.push_back(2);
    EXPECT_FALSE(q.push_back(3)); // Expect push to fail because queue is full
}

TEST(SpscQueueTest, Peek) {
    Queue<int> q(1);
    q.push_back(1);
    auto peeked_elem = q.front(); // Assuming front() peeks the element

    ASSERT_TRUE(peeked_elem.has_value()); // Ensure there is a value
    EXPECT_FALSE(q.empty());
    EXPECT_EQ(1, peeked_elem.value());

    q.pop_front(); // Remove the element after peeking
    EXPECT_TRUE(q.empty());
}

TEST(SpscQueueTest, SingleItemEnqueueDequeue) {
    Queue<int> q(1);
    q.push_back(1);
    auto elem = q.pop_front();

    ASSERT_TRUE(elem.has_value()); // Ensure there is a value
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(1, elem.value());
}

TEST(SpscQueueTest, MaxCapacity) {
    const size_t max_capacity = 5;
    Queue<int> q(max_capacity);

    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(q.push_back(i));
    }

    EXPECT_FALSE(q.push_back(max_capacity)); // Expect this push to fail due to max capacity reached
}

TEST(SpscQueueTest, WrapAround) {
    Queue<int> q(2); // Small queue to force wrap-around
    q.push_back(1);
    q.push_back(2);
    q.pop_front();  // Make space by popping one element

    EXPECT_TRUE(q.push_back(3)); // This should succeed due to wrap-around
    auto elem = q.pop_front();
    ASSERT_TRUE(elem.has_value()); // Ensure there is a value
    EXPECT_EQ(2, elem.value()); // Ensure FIFO order is preserved
    elem = q.pop_front();
    ASSERT_TRUE(elem.has_value()); // Ensure there is a value
    EXPECT_EQ(3, elem.value()); // Check wrap-around element
}

TEST(SpscQueueTest, ZeroCapacityQueue) {
    Queue<int> q(0); // Assuming a zero-capacity queue is allowed
    EXPECT_FALSE(q.push_back(1)); // Expect push to fail because queue has zero capacity
    auto popped_elem = q.pop_front();
    EXPECT_FALSE(popped_elem.has_value()); // Expect no value when popping from an empty queue
}

// Thread Safety and Concurrency
// Note: These tests are simplified and might not fully capture the complexities of concurrent operations.
TEST(SpscQueueTest, ProducerConsumerSync) {
    Queue<int> q(10);
    std::thread producer([&]() {
        for (size_t i = 0; i < 5; ++i) {
            q.push_back(i);
        }
    });

    std::thread consumer([&]() {
        for (size_t i = 0; i < 5; ++i) {
            while (!q.pop_front().has_value()) {
                // Busy wait for the producer to push items
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();
    EXPECT_TRUE(q.empty());
}

TEST(SpscQueueTest, MemoryVisibility) {
    Queue<int> q(10);
    int consumed_value = -1;

    std::thread producer([&]() {
        q.push_back(42);
    });

    std::thread consumer([&]() {
        while (!q.front().has_value()) {
            std::this_thread::yield();
        }
        auto popped_elem = q.pop_front();
        ASSERT_TRUE(popped_elem.has_value());
        consumed_value = popped_elem.value();
        EXPECT_EQ(42, consumed_value);
    });

    producer.join();
    consumer.join();
}

TEST(SpscQueueTest, OrderPreservation) {
    Queue<int> q(10);
    std::vector<int> produced = {1, 2, 3, 4, 5};
    std::vector<int> consumed;

    std::thread producer([&]() {
        for (int item : produced) {
            q.push_back(item);
        }
    });

    std::thread consumer([&]() {
        for (size_t i = 0; i < produced.size(); ++i) {
            std::optional<int> item;
            while (!(item = q.pop_front()).has_value()) {
                std::this_thread::yield();
            }
            consumed.push_back(item.value());
        }
    });

    producer.join();
    consumer.join();
    EXPECT_EQ(produced, consumed);
}

// Performance and Stress
TEST(SpscQueueTest, HighVolume) {
    Queue<int> q(10000);
    const size_t num_items = 10000;

    std::thread producer([&]() {
        for (size_t i = 0; i < num_items; ++i) {
            q.push_back(i);
        }
    });

    std::thread consumer([&]() {
        for (size_t i = 0; i < num_items; ++i) {
            while (!q.pop_front().has_value()) {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();
    EXPECT_TRUE(q.empty());
}

TEST(SpscQueueTest, BurstTraffic) {
    Queue<int> q(100);
    const size_t num_bursts = 10;
    const size_t burst_size = 100;

    std::thread producer([&]() {
        for (size_t burst = 0; burst < num_bursts; ++burst) {
            for (size_t i = 0; i < burst_size; ++i) {
                bool pushed = q.push_back(i);
                EXPECT_TRUE(pushed);
            }
        }
    });

    std::thread consumer([&]() {
        for (size_t burst = 0; burst < num_bursts; ++burst) {
            for (size_t i = 0; i < burst_size; ++i) {
                while (!q.pop_front().has_value()) {
                    std::this_thread::yield();
                }
            }
        }
    });

    producer.join();
    consumer.join();
    EXPECT_TRUE(q.empty());
}

// Special Cases and Edge Conditions
TEST(SpscQueueTest, NullItems) {
    Queue<std::optional<int>> q(10); // Using std::optional<int> to allow "null" values
    q.push_back(std::nullopt); // Simulating a "null" item
    auto popped_elem = q.pop_front();

    EXPECT_TRUE(popped_elem.has_value()); // There is an item, but it's "null"
    EXPECT_FALSE(popped_elem.value().has_value()); // The item itself is "null"
}

TEST(SpscQueueTest, TypeSafety) {
    Queue<std::string> q(10);
    q.push_back("test");
    auto popped_elem = q.pop_front();

    ASSERT_TRUE(popped_elem.has_value()); // Ensure there is a value
    EXPECT_EQ("test", popped_elem.value());
}
#endif
