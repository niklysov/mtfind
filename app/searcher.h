#pragma once
#include "search_task.h"
#include "stream_reader.h"
#include <future>
#include <cmath>

template <typename _TaskRunner, typename _StringFinder>
class searcher
{
    using task_t = search_task<_StringFinder>;

    stream_reader reader_;
    const size_t min_search_task_size_;
    std::vector<std::future<search_task_result>> buffer_results_;
    int last_line_processed_ = {};
    int last_line_chars_processed_ = {};

    size_t lines_processed_ = {};

    void fix_lines_and_positions(search_task_result &result)
    {
        if (last_line_processed_ >= 0 &&
            last_line_processed_ < (lines_processed_ + result.lines_processed))
            last_line_chars_processed_ = 0;

        for (auto &entry : result.entries)
        {
            entry.line += lines_processed_;
            entry.position += last_line_chars_processed_;
        }

        if (result.lines_processed != last_line_processed_)
            last_line_processed_ = result.lines_processed;

        last_line_chars_processed_ += result.line_chars_processed;
        lines_processed_ += result.lines_processed;
    }

    void process_results(std::vector<search_entry> &entries)
    {
        for (auto &result : buffer_results_)
        {
            auto task_result = result.get();
            fix_lines_and_positions(task_result);
            std::move(task_result.entries.begin(), task_result.entries.end(), std::back_inserter(entries));
        }
        buffer_results_.clear();
    }

    std::string get_previous_buffer_end(size_t n)
    {
        auto back_buffer = reader_.get_back_buffer();
        if (back_buffer.empty())
            return "";

        auto ending = back_buffer.substr(back_buffer.size() - n, n);
        return std::string(ending.begin(), ending.end());
    }

    std::vector<task_t> create_tasks(std::string_view buffer,
                                          const std::string &mask)
    {
        std::vector<task_t> tasks;

        auto task_size = std::max(min_search_task_size_, buffer.size() / std::thread::hardware_concurrency());
        auto tasks_count = std::ceil((float)buffer.size() / task_size);

        if (tasks_count == 0)
            return tasks;

        tasks.emplace_back(mask, buffer, 0, task_size, get_previous_buffer_end(mask.size() - 1));
        for (auto i = 1u; i < tasks_count; i++)
            tasks.emplace_back(mask, buffer, i * task_size, std::min(task_size, buffer.size() - i * task_size));
        return tasks;
    }

    void run_tasks(const std::vector<task_t> &tasks)
    {
        for (auto &task : tasks)
            buffer_results_.push_back(_TaskRunner::start(task));
    }

public:
    searcher(std::istream &stream,
             size_t buffer_size,
             size_t min_search_task_size) : reader_(stream, buffer_size),
                                            min_search_task_size_(min_search_task_size)
    {
    }

    std::vector<search_entry> run(const std::string &mask)
    {
        last_line_processed_ = -1;

        std::vector<search_entry> entries;
        do
        {
            auto buffer = reader_.read_buffer();
            process_results(entries);
            run_tasks(create_tasks(buffer, mask));
        } while (!reader_.is_completed());

        process_results(entries);
        return entries;
    }
};