#include <iostream>
#include <string>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <thread>
#include <cstddef>
#include <cassert>
#include <utility>

using namespace boost::interprocess;

int main(int argc, char **argv)
{
    try
    {
        managed_shared_memory segment(open_only, "GitSyncd-sharedMemory");
        typedef std::pair<std::string, int> command;
        std::pair<command *, managed_shared_memory::size_type> res;

        segment.construct<command>("commandVector")("addFile", 0);

        while (1)
        {
            // sleep for 1 second
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            std::cout << "Checking for new commands..." << std::endl;
            res = segment.find<command>("response");
            if (res.first->first == "addFile")
            {
                std::cout << "Adding file..." << std::endl;
                segment.destroy<command>("response");
            }
        }
    }
    catch (interprocess_exception &ex)
    {
        std::cout << ex.what() << std::endl;
        return 1;
    }
    catch (std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
        return 1;
    }
}