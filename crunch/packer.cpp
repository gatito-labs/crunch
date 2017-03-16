/*
 
 MIT License
 
 Copyright (c) 2017 Chevy Ray Johnston
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 
 */

#include "packer.hpp"
#include "MaxRectsBinPack.h"
#include "GuillotineBinPack.h"
#include "binary.hpp"
#include <iostream>
#include <algorithm>

using namespace std;
using namespace rbp;

Packer::Packer(int width, int height)
: width(width), height(height)
{
    
}

void Packer::Pack(vector<Bitmap*>& bitmaps, bool verbose, int reduce)
{
    MaxRectsBinPack packer(width, height);
    
    int ww = 0;
    int hh = 0;
    while (!bitmaps.empty())
    {
        auto bitmap = bitmaps.back();
        
        if (verbose)
            cout << '\t' << bitmaps.size() << ": " << bitmap->name << endl;
        
        //Check to see if this is a duplicate of an already packed bitmap
        auto di = dupLookup.find(bitmap->hashValue);
        if (reduce > 0 && di != dupLookup.end() && (reduce < 2 || bitmap->Equals(bitmaps[di->second])))
        {
            Point p = points[di->second];
            points.push_back({ p.x, p.y, di->second });
        }
        else
        {
            Rect rect = packer.Insert(bitmap->width + 1, bitmap->height + 1, MaxRectsBinPack::RectBestShortSideFit);
            
            if (rect.width == 0 || rect.height == 0)
                return;
            
            if (reduce > 0)
                dupLookup[bitmap->hashValue] = static_cast<int>(points.size());
            points.push_back({ rect.x, rect.y, -1 });
            
            ww = max(rect.x + rect.width, ww);
            hh = max(rect.y + rect.height, hh);
        }
        
        this->bitmaps.push_back(bitmap);
        bitmaps.pop_back();
    }
    
    while (width / 2 >= ww)
        width /= 2;
    while( height / 2 >= hh)
        height /= 2;
}

void Packer::SavePng(const string& file)
{
    Bitmap bitmap(width, height);
    for (size_t i = 0, j = bitmaps.size(); i < j; ++i)
        if (points[i].dupID < 0)
            bitmap.CopyPixels(bitmaps[i], points[i].x, points[i].y);
    bitmap.SaveAs(file);
}

void Packer::SaveXml(const string& name, ofstream& xml, bool trim)
{
    xml << "\t<tex n=\"" << name << "\">" << endl;
    for (size_t i = 0, j = bitmaps.size(); i < j; ++i)
    {
        xml << "\t\t<img n=\"" << bitmaps[i]->name << "\" ";
        xml << "x=\"" << points[i].x << "\" ";
        xml << "y=\"" << points[i].y << "\" ";
        xml << "w=\"" << bitmaps[i]->width << "\" ";
        xml << "h=\"" << bitmaps[i]->height << "\" ";
        if (trim)
        {
            xml << "fx=\"" << bitmaps[i]->frameX << "\" ";
            xml << "fy=\"" << bitmaps[i]->frameY << "\" ";
            xml << "fw=\"" << bitmaps[i]->frameW << "\" ";
            xml << "fh=\"" << bitmaps[i]->frameH << "\" ";
        }
        xml << "/>" << endl;
    }
    xml << "\t</tex>" << endl;
}

void Packer::SaveBin(const string& name, ofstream& bin, bool trim)
{
    WriteString(bin, name);
    WriteShort(bin, (int16_t)bitmaps.size());
    for (size_t i = 0, j = bitmaps.size(); i < j; ++i)
    {
        WriteString(bin, bitmaps[i]->name);
        WriteShort(bin, (int16_t)points[i].x);
        WriteShort(bin, (int16_t)points[i].x);
        WriteShort(bin, (int16_t)bitmaps[i]->width);
        WriteShort(bin, (int16_t)bitmaps[i]->height);
        if (trim)
        {
            WriteShort(bin, (int16_t)bitmaps[i]->frameX);
            WriteShort(bin, (int16_t)bitmaps[i]->frameY);
            WriteShort(bin, (int16_t)bitmaps[i]->frameW);
            WriteShort(bin, (int16_t)bitmaps[i]->frameH);
        }
    }
}
