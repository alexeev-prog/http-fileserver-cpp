#include <cstddef>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>

#include "server.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/beast/core.hpp>

SHServer::SHServer(fs::path& root_path, unsigned short& port)
    : m_ROOT_PATH(root_path)
    , m_PORT(port) {
    run_server();
}

auto SHServer::generate_file_list(const fs::path& current_path) -> std::string {
    std::string const styles =
        "<style>body {background:rgb(207, 207, 207); font-family: monospace; font-size: 18px} div.links "
        "{background: #ebdbb2; color: #111111; padding: 15px} h1 {color: darkcyan; font-size: 38px} a.parent "
        "{ font-size: 24px; padding: 10px }</style>";

    std::string html = "<html>" + styles + "<body><h1>Files:</h1><br><hr><br><ol>";

    if (current_path != m_ROOT_PATH) {
        fs::path const parent_path = current_path.parent_path();
        std::string const parent_link = fs::relative(parent_path, m_ROOT_PATH).string();
        html += "<li><a class='parent' href=\"" + parent_link + "\">.. (Parent Directory)</a></li>";
    }

    html += "<div class='links'>";

    for (const auto& entry : fs::directory_iterator(current_path)) {
        std::string const name = entry.path().filename().string();
        std::string const link = fs::relative(entry.path(), m_ROOT_PATH).string();

        if (fs::is_directory(entry)) {
            html += "<li><a href=\"" + link + "\">" + name + "/</a></li>";
        } else {
            html += "<li><a href=\"" + link + "\">" + name + "</a></li>";
        }
    }

    html += "</div>";

    html += "</ol></body></html>";

    return html;
}

void SHServer::handle_request(const fs::path& root_path,
                              http::request<http::string_body>& req,
                              http::response<http::string_body>& res,
                              tcp::socket& socket) {
    std::string target = std::string(req.target());

    if (target.empty() || target == "/") {
        res.result(http::status::ok);
        res.body() = generate_file_list(root_path);
        res.set(http::field::content_type, "text/html");
        return;
    }

    target.erase(0, 1);

    fs::path const file_path = root_path / target;

    if (fs::is_directory(file_path)) {
        res.result(http::status::ok);
        res.body() = generate_file_list(file_path);
        res.set(http::field::content_type, "text/html");
        return;
    }

    if (!fs::exists(file_path) || !fs::is_regular_file(file_path)) {
        res.result(http::status::not_found);
        res.body() = "File not found";
        return;
    }

    std::ifstream file(file_path.string(), std::ios::binary);
    if (!file) {
        res.result(http::status::internal_server_error);
        res.body() = "Failed to open file";
        return;
    }

    res.result(http::status::ok);
    res.set(http::field::content_type, "application/octet-stream");
    res.set(http::field::content_disposition,
            "attachment; filename=\"" + file_path.filename().string() + "\"");

    constexpr size_t buffer_size = 8192;
    char buffer[buffer_size];

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
        res.result(http::status::internal_server_error);
        res.body() = "Error reading or sending file: " + std::string(e.what());
        return;
    }
}

void SHServer::run_server() {
    try {
        net::io_context ioc;

        tcp::acceptor acceptor(ioc, {tcp::v4(), m_PORT});
        std::cout << "Localhost Server started at port " << m_PORT << "\n";

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
                    std::cerr << "Client disconnected: " << e.what() << "\n";
                    continue;
                }
                std::cerr << "Error: " << e.what() << "\n";
                continue;
            }

            handle_request(m_ROOT_PATH, req, res, socket);

            try {
                http::write(socket, res);
            } catch (const boost::system::system_error& e) {
                if (e.code() == boost::asio::error::broken_pipe) {
                    std::cerr << "Client disconnected: " << e.what() << "\n";
                } else {
                    std::cerr << "Error: " << e.what() << "\n";
                }
            }
        }
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}
