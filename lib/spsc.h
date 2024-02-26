#include <optional>
#include <iostream>
#include <cstddef>

namespace lockfree {
namespace spsc {

template <typename T>
class Queue {
public:
    Queue(size_t capacity) {
        this->capacity_ = capacity;
        this->buffer_ = new T[capacity + 1];
        this->write_idx_ = 0;
        this->read_idx_ = 0;
    }

    ~Queue() {
        delete[] this->buffer_;
    }

    bool push_back(const T& elem) {
        // Return false if the queue is full
        if ((this->write_idx_+1) % (this->capacity_+1) == this->read_idx_) {
            return false;
        }

        // Push the element to the back of the queue
        this->buffer_[this->write_idx_] = elem;
        this->write_idx_ = (this->write_idx_ + 1) % (this->capacity_ + 1);
        return true;
    }

    std::optional<T> pop_front() {
        // Return nothing if the queue is empty
        if (this->write_idx_ == this->read_idx_) {
            return std::nullopt;
        }

        auto elem = this->buffer_[this->read_idx_];
        this->read_idx_ = (this->read_idx_ + 1) % (this->capacity_ + 1);
        return elem;
    }

    std::optional<T> front() {
        // Return nothing if the queue is empty
        if (this->write_idx_ == this->read_idx_) {
            return std::nullopt;
        }

        auto elem = this->buffer_[this->read_idx_];
        return elem;
    }

    bool empty() const {
        return this->write_idx_ == this->read_idx_;
    }

    bool full() const {
        return this->write_idx_ + 1 == this->read_idx_;
    }
private:
    size_t capacity_;
    T* buffer_;
    size_t write_idx_;
    size_t read_idx_;
};

} // namespace spsc
} // namespace lockfree
