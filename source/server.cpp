#include <cstddef>
#include <cstdint>
#include <ctime>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "server.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/beast/core.hpp>
#include <boost/range/algorithm/sort.hpp>

#include "logger.hpp"
#include "tracelogger.hpp"

/**
 * @brief Anonymous namespace for helper functions
 *
 **/
namespace {
    /**
     * @brief Get the file type style object
     *
     * @param path
     * @return std::string
     **/
    auto get_file_type_style(const fs::path& path) -> std::string {
        if (fs::is_directory(path)) {
            return "font-weight: bold; color: #2196F3;";
        }
        if (path.extension() == ".mp4" || path.extension() == ".mp3" || path.extension() == ".jpg"
            || path.extension() == ".jpeg" || path.extension() == ".png" || path.extension() == ".gif"
            || path.extension() == ".avi" || path.extension() == ".mov" || path.extension() == ".wav")
        {
            return "color: #9C27B0;";
        }
        if (path.extension() == ".exe" || path.extension() == ".bat" || path.extension() == ".msi"
            || path.extension() == ".sh")
        {
            return "color: #FF9800;";
        } else if (path.extension() == ".zip" || path.extension() == ".tar" || path.extension() == ".gz"
                   || path.extension() == ".rar" || path.extension() == ".7z")
        {
            return "color: #4CAF50;";
        }
        return "color: #FFFFFF;";
    }

    /**
     * @brief Construct CSS Styles
     *
     * @return std::string
     **/
    auto construct_css_styles() -> std::string {
        std::string const STYLES = R"(
<style>
    * {
        box-sizing: border-box;
    }
    body {
        background-color: #1f1f1f;
        color: #FFFFFF;
        font-size: 16px;
        font-family: 'Arial', sans-serif;
        margin: 0;
        padding: 20px;
    }
    h1 {
        color: #90CAF9;
        font-size: 32px;
        text-align: center;
        margin: 10px 0;
    }
    h2 {
        color: #FFCC00;
        font-size: 24px;
        margin: 20px 0;
    }
    table {
        width: 100%;
        border-collapse: collapse;
        margin: 20px 0;
        background-color: #2f2f2f;
        border: 1px solid #3C3C3C;
    }
    th {
        background-color: #3C3C3C;
        color: #FFFFFF;
        padding: 12px;
    }
    td {
        background-color: #2f2f2f;
        color: #DDDDDD;
        padding: 12px;
        border: 1px solid #3C3C3C;
    }
    a {
        color: #FFCC00;
        text-decoration: underline;
    }
    a:hover {
        color: #FFD54F;
        text-decoration: underline;
    }
    .parent {
        font-weight: bold;
        color: #90CAF9;
    }
    .footer {
        text-align: center;
        margin: 20px 0;
        font-size: 14px;
        color: #AAAAAA;
    }
    .name-col {
        width: 25%;
    }
    .link-col {
        width: 55%;
    }
    .date-col {
        width: 20%;
    }
</style>
        )";

        return STYLES;
    }
}    // namespace

/**
 * @brief Construct a new SHServer::SHServer object
 *
 * @param root_path root path
 * @param port server port
 **/
SHServer::SHServer(fs::path& root_path, std::uint16_t& port)
    : m_ROOT_PATH(root_path)
    , m_PORT(port) {
    LOG_TRACE

    run_server();
}

/**
 * @brief Generate file list
 *
 * @param current_path current path
 * @return std::string html page
 **/
auto SHServer::generate_file_list(const fs::path& current_path) -> std::string {
    LOG_TRACE

    std::string const BASE_LINK = fs::relative(current_path, m_ROOT_PATH).string();
    log_debug("Generate file list HTML page for: %s\n", BASE_LINK.c_str());

    std::string const STYLES = construct_css_styles();

    std::string html = "<html>" + STYLES + "<body><h1>Files in: " + BASE_LINK + "</h1><br><hr><br>";

    if (current_path != m_ROOT_PATH) {
        fs::path const PARENT_PATH = current_path.parent_path();
        std::string const PARENT_LINK = fs::relative(PARENT_PATH, m_ROOT_PATH).string();
        html += "<a class='parent' href=\"" + PARENT_LINK + "\">Back to Parent Directory</a><br><br>";
    }

    std::vector<std::pair<fs::path, std::time_t>> entries;

    size_t dir_count = 0;
    size_t file_count = 0;

    for (const auto& entry : fs::directory_iterator(current_path)) {
        entries.emplace_back(entry.path(), fs::last_write_time(entry));
        if (fs::is_directory(entry)) {
            dir_count++;
        } else {
            file_count++;
        }
    }

    boost::range::sort(entries,
                       [](const auto& a, const auto& b)
                       {
                           if (fs::is_directory(a.first) && !fs::is_directory(b.first)) {
                               return true;
                           }
                           if (!fs::is_directory(a.first) && fs::is_directory(b.first)) {
                               return false;
                           }
                           return a.first.filename() < b.first.filename();
                       });

    html += "<h2>Summary Information</h2>";
    html += "<p>Total Directories: " + std::to_string(dir_count) + "</p>";
    html += "<p>Total Files: " + std::to_string(file_count) + "</p>";
    html += "<hr>";

    std::time_t const CURRENT_TIME = std::time(nullptr);
    std::string current_time_str = std::asctime(std::localtime(&CURRENT_TIME));
    current_time_str.erase(current_time_str.length() - 1);
    html += "<p>Current Server Time: " + current_time_str + "</p>";
    html += "<hr>";

    html +=
        "<table><tr><th>N</th><th class='name-col'>NAME</th><th class='link-col'>LINK</th><th "
        "class='date-col'>DATE</th></tr>";

    int index = 1;
    for (const auto& entry : entries) {
        std::string const NAME = entry.first.filename().string();
        std::string const LINK = fs::relative(entry.first, m_ROOT_PATH).string();
        std::time_t const MOD_TIME = entry.second;
        std::string date_str = std::asctime(std::localtime(&MOD_TIME));
        date_str.erase(date_str.length() - 1);

        html += "<tr>";
        html += "<td>" + std::to_string(index++) + "</td>";
        html += "<td class='name-col' style='" + get_file_type_style(entry.first) + "'>" + NAME
            + (fs::is_directory(entry.first) ? "/" : "") + "</td>";
        html += "<td class='link-col'><a href=\"" + LINK + "\">" + NAME + "</a></td>";
        html += "<td class='date-col'>" + date_str + "</td>";
        html += "</tr>";
    }

    html += "</table><br><hr><br>";
    html +=
        "<p class='footer'>For more, visit <a href='https://github.com/alexeev-prog/http-fileserver-cpp'>the "
        "repository</a>. &copy; 2025 Alexeev Bronislaw</p>";
    html += "</body></html>";

    return html;
}

/**
 * @brief Process the HTTP request by routing it to the appropriate handler.
 *
 * This function coordinates the handling of HTTP requests and sends corresponding responses based on the type
 * of request.
 *
 * @param root_path The root directory where files are served from.
 * @param req The HTTP request object.
 * @param res The HTTP response object.
 * @param socket The TCP socket for communication.
 */
void SHServer::handle_request(const fs::path& root_path,
                              http::request<http::string_body>& req,
                              http::response<http::string_body>& res,
                              tcp::socket& socket) {
    LOG_TRACE

    std::string const target = std::string(req.target());
    log_info("Handle request for target: %s\n", target.c_str());

    if (target.empty() || target == "/") {
        SHServer::handle_root_request(root_path, res);
    } else {
        fs::path const file_path = sanitize_target(root_path, target);
        if (fs::is_directory(file_path)) {
            handle_directory_request(file_path, res);
        } else if (!fs::exists(file_path) || !fs::is_regular_file(file_path)) {
            handle_not_found(file_path, res);
        } else {
            handle_file_request(file_path, res, socket);
        }
    }
}

/**
 * @brief Handle requests for the root directory.
 *
 * @param root_path The root directory.
 * @param res The HTTP response object.
 */
void SHServer::handle_root_request(const fs::path& root_path, http::response<http::string_body>& res) {
    res.result(http::status::ok);
    res.body() = generate_file_list(root_path);
    res.set(http::field::content_type, "text/html");
}

/**
 * @brief Sanitize the target path for secure access.
 *
 * @param root_path The root directory.
 * @param target The target path from the request.
 * @return The sanitized file path.
 */
auto SHServer::sanitize_target(const fs::path& root_path, const std::string& target) -> fs::path {
    std::string const sanitized_target = target.substr(1);    // Remove leading '/'
    return root_path / sanitized_target;
}

/**
 * @brief Handle requests for a directory.
 *
 * @param file_path The path to the directory.
 * @param res The HTTP response object.
 */
void SHServer::handle_directory_request(const fs::path& file_path, http::response<http::string_body>& res) {
    res.result(http::status::ok);
    res.body() = generate_file_list(file_path);
    res.set(http::field::content_type, "text/html");
}

/**
 * @brief Handle requests for non-existing files.
 *
 * @param file_path The path that does not exist.
 * @param res The HTTP response object.
 */
void SHServer::handle_not_found(const fs::path& file_path, http::response<http::string_body>& res) {
    log_debug("File path %s does not exist\n", file_path.c_str());
    res.result(http::status::not_found);
    res.body() = "File not found";
}

/**
 * @brief Handle requests for regular files.
 *
 * @param file_path The path to the file.
 * @param res The HTTP response object.
 * @param socket The TCP socket for communication.
 */
void SHServer::handle_file_request(const fs::path& file_path,
                                   http::response<http::string_body>& res,
                                   tcp::socket& socket) {
    log_debug("Attempting to open file: %s\n", file_path.c_str());

    std::ifstream file(file_path.string(), std::ios::binary);
    if (!file) {
        log_debug("Failed to open file: %s\n", file_path.c_str());
        res.result(http::status::internal_server_error);
        res.body() = "Failed to open file";
        return;
    }

    configure_response_for_file(file_path, res);
    send_file_content(file, res, socket);
}

/**

 * @brief Configure the HTTP response for a file download.
 *
 * @param file_path The path of the file being requested.
 * @param res The HTTP response object.
 */
void SHServer::configure_response_for_file(const fs::path& file_path,
                                           http::response<http::string_body>& res) {
    res.result(http::status::ok);
    res.set(http::field::content_type, "application/octet-stream");
    res.set(http::field::content_disposition,
            "attachment; filename=\"" + file_path.filename().string() + "\"");
}

/**
 * @brief Send the content of the file in chunks through the socket.
 *
 * @param file The input file stream.
 * @param res The HTTP response object.
 * @param socket The TCP socket for communication.
 */
void SHServer::send_file_content(std::ifstream& file,
                                 http::response<http::string_body>& res,
                                 tcp::socket& socket) {
    constexpr size_t buffer_size = 8192;
    char buffer[buffer_size];
    log_debug("Open buffer (%zu) for file transfer\n", buffer_size);

    try {
        while (file) {
            file.read(buffer, buffer_size);
            std::streamsize const bytes_read = file.gcount();

            if (bytes_read > 0) {
                res.body() = std::string(buffer, buffer + bytes_read);
                http::write(socket, res);
            }
        }
    } catch (const std::exception& e) {
        log_error("Error reading or sending file: %s\n", e.what());
        res.result(http::status::internal_server_error);
        res.body() = "Error reading or sending file: " + std::string(e.what());
    }
}

/**
 * @brief Run HTTP Server
 *
 **/
void SHServer::run_server() {
    LOG_TRACE

    try {
        net::io_context ioc;

        tcp::acceptor acceptor(ioc, {tcp::v4(), m_PORT});
        std::cout << "Localhost Server started at port " << m_PORT << "\n";

        log_info("HTTP Fileserver started at 127.0.0.1:%d\n", m_PORT);

        while (true) {
            tcp::socket socket(ioc);

            acceptor.accept(socket);

            beast::flat_buffer buffer;
            http::request<http::string_body> req;
            http::response<http::string_body> res;

            try {
                http::read(socket, buffer, req);
            } catch (const boost::system::system_error& e) {
                if (e.code() == boost::beast::http::error::end_of_stream) {
                    log_error("Client disconnected: %s\n", e.what());
                    std::cerr << "Client disconnected: " << e.what() << "\n";
                    continue;
                }
                std::cerr << "Error: " << e.what() << "\n";
                log_error("Error: %s\n", e.what());
                continue;
            }

            handle_request(m_ROOT_PATH, req, res, socket);

            try {
                http::write(socket, res);
            } catch (const boost::system::system_error& e) {
                if (e.code() == boost::asio::error::broken_pipe) {
                    log_error("Client disconnected: %s", e.what());
                    std::cerr << "Client disconnected: " << e.what() << "\n";
                } else {
                    log_error("Error: %s\n", e.what());
                    std::cerr << "Error: " << e.what() << "\n";
                }
            }
        }
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}
