#include "stream_reader.h"
#include "search_task.h"
#include "searcher.h"
#include "output_formatter.h"
#include <fstream>

struct async_task_runner
{
    static std::future<search_task_result> start(std::function<search_task_result()> task)
    {
        return std::async(std::launch::async, task);
    }
};

struct args
{
    std::string path;
    std::string mask;
};

args parge_args(int argc, char *argv[])
{
    if(argc < 3)
        throw std::runtime_error("Program arguments are incorrect: <path> <search mask>");
    
    return { argv[1], argv[2] };
}

int main(int argc, char *argv[])
{
    try
    {
        auto args = parge_args(argc, argv);
        std::ifstream file(args.path);
        searcher<async_task_runner, naive_mask_finder> searcher(file, 8192, 2048);

        output_formatter formatter(std::cout);
        formatter.print(searcher.run(args.mask));
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}