#ifndef _MYGL_H_
#define _MYGL_H_

#include <stdexcept>
#include <utility>
#include <tuple>
#include <glm/glm.hpp>

#include "definitions.h"

//*****************************************************************************
// Defina aqui as suas funções gráficas
//*****************************************************************************
//
#define PI 3.1415926535897932384626433832f
#define EPSILON 0.0000001f

struct PixelColor {
    unsigned char r, g, b, a;
    PixelColor(unsigned char r,
                unsigned char g,
                unsigned char b, 
                unsigned char a) : r(r), g(g), b(b), a(a) {}
};

struct PixelCoord {
    unsigned int x, y;
    PixelCoord(unsigned int x, unsigned int y) : x(x), y(y) {}
};

inline PixelCoord convertCoord(glm::vec3 p) {
    return {static_cast<unsigned int>(p.x*((IMAGE_WIDTH-1)/2.0f)+((IMAGE_WIDTH-1)/2.0f)), 
            static_cast<unsigned int>(p.y*-1.0f*((IMAGE_HEIGHT-1)/2.0f)+((IMAGE_HEIGHT-1)/2.0f))};
}

inline PixelColor convertColor(glm::vec4 c) {
    return {static_cast<unsigned char>(c.r * 255),
            static_cast<unsigned char>(c.g * 255),
            static_cast<unsigned char>(c.b * 255),
            static_cast<unsigned char>(c.a * 255)};
}

void PutPixel(PixelCoord p, PixelColor c) {
    if(p.x >= IMAGE_WIDTH || p.y >= IMAGE_HEIGHT) 
        throw std::length_error("Frame buffer acess out of bounds");

    unsigned int index = (p.x + p.y * IMAGE_WIDTH) * 4;
    FBptr[index    ] = c.r;
    FBptr[index + 1] = c.g;
    FBptr[index + 2] = c.b;
    FBptr[index + 3] = c.b;
}

void ClearBuffer(glm::vec4 c) {
    for(unsigned int i = 0; i < IMAGE_WIDTH; i++)
        for(unsigned int j = 0; j < IMAGE_HEIGHT; j++) {
            PutPixel({i, j}, convertColor(c));
        }
}

void DrawLine(PixelCoord p0, PixelCoord p1, glm::vec4 c0, glm::vec4 c1) {
    int dx = p1.x - p0.x;
    int dy = p1.y - p0.y;

    int dxtmp = dx;

    bool interpolate = false;
    glm::vec4 incC;
    if(c0 != c1) {
        if (dx > 0)
            incC = (c1 - c0)/(float)dx;
        else
            incC = (c0 - c1)/(float)dx;

        interpolate = true;
    }

    int incX = 1, incY = 1;
    if(dy < 0) { 
        dy = -dy; 
        incY = -1; 
    } 
    if(dx < 0) { 
        dx = -dx; 
        incX = -1; 
    } 


    bool invert = false;
    if(abs(dy) > abs(dx)) {
        std::swap(dx, dy);
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        invert = true;
    }


    int incE  = 2 * dy;
    int incNE = 2 * (dy - dx);
    int d = 2*dy - dx;

    int currX = p0.x, currY = p0.y;
    while(incX*currX <= incX*p1.x) {
        if(invert)
            PutPixel({(unsigned int)currY, (unsigned int)currX}, convertColor(c0));
        else 
            PutPixel({(unsigned int)currX, (unsigned int)currY}, convertColor(c0));

        if(d > 0) {
            d += incNE;
            currX += incX; currY += incY;
        } else {
            d += incE;
            currX += incX;
        }

        if(interpolate) 
            c0 += incC;
    }
}

struct Vertex {
    glm::vec2 texcoord;
    glm::vec3 pos;
    glm::vec4 color;
};

/*
void DrawTriangle(Vertex v0, Vertex v1, Vertex v2) {
    PixelCoord p0 = convertCoord(v0);
    PixelCoord p1 = convertCoord(v1);
    PixelCoord p2 = convertCoord(v2);

    DrawLine(v0.pos, v1.pos, v0.color, v1.color);
    DrawLine(v1.pos, v2.pos, v1.color, v2.color);
    DrawLine(v2.pos, v0.pos, v2.color, v0.color);
}
*/

void DrawBottomFlatTriangle(Vertex v0, Vertex v1, Vertex v2) {
    if (v1.pos.x > v2.pos.x)
        std::swap(v1.pos, v2.pos);

    PixelCoord p0 = convertCoord(v0.pos);
    PixelCoord p1 = convertCoord(v1.pos);
    PixelCoord p2 = convertCoord(v2.pos);

    float invSlope1 = ((float)p1.x - p0.x) / ((float)p1.y - p0.y);
    float invSlope2 = ((float)p2.x - p0.x) / ((float)p2.y - p0.y);

    float currX1 = p0.x;
    float currX2 = p0.x;

    glm::vec4 c0 = v0.color;
    glm::vec4 c1 = v0.color;

    float dy = p1.y - p0.y;
    glm::vec4 incC0 = (v1.color - v0.color) / dy;
    glm::vec4 incC1 = (v2.color - v0.color) / dy;

    for (unsigned int scanLineY = p0.y; scanLineY <= p1.y; scanLineY++) {
        DrawLine({(unsigned int) currX1, scanLineY}, 
                 {(unsigned int) currX2, scanLineY}, c0, c1);
        currX1 += invSlope1;
        currX2 += invSlope2;

        c0 += incC0;
        c1 += incC1;
    }
}

void DrawTopFlatTriangle(Vertex v0, Vertex v1, Vertex v2) {
    if (v0.pos.x > v1.pos.x)
        std::swap(v0.pos, v1.pos);

    PixelCoord p0 = convertCoord(v0.pos);
    PixelCoord p1 = convertCoord(v1.pos);
    PixelCoord p2 = convertCoord(v2.pos);

    float invSlope1 = ((float)p2.x - p0.x) / ((float)p2.y - p0.y);
    float invSlope2 = ((float)p2.x - p1.x) / ((float)p2.y - p1.y);

    float currX1 = p2.x;
    float currX2 = p2.x;

    glm::vec4 c0 = v2.color;
    glm::vec4 c1 = v2.color;

    float dy = p2.y - p0.y;
    glm::vec4 incC0 = (v0.color - v2.color) / dy;
    glm::vec4 incC1 = (v1.color - v2.color) / dy;

    for (unsigned int scanLineY = p2.y; scanLineY > p0.y; scanLineY--) {
        DrawLine({(unsigned int) currX1, scanLineY}, 
                 {(unsigned int) currX2, scanLineY}, c0, c1);
        currX1 -= invSlope1;
        currX2 -= invSlope2;

        c0 += incC0;
        c1 += incC1;
    }
}

void DrawTriangleFill(Vertex v0, Vertex v1, Vertex v2) {
    if (v1.pos.y > v0.pos.y) 
        std::swap(v0.pos, v1.pos);

    if (v2.pos.y > v0.pos.y) 
        std::swap(v0.pos, v2.pos);

    bool invert = false;
    if (v2.pos.y > v1.pos.y) {
        std::swap(v2, v1);
        invert = true;
    }

    if (v1.pos.y == v2.pos.y) {
        DrawBottomFlatTriangle(v0, v1, v2);

    } else if (v0.pos.y == v1.pos.y) {
        DrawTopFlatTriangle(v0, v1, v2);
        
    } else {
        Vertex v3;

        v3.pos = glm::vec3((v0.pos.x + (v1.pos.y - v0.pos.y) / (v2.pos.y - v0.pos.y) * 
                    (v2.pos.x - v0.pos.x)), v1.pos.y, 0.0f);

        v3.color = (v2.color * (v0.pos.y - v3.pos.y) + v0.color * (v3.pos.y - v2.pos.y)) / 
            (v0.pos.y - v2.pos.y);

        if (!invert) {
            DrawBottomFlatTriangle(v0, v1, v3);
            DrawTopFlatTriangle(v1, v3, v2);
        } else {
            DrawBottomFlatTriangle(v0, v3, v1);
            DrawTopFlatTriangle(v3, v1, v2);
        }  
    }
}

#endif // _MYGL_H_

