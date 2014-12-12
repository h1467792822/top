

#ifndef TOP_CORE_TEST_TIMER_HPP
#define TOP_CORE_TEST_TIMER_HPP

#include <iostream>
#include <string>
#include <sys/time.h>

namespace top
{

namespace test
{
class Timer
{
    struct timeval start,end;
    std::string out_msg;
public:
    Timer(const std::string& msg = "+++ !!! elapse"): out_msg(msg)
    {
        gettimeofday(&start,0);
    }
    ~Timer()
    {
        gettimeofday(&end,0);
        std::cout << std::endl << out_msg << ":" << (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) << "ms" << std::endl;
    }
};

}

}

#endif

