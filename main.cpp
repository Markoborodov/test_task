#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>
#include <functional>
#include <algorithm>

class Command {
    std::function<char(char)> fun;
public:
    explicit Command(std::function<char(char)>&& f) :fun(f) { }

    void apply(std::string& str) const
    {
        std::transform(str.begin(), str.end(), str.begin(), fun);
    }
};

Command make_command(const std::string& str)
{
    switch (str[0]) {
        case 'u':
            return Command([](char c) {
                return std::tolower(c);
            });
        case 'U':
            return Command([](char c) {
                return std::toupper(c);
            });
        case 'R': {
            char first = str[1], second = str[2];
            return Command([first, second](char c) {
                return (first == c) ? second : c;
            });
        }
        default:
            return Command([](char c){
                return c;
            });
    }
}

class Converter {
    const std::map<unsigned, std::vector<Command>>& commands;
public:
    explicit Converter(std::map<unsigned, std::vector<Command>>&& commands)
    :commands(commands) { }

    void convert_file(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
            throw std::runtime_error("Bad filename!");
        for (std::string line; std::getline(file, line);) {
            unsigned num = 0;
            std::stringstream ss(line);
            for (std::string field; std::getline(ss, field, '\t');) {
                auto vec = commands.find(num++);
                if (vec != commands.end())
                    for (const auto& cmd : vec->second)
                        cmd.apply(field);
                std::cout << field;
                if (!ss.eof())
                    std::cout << '\t';
            }
            std::cout << std::endl;
        }
        if (file.bad())
            throw std::runtime_error("Reading error!");
    }
};

int main(int argc, char* argv[])
{
    std::map<unsigned, std::vector<Command>> commands;
    for (int i = 2; i < argc; i++) {
        std::string cmd(argv[i]);
        auto pos = cmd.find(':');
        unsigned num = std::stoul(cmd.substr(0, pos));
        auto& vec = commands[num];
        vec.push_back(make_command(cmd.substr(pos + 1)));
    }
    Converter converter(std::move(commands));
    try {
        converter.convert_file(argv[1]);
    }
    catch(std::exception& exc)
    {
        std::cerr << exc.what();
        return 1;
    }
    return 0;
}
