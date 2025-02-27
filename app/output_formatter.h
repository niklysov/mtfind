#pragma once
#include "search_task.h"
#include <iostream>

class output_formatter
{
    std::ostream &stream_;

public:
    output_formatter(std::ostream &stream) : stream_(stream)
    {
    }

    void print(const std::vector<search_entry> &entries)
    {
        stream_ << entries.size() << std::endl;

        for (auto &entry : entries)
            stream_ << entry.line + 1 << " "
                    << entry.position + 1 << " "
                    << entry.value << std::endl;
    }
};