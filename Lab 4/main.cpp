#include <iostream>
#include <exception>
#include "colorspaces.hpp"

int main(int argc, char* argv[])
{
    Picture* new_pic = new PNM;
    try{
        new_pic->input(argc, argv);
        new_pic->make();
        new_pic->save();
    }
    catch(MyException& ex){
        std::cerr << ex.whats() << " " << ex.extra_info();
    }
    catch(std::exception& ex){
        std::cerr << ex.what();
    }
    delete new_pic;
    return 0;
}
