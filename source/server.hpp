#pragma once

#include <string>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace fs = boost::filesystem;
using tcp = boost::asio::ip::tcp;

class SHServer {
  public:
    SHServer(fs::path& root_path, unsigned short& port);

  private:
    auto generate_file_list(const fs::path& current_path) -> std::string;

    void handle_request(const fs::path& root_path,
                        http::request<http::string_body>& req,
                        http::response<http::string_body>& res,
                        tcp::socket& socket);

    void run_server();

    fs::path& m_ROOT_PATH;

    unsigned short m_PORT;
};
