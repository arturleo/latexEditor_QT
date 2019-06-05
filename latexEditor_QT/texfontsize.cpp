#include "texfontsize.h"

texFontSize::texFontSize(double defaultsize):
    m_size(defaultsize)
{

}

int texFontSize::getIndex(double size)
{
    if(size<(rule[0]+0.1)*m_size)
        return 0;
    else if(size<(rule[1]+0.05)*m_size)
        return 1;
    else if(size<(rule[2]+0.05)*m_size)
        return 2;
    else if(size<(rule[3]+0.05)*m_size)
        return 3;
    else if(size<(rule[4]+0.1)*m_size)
        return 4;
    else if(size<(rule[5]+0.12)*m_size)
        return 5;
    else if(size<(rule[6]+0.144)*m_size)
        return 6;
    else if(size<(rule[7]+0.173)*m_size)
        return 7;
    else if(size<(rule[8]+0.207)*m_size)
        return 8;
    else
        return 9;
}

QString texFontSize::getname(double size)
{
    int index=getIndex(size);
    return rules[index];
}

double texFontSize::bigger(double size)
{
    int index=getIndex(size);
    return index==9?rule[index]:rule[index+1];
}

double texFontSize::smaller(double size)
{
    int index=getIndex(size);
    return index==0?rule[index]:rule[index-1];
}
