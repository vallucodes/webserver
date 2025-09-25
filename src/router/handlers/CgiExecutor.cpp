#include "CgiExecutor.hpp"
#include <filesystem> // for std::filesystem::path, std::filesystem::parent_path, std::filesystem::filename
#include <unistd.h> // for pipe, fork, dup2, close, write, read, chdir, execve, STDIN_FILENO, STDOUT_FILENO
#include <sys/wait.h> // for waitpid, WNOHANG, WIFEXITED, WEXITSTATUS
#include <signal.h> // for kill, SIGKILL
#include <cstdlib> // for std::stoul
#include <fcntl.h> // for fcntl, F_GETFL, F_SETFL, O_NONBLOCK
#include <ostream> // for std::ostream
#include <ctime> // for time, time_t
#include <iostream> // for std::cout




/** Execute CGI script and capture output */
std::string executeCgiScript(const std::string& scriptPath, const std::vector<std::string>& env, const std::string& input) {
    // std::cout << "CGI: executeCgiScript called for: " << scriptPath << std::endl;
    int pipe_in[2];  // For sending input to CGI
    int pipe_out[2]; // For receiving output from CGI

    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
        // std::cout << "CGI: Failed to create pipes" << std::endl;
        return "";
    }

    pid_t pid = fork();
    if (pid == -1) {
        // std::cout << "CGI: Fork failed" << std::endl;
        close(pipe_in[0]);
        close(pipe_in[1]);
        close(pipe_out[0]);
        close(pipe_out[1]);
        return "";
    }
    // std::cout << "CGI: Fork successful, child PID: " << pid << std::endl;

    if (pid == 0) { // Child process
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

        // Set up environment variables
        std::vector<std::string> envStrings(env.begin(), env.end());
        std::vector<char*> envp;
        for (auto& var : envStrings) {
            envp.push_back(const_cast<char*>(var.c_str()));
        }
        envp.push_back(nullptr);

        // Change to script directory for relative path access and get just the filename
        std::filesystem::path scriptDir = std::filesystem::path(scriptPath).parent_path();
        std::string scriptName = std::filesystem::path(scriptPath).filename().string();

        if (!scriptDir.empty()) {
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

            // std::cout << "CGI Child: Executing " << ext << " script: " << scriptName << " from dir: " << scriptDir << std::endl;

            if (ext == "py") {
                // Execute Python script with execve()
                char* args[] = {
                    const_cast<char*>("python3"),
                    const_cast<char*>(scriptName.c_str()),
                    nullptr
                };
                execve("/usr/bin/python3", args, envp.data());
            } else if (ext == "js") {
                // Execute JavaScript script with Node.js using execve()
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
            // No extension, try direct execution with execve()
            char* args[] = {
                const_cast<char*>(scriptName.c_str()),
                const_cast<char*>(scriptName.c_str()),
                nullptr
            };
            execve(scriptName.c_str(), args, envp.data());
        }
        std::cout << "CGI Child: execve failed" << std::endl;

        // Clean up environment variables on failure
        // No manual cleanup needed - RAII handles std::string and std::vector

        // If execve fails, return error
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
        int timeout = 5; // 5 second timeout
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
            return (WIFEXITED(status) && WEXITSTATUS(status) == 0) ? output : "";
        } else if (result == 0) {
            // Child still running, check if data available with select()
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(pipe_out[0], &readfds);

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 500000; // 5sec timeout

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

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return output;
        } else {
            return "";
        }
    }
}
