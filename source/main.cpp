#include <cstdint>
#include <cstdlib>
#include <iostream>

#include "server.hpp"
#include "tracelogger.hpp"

#include <boost/filesystem.hpp>

auto main(int argc, char* argv[]) -> int {
    LOG_TRACE

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <path_to_directory> <port>" << "\n";
        return 1;
    }

    boost::filesystem::path root_path(argv[1]);

    auto port = static_cast<std::uint16_t>(std::atoi(argv[2]));

    if (!boost::filesystem::exists(root_path) || !boost::filesystem::is_directory(root_path)) {
        std::cerr << "Invalid directory path" << "\n";
        return 1;
    }

    SHServer const SERVER(root_path, port);

    return 0;
}
