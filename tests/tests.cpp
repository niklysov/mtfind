#define BOOST_TEST_MODULE mtfind_tests
#define BOOST_TEST_ALTERNATIVE_INIT_API

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <sstream>
#include <app/searcher.h>

bool operator== (const search_entry& l, const search_entry& r)
{
    return l.line == r.line && l.position == r.position && l.value == r.value;
}

std::ostream& operator<< (std::ostream& s, const search_entry& e)
{
    s << "line: " << e.line << ", position: " << e.position << ", value: " << e.value;
    return s;
}

struct deferred_task_runner
{
    static std::future<search_task_result> start(std::function<search_task_result()> task)
    {
        return std::async(std::launch::deferred, task);
    }
};

struct searcher_fixture
{
    std::stringstream stream;
    searcher<deferred_task_runner, naive_mask_finder> sut;

    constexpr static auto buffer_size = 10;
    constexpr static auto min_search_task_size = 9;

    searcher_fixture() : sut(stream, buffer_size, min_search_task_size)
    {}
};

BOOST_FIXTURE_TEST_SUITE(searcher_suite, searcher_fixture)

BOOST_AUTO_TEST_CASE(search_in_empty)
{
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected;
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_occurence_in_single_line)
{
    stream << "test";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected = { { 0, 0, "test"} };
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_non_occurence_in_single_short_line)
{
    stream << "wow";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected;
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_non_occurence_in_single_big_line)
{
    stream << "wowwowwow!";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected;
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_occurence_in_the_end_of_single_line)
{
    stream << "wowtest";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected = { { 0, 3, "test"} };
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_occurence_in_the_begging_of_single_line)
{
    stream << "testwow";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected = { { 0, 0, "test"} };
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_occurence_in_the_middle_of_single_line)
{
    stream << "wowtestwow";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected = { { 0, 3, "test"} };
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_non_occurence_in_long_single_line)
{
    stream << "wowwowwow_wowwow";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected;
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_occurence_in_the_begging_of_long_single_line)
{
    stream << "wtestwwow_wowwow";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected = { { 0, 1, "test"} };
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_occurence_in_the_end_of_long_single_line)
{
    stream << "wowwowwow_wtestw";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected = { { 0, 11, "test"} };
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_occurence_in_the_middle_of_long_single_line)
{
    stream << "wowwowwow_test__xxxxwowwow";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected = { { 0, 10, "test"} };
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_occurence_between_chunks)
{
    stream << "wowwowwotest__xxxxwowwow";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected = { { 0, 8, "test"} };
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_occurence_between_next_chunks)
{
    stream << "wowwowwowwow__xxxxtestow";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected = { { 0, 18, "test"} };
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_occurence_in_very_end)
{
    stream << "wowwowwote3st__xxxxwowwowtest";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected = { { 0, 25, "test"} };
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_occurence_on_first_line)
{
    stream << "wotestwote\nxxxxwowwowddd";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected = { { 0, 2, "test"} };
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_occurence_on_next_line)
{
    stream << "wowwowwote\nxxxxtestowddd";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected = { { 1, 4, "test"} };
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_multiple_occurences_on_different_lines)
{
    stream << "wowtestote\nxxxxtestowddd";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected = { { 0, 3, "test"}, { 1, 4, "test"} };
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_occurence_between_lines)
{
    stream << "wowwwowote\nstxxxxxwowddd";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected;
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_multiple_occurences_on_same_line)
{
    stream << "wtestotest\nuuxxxxxwowddd";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected = { { 0, 1, "test"}, { 0, 6, "test"} };
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(search_occurence_after_empty_line)
{
    stream << "someline\n\nxxxxtestowddd";
    auto actual = sut.run("t?s?");

    std::vector<search_entry> expected = { { 2, 4, "test"} };
    BOOST_TEST(expected == actual, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(naive_mask_finder_test_suite)

BOOST_AUTO_TEST_CASE(find_in_the_middle)
{
    BOOST_TEST(1 == naive_mask_finder::find("e2d4ff", "2?4?", 0));
}

BOOST_AUTO_TEST_CASE(find_in_the_begging)
{
    BOOST_TEST(0 == naive_mask_finder::find("2u4k 4", "2?4?", 0));
}

BOOST_AUTO_TEST_CASE(find_in_the_end)
{
    BOOST_TEST(2 == naive_mask_finder::find("fs2d4.", "2?4?", 0));
}

BOOST_AUTO_TEST_CASE(find_first_occurence)
{
    BOOST_TEST(3 == naive_mask_finder::find("asd2G4D_das_2u4&sas", "2?4?", 0));
}

BOOST_AUTO_TEST_CASE(find_next_occurence)
{
    BOOST_TEST(6 == naive_mask_finder::find("2m4afs2d4.2_4Ddd", "2?4?", 5));
}

BOOST_AUTO_TEST_CASE(find_occurence_that_matches_all_chars)
{
    BOOST_TEST(0 == naive_mask_finder::find("2m4afs2d4.2_4Ddd", "?", 0));
}

BOOST_AUTO_TEST_SUITE_END()

int main(int argc, char *argv[], char *envp[])
{
    return boost::unit_test::unit_test_main(init_unit_test, argc, argv);
}