#pragma once
#include <iostream>
#include <memory>

class stream_reader
{
    struct string_buffer
    {
        std::unique_ptr<char[]> data;
        size_t count = {};

        std::string_view get_view()
        {
            return std::string_view(data.get(), count);
        }
    };

    std::istream &stream_;
    const size_t buffer_size_;
    std::pair<string_buffer, string_buffer> double_buffer_;
    string_buffer *current_buffer_;

    void swap_buffers() noexcept
    {
        current_buffer_ = current_buffer_ == &double_buffer_.first ? &double_buffer_.second : &double_buffer_.first;
    }

public:
    stream_reader(std::istream &stream, size_t buffer_size) : stream_(stream), buffer_size_(buffer_size),
                                                              current_buffer_(&double_buffer_.second)
    {
        double_buffer_.first.data = std::make_unique<char[]>(buffer_size_);
        double_buffer_.second.data = std::make_unique<char[]>(buffer_size_);
    }

    std::string_view read_buffer()
    {
        if (!stream_)
            throw std::runtime_error("Stream is invalid: IO error or file does not exist.");

        swap_buffers();
        stream_.read(current_buffer_->data.get(), buffer_size_);
        current_buffer_->count = stream_.gcount();
        return current_buffer_->get_view();
    }

    std::string_view get_back_buffer() noexcept
    {
        return current_buffer_ == &double_buffer_.first ? double_buffer_.second.get_view() : double_buffer_.first.get_view();
    }

    bool is_completed() const noexcept
    {
        return stream_.eof();
    }

    size_t get_reading_size() const noexcept
    {
        return buffer_size_;
    }
};