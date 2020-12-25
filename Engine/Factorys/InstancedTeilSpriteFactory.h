#ifndef _InstancedTeilSpriteFactory_H_
#define _InstancedTeilSpriteFactory_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "../DrawableRenderer/DrawableRenderer.h"

namespace Factory {

    struct float6 {
        float a;
        float b;
        float c;
        float d;
        float e;
        float f;
    };

	std::vector<XMFLOAT4> RectVectorBuilder(const char* describerPath) {
        std::vector<XMFLOAT4> toRet = std::vector<XMFLOAT4>();
        std::string line;
        std::ifstream myfile(describerPath);
        if (myfile.is_open())
        {
            std::getline(myfile, line);
            std::string temps = line.substr(0, line.find(", "));

            int width, height, a, b, c, d;

            width = std::stoi(line.substr(0, line.find(", ")));
            line = line.substr(line.find(", ") + 1);
            height = std::stoi(line.substr(0, line.find(", ")));

            while (std::getline(myfile, line))
            {
                XMFLOAT4 tempf;

                a = std::stoi(line.substr(0, line.find(", ")));
                line = line.substr(line.find(", ") + 1);
                b = std::stoi(line.substr(0, line.find(", ")));
                line = line.substr(line.find(", ") + 1);
                c = std::stoi(line.substr(0, line.find(", ")));
                line = line.substr(line.find(", ") + 1);
                d = std::stoi(line.substr(0, line.find(", ")));
                line = line.substr(line.find(", ") + 1);

                tempf.x = (float)(c - a) / (float)width;
                tempf.y = (float)(d - b) / (float)height;
                tempf.z = (float)a / (float)width;
                tempf.w = (float)b / (float)height;

                toRet.push_back(tempf);
            }
            myfile.close();
        }

        return toRet;
    }
    std::vector<float6> charVectorBuilder(const char* describerPath) {
        std::vector<float6> toRet = std::vector<float6>();
        std::string line;
        std::ifstream myfile(describerPath);
        if (myfile.is_open())
        {
            std::getline(myfile, line);
            std::string temps = line.substr(0, line.find(", "));

            int imgWidth, imgHeight, width, height, a, b, c, d;
            float widthH, heightH;

            imgWidth = std::stoi(line.substr(0, line.find(", ")));
            line = line.substr(line.find(", ") + 1);
            imgHeight = std::stoi(line.substr(0, line.find(", ")));
            line = line.substr(line.find(", ") + 1);
            width = std::stoi(line.substr(0, line.find(", ")));
            line = line.substr(line.find(", ") + 1);
            height = std::stoi(line.substr(0, line.find(", ")));

            widthH = (float)width / 2.0f;
            heightH = (float)height / 2.0f;

            while (std::getline(myfile, line))
            {
                float6 tempf;

                a = std::stoi(line.substr(0, line.find(", ")));
                line = line.substr(line.find(", ") + 1);
                b = std::stoi(line.substr(0, line.find(", ")));
                line = line.substr(line.find(", ") + 1);
                c = std::stoi(line.substr(0, line.find(", ")));
                line = line.substr(line.find(", ") + 1);
                d = std::stoi(line.substr(0, line.find(", ")));
                line = line.substr(line.find(", ") + 1);

                tempf.a = (float)(c) / (float)imgWidth;
                tempf.b = (float)(d) / (float)imgHeight;
                tempf.c = (float)(a - c/2) / (float)imgWidth;
                tempf.d = (float)(b - d/2) / (float)imgHeight;
                tempf.e = c;
                tempf.f = d;

                toRet.push_back(tempf);
            }
            myfile.close();
        }

        return toRet;
    }
}
#endif