#include "log.h"

#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>

bool log_enabled =
#ifdef NDEBUG
    false
#else
    true
#endif
    ;

std::string log_file = LOG_DEFAULT_FILE;
std::ofstream log_stream(log_file, std::ios::out | std::ios::app);

std::chrono::system_clock hr_clock;
std::mutex log_lock;

std::string current_time_and_date()
{
    auto now = hr_clock.now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
	ss << '.' << std::setfill('0') << std::setw(3) << ms;
    return ss.str();
}

std::ostream & log_prefix(std::ofstream & stream) {
    stream << current_time_and_date() << " [" << std::this_thread::get_id() << "] ";
    return stream;
}

