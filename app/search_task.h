#pragma once
#include <string>
#include <functional>
#include <string_view>
#include <cassert>

struct search_entry
{
    int line = {};
    int position = {};
    std::string value;
};

struct search_task_result
{
    size_t lines_processed = {};
    size_t line_chars_processed = {};
    std::vector<search_entry> entries;
};

struct naive_mask_finder
{
    static size_t find(const std::string_view &line, const std::string_view &mask, size_t pos)
    {
        if (mask.size() > line.size() || pos >= line.size())
            return line.npos;

        for (auto i = pos; i < line.size() - mask.size() + 1; i++)
        {
            auto found = true;
            for (auto j = 0u; j < mask.size(); j++)
            {
                if (mask[j] != '?' && mask[j] != line[i + j])
                {
                    found = false;
                    break;
                }
            }

            if (found)
                return i;
        }

        return line.npos;
    }
};

template <typename _Finder>
class search_task
{
    const std::string &mask_;
    std::string_view buffer_;
    size_t offset_ = {};
    size_t size_ = {};
    std::string previous_buffer_ending_;

    void search_between_buffers(search_task_result &result)
    {
        if (previous_buffer_ending_.empty())
            return;

        auto n = mask_.size() - 1;
        previous_buffer_ending_.append(buffer_.begin(), buffer_.begin() + n);

        auto pos = _Finder::find(previous_buffer_ending_, mask_, 0);
        if (pos == previous_buffer_ending_.npos)
            return;

        result.entries.emplace_back(result.lines_processed, (int)pos - n,
                                    previous_buffer_ending_.substr(pos, mask_.size()));
    }

    void search_in_line(const std::string_view &line, search_task_result &result)
    {
        auto pos = _Finder::find(line, mask_, 0);
        while (pos != line.npos)
        {
            result.entries.emplace_back(result.lines_processed, pos, std::string(line.substr(pos, mask_.size())));
            pos = _Finder::find(line, mask_, pos + mask_.size());
        }
    }

public:
    search_task(const std::string &mask,
                std::string_view buffer,
                size_t offset,
                size_t size) : mask_(mask),
                               buffer_(buffer),
                               offset_(offset),
                               size_(size)
    {
    }

    search_task(const std::string &mask,
                std::string_view buffer,
                size_t offset,
                size_t size,
                const std::string &prev_ending) : mask_(mask),
                                                  buffer_(buffer),
                                                  offset_(offset),
                                                  size_(size),
                                                  previous_buffer_ending_(prev_ending)
    {
        assert(previous_buffer_ending_.size() < 2 * mask_.size());
    }

    search_task_result operator()()
    {
        search_task_result result;

        search_between_buffers(result);

        auto search_area = buffer_.substr(offset_, size_);

        auto current(0), last(0);
        while ((current = search_area.find_first_of('\n', last)) != search_area.npos)
        {
            auto line = search_area.substr(last, current - last);
            search_in_line(line, result);
            ++result.lines_processed;
            last = current + 1;
        };

        auto overlap = offset_ + size_ < buffer_.size() ? mask_.size() - 1 : 0;
        auto ending_search_area = buffer_.substr(offset_ + last, size_ - last + overlap);
        search_in_line(ending_search_area, result);

        result.line_chars_processed = size_ - last;

        return result;
    }
};