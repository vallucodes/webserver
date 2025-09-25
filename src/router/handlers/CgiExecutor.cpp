#include "CgiExecutor.hpp"
#include <filesystem> // for std::filesystem::path, std::filesystem::parent_path, std::filesystem::filename
#include <unistd.h> // for pipe, fork, dup2, close, write, read, chdir, execve, STDIN_FILENO, STDOUT_FILENO
#include <sys/wait.h> // for waitpid, WNOHANG, WIFEXITED, WEXITSTATUS
#include <signal.h> // for kill, SIGKILL
#include <cstdlib> // for std::stoul
// #include <fcntl.h> // for F_GETFL, F_SETFL, O_NONBLOCK
#include <ostream> // for std::ostream
#include <ctime> // for time, time_t
#include <iostream> // for std::cout
#include <sstream> // for std::istringstream
#include <algorithm> // for std::find_if

/** Execute CGI script and capture output */
std::string executeCgiScript(const std::string& scriptPath, const std::vector<std::string>& env, const std::string& input) {
    int pipe_in[2];  // For sending input to CGI
    int pipe_out[2]; // For receiving output from CGI

    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
        return "";
    }

    pid_t pid = fork();
    if (pid == -1) {
        // Fork failed, close pipes
        close(pipe_in[0]);
        close(pipe_in[1]);
        close(pipe_out[0]);
        close(pipe_out[1]);
        return "";
    }

    // Child process
    if (pid == 0) {
        // Close unused pipe ends
        close(pipe_in[1]);
        close(pipe_out[0]);

        // Redirect stdin to read from pipe_in
        if (dup2(pipe_in[0], STDIN_FILENO) == -1) {
            close(pipe_in[0]);
            close(pipe_out[1]);
            return "";
        }

        // Redirect stdout to write to pipe_out
        if (dup2(pipe_out[1], STDOUT_FILENO) == -1) {
            close(pipe_in[0]);
            close(pipe_out[1]);
            return "";
        }

        // Close original pipe descriptors
        close(pipe_in[0]);
        close(pipe_out[1]);

        // Fill up envp with environment variables
        std::vector<std::string> envStrings(env.begin(), env.end());
        std::vector<char*> envp;

        for (auto& var : envStrings) {
            // Convert to char*
            envp.push_back(const_cast<char*>(var.c_str()));
        }
        // Add nullptr to the end of the environment variables
        envp.push_back(nullptr);

        // Change to script directory for relative path access and get just the filename
        // example: /home/ilyam/42/webserver/www/cgi-bin/script.py -> /home/ilyam/42/webserver/www/cgi-bin
        std::filesystem::path scriptDir = std::filesystem::path(scriptPath).parent_path();
        // example: /home/ilyam/42/webserver/www/cgi-bin/script.py -> script.py
        std::string scriptName = std::filesystem::path(scriptPath).filename().string();

        if (!scriptDir.empty()) {
            // Change to script directory for relative path access
            chdir(scriptDir.c_str());
        }

        // Execute CGI script based on file extension using execve()
        size_t dotPos = scriptName.find_last_of('.');
        if (dotPos != std::string::npos) {
            std::string ext = scriptName.substr(dotPos + 1);
            // Convert to lowercase for case-insensitive comparison
            for (char& c : ext) {
                c = std::tolower(c);
            }

            if (ext == "py") {
                // Execute Python script with execve()
                // example: /home/ilyam/42/webserver/www/cgi-bin/script.py -> python3 script.py
                char* args[] = {
                    const_cast<char*>("python3"),
                    const_cast<char*>(scriptName.c_str()),
                    nullptr
                };
                execve("/usr/bin/python3", args, envp.data());

            } else if (ext == "js") {
                // Execute JavaScript script with Node.js using execve()
                // example: /home/ilyam/42/webserver/www/cgi-bin/script.js -> node script.js
                char* args[] = {
                    const_cast<char*>("node"),
                    const_cast<char*>(scriptName.c_str()),
                    nullptr
                };
                execve("/usr/bin/node", args, envp.data());

            } else {
                // For unknown extensions, try direct execution with execve()
                char* args[] = {
                    const_cast<char*>(scriptName.c_str()),
                    const_cast<char*>(scriptName.c_str()),
                    nullptr
                };
                execve(scriptName.c_str(), args, envp.data());
            }

        } else {
            // If no extension, try direct execution with execve()
            char* args[] = {
                const_cast<char*>(scriptName.c_str()),
                const_cast<char*>(scriptName.c_str()),
                nullptr
            };
            execve(scriptName.c_str(), args, envp.data());
        }
        // If execve fails, return empty string
        return "";
    } else { // Parent process
        // Close unused pipe ends
        close(pipe_in[0]);
        close(pipe_out[1]);

        // Send input to CGI (already processed for chunked requests)
        if (!input.empty()) {
            ssize_t bytesWritten = write(pipe_in[1], input.c_str(), input.length());
            if (bytesWritten == -1) {
                std::cout << "CGI: Failed to write input to CGI" << std::endl;
            }
        }
        close(pipe_in[1]); // Send EOF

        // Read output from CGI with timeout
        std::string output;
        char buffer[4096];
        ssize_t bytesRead;
        int timeout = 5; // 5 second timeout for infinite loop
        time_t startTime = time(nullptr);

        // Make pipe non-blocking
        while (time(nullptr) - startTime < timeout) {
        // Check if child has finished
        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);

        if (result == pid) {
            // Child finished, read remaining output
            while ((bytesRead = read(pipe_out[0], buffer, sizeof(buffer))) > 0) {
                output.append(buffer, bytesRead);
            }
            close(pipe_out[0]);
            // If child finished and exited successfully, return output
            return (WIFEXITED(status) && WEXITSTATUS(status) == 0) ? output : "";
        } else if (result == 0) {
            // Child still running, check if data available with select()
            // Set up readfds for select()
            fd_set readfds;
            // Zero out readfds
            FD_ZERO(&readfds);
            // Set up readfds for select()
            FD_SET(pipe_out[0], &readfds);

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 500000; // 5sec timeout

            // Check if data available with select()
            // the highest-numbered file descriptor to monitor, plus one (f0, f1, f2, ...)
            if (select(pipe_out[0] + 1, &readfds, nullptr, nullptr, &tv) > 0) {
                // Data available, read it
                bytesRead = read(pipe_out[0], buffer, sizeof(buffer));
                if (bytesRead > 0) {
                    output.append(buffer, bytesRead);
                }
            }
        }
    }

        close(pipe_out[0]);

        // Check if timeout occurred
        if (time(nullptr) - startTime >= timeout) {
            std::cout << "CGI: Script timeout after " << timeout << " seconds - killing process" << std::endl;
            kill(pid, SIGKILL);
            waitpid(pid, nullptr, 0); // Wait for cleanup
            return "";
        }

        // Wait for child to finish and get exit status
        int status;
        waitpid(pid, &status, 0);

        // If child finished and exited successfully, return output
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return output;
        } else {
            return "";
        }
    }
}

/** Execute CGI script and parse output into structured result */
CgiResult executeAndParseCgiScript(const std::string& scriptPath, const std::vector<std::string>& env, const std::string& input) {
    CgiResult result;

    // Execute CGI script to get raw output
    std::string cgiOutput = executeCgiScript(scriptPath, env, input);
    if (cgiOutput.empty()) {
        return result; // success = false by default
    }

    result.success = true;

    // Parse CGI output format: headers followed by blank line, then body
    size_t headerEnd = cgiOutput.find("\r\n\r\n");
    // If no headers, look for \n\n
    if (headerEnd == std::string::npos) {
        headerEnd = cgiOutput.find("\n\n");
    }

    std::string headersPart;
    std::string bodyPart;

    if (headerEnd != std::string::npos) {
        headersPart = cgiOutput.substr(0, headerEnd);
        // Extract body part after headers
        bodyPart = cgiOutput.substr(headerEnd + (cgiOutput[headerEnd] == '\r' ? 4 : 2));
    } else {
        // No headers, treat whole output as body
        bodyPart = cgiOutput;
    }

    // Parse headers from CGI output
    std::istringstream headerStream(headersPart);
    std::string headerLine;
    while (std::getline(headerStream, headerLine)) {
        // Remove \r if present
        if (!headerLine.empty() && headerLine.back() == '\r') {
            headerLine.pop_back();
        }

        size_t colonPos = headerLine.find(':');
        if (colonPos != std::string::npos) {
            std::string headerName = headerLine.substr(0, colonPos);
            std::string headerValue = headerLine.substr(colonPos + 1);

            // Full trim (leading and trailing whitespace)

            // Trim leading whitespace
            // Scans through the string from the start until it finds the first character that is not whitespace
            // [](int ch) -> bool: A lambda function (inline) that returns true when the char is not whitespace
            headerValue.erase(headerValue.begin(), std::find_if(headerValue.begin(), headerValue.end(), [](int ch) {
                return !std::isspace(ch);
            }));

            // Trim trailing whitespace
            // Scans through the string from the end until it finds the first character that is not whitespace
            // [](int ch) -> bool: A lambda function (inline) that returns true when the char is not whitespace
            headerValue.erase(std::find_if(headerValue.rbegin(), headerValue.rend(), [](int ch) {
                return !std::isspace(ch);
            }).base(), headerValue.end());

            // Special handling for Status header
            if (headerName == "Status") {
                result.status = "HTTP/1.1 " + headerValue;
            } else {
                result.headers[headerName] = headerValue;
            }
        }
    }

    result.body = bodyPart;

    // Set default status if not specified
    if (result.status.empty()) {
        result.status = "HTTP/1.1 200 OK";
    }

    return result;
}
