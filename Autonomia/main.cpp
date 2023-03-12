#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <thread>

int main()
{
    // Get the start time of the program
    auto start = std::chrono::system_clock::now();

    while (true) {
        // Get the current time
        auto now = std::chrono::system_clock::now();
        std::time_t t_now = std::chrono::system_clock::to_time_t(now);

        // Convert the time to a string
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&t_now));
        std::string datetime = buffer;

        // Get the elapsed time since the program started
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();

        // Convert the elapsed time to a string
        std::string elapsed_str = std::to_string(elapsed);

        // Write the datetimes and elapsed time to a text file
        std::ofstream outfile("datetime.txt", std::ofstream::out | std::ofstream::trunc);
        outfile << "Current time: " << datetime << std::endl;
        outfile << "Start time: " << std::ctime(&t_now) << std::endl;
        outfile << "Elapsed time (seconds): " << elapsed_str << std::endl;
        outfile.close();

        std::cout << "Datetime written to datetime.txt" << std::endl;

        // Wait for 60 seconds before repeating
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return 0;
}
