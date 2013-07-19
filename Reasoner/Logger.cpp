#include "Logger.h"

#include <mutex>
#include <iostream>
#include <sstream>
#include <thread>
namespace {
	std::mutex consoleAccess;
}
void Logger::print(const std::string& s) {
	std::lock_guard<std::mutex> lock(consoleAccess);
	std::cout << "(" << std::this_thread::get_id() << ")" << s << std::endl;
}
void Logger::unsafePrint(const std::string& s) {
	std::cout << s << std::endl;
}