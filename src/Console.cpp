#include "Console.hpp"

inline void Console::PrintUI(std::string text){
    this->storedText.append(text+"\n");
}

inline void Console::Clear(){
    this->storedText.clear();
}

inline std::string Console::GetConsoleText() const{
    return this->storedText;
}