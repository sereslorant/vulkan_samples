
#include "lFileUtils.h"

#include <iostream>

#include <fstream>

std::vector<char> lLoadBinaryFile(const std::string &filename)
{
    std::vector<char> Result;

    std::ifstream fin(filename,std::ios::ate | std::ios::binary);
    if(fin.is_open())
    {
        std::size_t FileSize = fin.tellg();
        Result.resize(FileSize);
        
        fin.seekg(0);
        fin.read(Result.data(),FileSize);

        return Result;
    }
    else
    {
        std::cerr << "Couldn't open " << filename << "\n";
        return {};
    }
}

std::string lLoadTextFile(const std::string &filename)
{
    std::ifstream In;
    In.open(filename.c_str());

    if(In.is_open())
    {
        std::string RetVal;

        while(!In.eof())
        {
            std::string Tmp;
            getline(In,Tmp);

            RetVal += Tmp;
            RetVal += "\n";
        }
        
        return RetVal;
    }
    else
    {
        std::cerr << "Couldn't open " << filename << "\n";
        return {};
    }
}
