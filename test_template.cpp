#include <iostream>
#include <fstream>
#include <string>

std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

int main() {
    std::string templatePath = "/home/ilyam/42/webserver/www/autoindex_template.html";
    std::ifstream file(templatePath);

    if (!file.is_open()) {
        std::cout << "Error: Could not open template file" << std::endl;
        return 1;
    }

    std::string html((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::cout << "Template loaded successfully, size: " << html.size() << " bytes" << std::endl;

    // Test replacement
    html = replaceAll(html, "{{PATH}}", "/test");
    html = replaceAll(html, "{{PARENT_LINK}}", "");
    html = replaceAll(html, "{{ITEMS}}", "<div>Test item</div>");

    std::cout << "Template processed successfully" << std::endl;
    return 0;
}
