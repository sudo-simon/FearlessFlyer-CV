#pragma once

#include <iostream>
#include <string>

class Console {

    private:
        std::string storedText;


    public:
        inline void PrintUI(std::string text){ this->storedText.append(text+"\n"); }   

        inline void Clear(){ this->storedText.clear();}

        inline std::string GetConsoleText() const{ return this->storedText; }

        static void Log(std::string message){
            std::cout << message << std::endl;
        }
        static void LogError(std::string message){
            std::cerr << message << std::endl;
        }
};
