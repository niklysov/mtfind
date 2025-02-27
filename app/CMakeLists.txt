project(app)

set(SOURCE_FILES mtfind.cpp 
                 stream_reader.h
                 search_task.h
                 searcher.h
                 output_formatter.h)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})