//? Local modules to be tested ----------
#include "modules/GUI/GUI_demo.cpp"
#include "modules/network/Example.cpp"
#include "modules/network/NetConf.cpp"
//? -------------------------------------

#include <iostream>
#include <string>
#include <vector>

using namespace std;



int main(int argc, char** argv){

    //? Default help message, update when needed
    if (argc < 2){
        cout <<
        "Pass one of the following arguments:\n"
        "\t--gui : test the GUI module\n"
        "\t--net : test the network module\n"
        << endl;
        exit(0);
    }

    
    //? Function pointers to be called via passed arguments, update when needed
    vector<int(*)()> function_pointers = {
        GUI_demo, 
        Network_demo
    };

    int function_i = -1;


    //? Argument parsing
    //? L'ho fatto con un ciclo così se vogliamo poi possiamo passare più di un argomento
    for (int i=1; i<argc; ++i){
        string arg = (string) argv[i];

        if (arg == "--gui"){
            function_i = 0;
            break;
        }
        if (arg == "--net"){
            function_i = 1;
            break;
        }
    }


    //? No valid function argument
    if (function_i == -1){
        cout << "Invalid argument, aborting" << endl;
        exit(1);
    }

    //? Actual call to correct function
    function_pointers[function_i]();

    return 0;
}