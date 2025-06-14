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
     * @brief Construct a new SHServer instance.
     *
     * This constructor initializes the server with a specified root path
     * for serving files and a port number for accepting incoming connections.
     *
     * @param root_path Reference to the root path from which files will be served.
     * @param port Reference to the server port for handling requests.
     */
    SHServer(fs::path& root_path, std::uint16_t& port);

    /**
     * @brief Generate a list of files in the specified directory.
     *
     * This function scans the current path and creates an HTML page that
     * lists all accessible files and directories.
     *
     * @param current_path The path of the directory to scan.
     * @return std::string An HTML formatted string representing the list of files.
     */
    auto generate_file_list(const fs::path& current_path) -> std::string;

    /**
     * @brief Handle an incoming HTTP request.
     *
     * This function processes the HTTP request, determines the appropriate
     * response, and routes it to the relevant handler based on the request target.
     *
     * @param root_path The root path for serving files.
     * @param req The HTTP request to handle.
     * @param res The HTTP response to populate.
     * @param socket The TCP socket used for communication.
     */
    void handle_request(const fs::path& root_path,
                        http::request<http::string_body>& req,
                        http::response<http::string_body>& res,
                        tcp::socket& socket);

    /**
     * @brief Handle requests for the root directory.
     *
     * This function generates a file listing for the root path and populates
     * the response accordingly.
     *
     * @param root_path The root directory to list.
     * @param res The HTTP response object to populate.
     */
    void handle_root_request(const fs::path& root_path, http::response<http::string_body>& res);

    /**
     * @brief Sanitize the target path to ensure secure access.
     *
     * This function cleans up the request target path to prevent directory
     * traversal attacks and ensures that the resulting path is valid.
     *
     * @param root_path The root path for serving files.
     * @param target The target path from the request.
     * @return fs::path The sanitized path for safe handling.
     */
    auto sanitize_target(const fs::path& root_path, const std::string& target) -> fs::path;

    /**
     * @brief Handle requests for directories.
     *
     * This function generates a response that lists the contents of a directory
     * if the requested path points to a directory.
     *
     * @param file_path The path to the directory.
     * @param res The HTTP response object to populate.
     */
    void handle_directory_request(const fs::path& file_path, http::response<http::string_body>& res);

    /**
     * @brief Handle requests for files that do not exist.
     *
     * This function generates a "404 Not Found" response when a requested file
     * or directory cannot be located.
     *
     * @param file_path The path that was requested but not found.
     * @param res The HTTP response object to populate.
     */
    void handle_not_found(const fs::path& file_path, http::response<http::string_body>& res);

    /**

     * @brief Handle requests for regular files.
     *
     * This function prepares the response for a file download, including
     * setting the appropriate headers.
     *
     * @param file_path The path to the file being requested.
     * @param res The HTTP response object to populate.
     * @param socket The TCP socket used for sending the file content.
     */
    void handle_file_request(const fs::path& file_path,
                             http::response<http::string_body>& res,
                             tcp::socket& socket);

    /**
     * @brief Configure the HTTP response for a file download.
     *
     * This function sets the necessary headers for the given file, such as
     * the content type and disposition.
     *
     * @param file_path The path of the file being requested.
     * @param res The HTTP response object to modify.
     */
    void configure_response_for_file(const fs::path& file_path,
                                      http::response<http::string_body>& res);

    /**
     * @brief Send the content of a file in chunks to the client.
     *
     * This function reads the requested file's contents and streams it
     * to the client over the provided socket.
     *
     * @param file The input file stream of the file being sent.
     * @param res The HTTP response object to populate.
     * @param socket The TCP socket used for sending the file content.
     */
    void send_file_content(std::ifstream& file,
                           http::response<http::string_body>& res,
                           tcp::socket& socket);

    /**
     * @brief Run the server to start accepting connections.
     *
     * This function enters the server's main loop, accepting and
     * processing incoming requests as they arrive.
     */
    void run_server();

    /**
     * @brief Root Path
     *
     * A reference to the directory path where the server serves files from.
     */
    fs::path& m_ROOT_PATH;

    /**
     * @brief Server Port
     *
     * The port on which the server listens for incoming connections.
     */
    std::uint16_t m_PORT;

    /**
     * @brief IO Context
     *
     * The I/O context for managing asynchronous operations in the server.
     */
    net::io_context m_DEFAULT_IOC;
};

