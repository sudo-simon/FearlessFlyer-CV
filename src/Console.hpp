#pragma once

#include <iostream>
#include <string>

class Console {

    private:
        std::string storedText;


    public:
        inline void PrintUI(std::string text);
        inline void Clear();
        inline std::string GetConsoleText() const;

        static void Log(std::string message){
            std::cout << message << std::endl;
        }
        static void LogError(std::string message){
            std::cerr << message << std::endl;
        }
};
