#pragma once

#include <cstdint>
#include <string>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace fs = boost::filesystem;
using tcp = boost::asio::ip::tcp;

class SHServer {
  public:
    /**
     * @brief Construct a new SHServer object
     *
     * @param root_path root path
     * @param port server port
     **/
    SHServer(fs::path& root_path, std::uint16_t& port);

  private:
    /**
     * @brief Generate file list
     *
     * @param current_path current path
     * @return std::string html page
     **/
    auto generate_file_list(const fs::path& current_path) -> std::string;

    /**
     * @brief Handle HTTP request
     *
     * @param root_path root path
     * @param req request
     * @param res response
     * @param socket tcp socket
     **/
    void handle_request(const fs::path& root_path,
                        http::request<http::string_body>& req,
                        http::response<http::string_body>& res,
                        tcp::socket& socket);

    /**
     * @brief Run server
     *
     **/
    void run_server();

    /**
     * @brief Root Path
     *
     **/
    fs::path& m_ROOT_PATH;

    /**
     * @brief
     *
     **/
    std::uint16_t m_PORT;

    net::io_context m_DEFAULT_IOC;
};
