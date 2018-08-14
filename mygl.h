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

//Function to converto from NDC to screen coordinates
inline PixelCoord convertCoord(glm::vec3 p) {
    return {static_cast<unsigned int>(p.x*((IMAGE_WIDTH-1)/2.0f)+((IMAGE_WIDTH-1)/2.0f)), 
            static_cast<unsigned int>(p.y*-1.0f*((IMAGE_HEIGHT-1)/2.0f)+((IMAGE_HEIGHT-1)/2.0f))};
}

//Function to convert from float RGBA to unsigned char RGBA
inline PixelColor convertColor(glm::vec4 c) {
    return {static_cast<unsigned char>(c.r * 255),
            static_cast<unsigned char>(c.g * 255),
            static_cast<unsigned char>(c.b * 255),
            static_cast<unsigned char>(c.a * 255)};
}

//Write pixel to buffer
void PutPixel(PixelCoord p, PixelColor c) {
    if(p.x >= IMAGE_WIDTH || p.y >= IMAGE_HEIGHT) 
        throw std::length_error("Frame buffer acess out of bounds");

    unsigned int index = (p.x + p.y * IMAGE_WIDTH) * 4;
    FBptr[index    ] = c.r;
    FBptr[index + 1] = c.g;
    FBptr[index + 2] = c.b;
    FBptr[index + 3] = c.b;
}

//Fill buffer with a solid color
void ClearBuffer(glm::vec4 c) {
    for(unsigned int i = 0; i < IMAGE_WIDTH; i++)
        for(unsigned int j = 0; j < IMAGE_HEIGHT; j++) {
            PutPixel({i, j}, convertColor(c));
        }
}

void DrawLine(glm::vec3 v0, glm::vec3 v1, glm::vec4 c0, glm::vec4 c1) {

    //Convert from NDC to screen coordinates
    //NDC has (0.0, 0.0) as the center point
    //(-1.0, -1.0) lower left conner
    //(1.0, 1.0) higher right conner
    PixelCoord p0 = convertCoord(v0);
    PixelCoord p1 = convertCoord(v1);

    int dx = p1.x - p0.x;
    int dy = p1.y - p0.y;

    //If the slope is higher than 45 degrees (dy > dx),
    //swap x with y
    bool invert = false;
    if(abs(dy) > abs(dx)) {
        std::swap(dx, dy);
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        invert = true;
    }

    //Linear color interpolation
    bool interpolate = false;
    glm::vec4 incC;
    if(c0 != c1) {
        //If the x coordinate is mirroed (it is on the oposite quadrant)
        //also mirror the interpolation
        if (dx > 0)
            incC = (c1 - c0)/(float)dx;
        else
            incC = (c0 - c1)/(float)dx;

        interpolate = true;
    }

    int incX = 1, incY = 1;
    //Mirror the y coordinate and decrement instead of increment
    if(dy < 0) { 
        dy = -dy; 
        incY = -1; 
    } 

    //Mirror the x coordinate and decrement instead of increment
    if(dx < 0) { 
        dx = -dx; 
        incX = -1; 
    } 

    //Bresenham algorithm
    int incE  = 2 * dy;
    int incNE = 2 * (dy - dx);
    int d = 2*dy - dx;

    //If the x coordinate has been mirroed, adjust the loop accordingly by
    //inverting the comparison
    int currX = p0.x, currY = p0.y;
    while(incX*currX <= (int)(incX*p1.x)) {
        //If x was been swaped with y, swap then back
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

        //Dont interpolate if it isnt necessary
        if(interpolate) 
            c0 += incC;
    }
}

struct Vertex {
    //glm::vec2 texcoord;
    glm::vec3 pos;
    glm::vec4 color;
};

void DrawTriangle(Vertex v0, Vertex v1, Vertex v2) {
    DrawLine(v0.pos, v1.pos, v0.color, v1.color);
    DrawLine(v1.pos, v2.pos, v1.color, v2.color);
    DrawLine(v2.pos, v0.pos, v2.color, v0.color);
}

//Simpler function for drawing horizontal lines.
//Used for fill triangles
void DrawHorizontalLine(PixelCoord p0, PixelCoord p1, glm::vec4 c0, glm::vec4 c1) {
    glm::vec4 incC = (c1 - c0) / ((float)p1.x - p0.x);

    for (unsigned int i = p0.x; i < p1.x; i++) {
        PutPixel({i, p0.y}, convertColor(c0));
        c0 += incC;
    }
}

void DrawBottomFlatTriangle(Vertex v0, Vertex v1, Vertex v2) {
    if (v1.pos.x > v2.pos.x)
        std::swap(v1.pos, v2.pos);

    //Convert from NDC to screen coordinates
    PixelCoord p0 = convertCoord(v0.pos);
    PixelCoord p1 = convertCoord(v1.pos);
    PixelCoord p2 = convertCoord(v2.pos);

    float invSlope1 = ((float)p1.x - p0.x) / ((float)p1.y - p0.y);
    float invSlope2 = ((float)p2.x - p0.x) / ((float)p2.y - p0.y);

    float currX1 = p0.x;
    float currX2 = p0.x;

    glm::vec4 c0 = v0.color;
    glm::vec4 c1 = v0.color;

    //Linear color interpolation from v0 (topmost vertex)
    //to v1 (leftmost vertex) and to v2(rightmost vertex)
    float dy = p1.y - p0.y;
    glm::vec4 incC0 = (v1.color - v0.color) / dy;
    glm::vec4 incC1 = (v2.color - v0.color) / dy;

    //Draw a scanline between two endpoints (currX1 and currX2)
    //from top to bottom
    for (unsigned int scanLineY = p0.y; scanLineY <= p1.y; scanLineY++) {
        DrawHorizontalLine({(unsigned int) currX1, scanLineY}, 
                           {(unsigned int) currX2, scanLineY}, c0, c1);

        //Increment the endpoint by their respective lines slopes
        //making the scanline bigger, thus creating a bottom flat triangle
        currX1 += invSlope1;
        currX2 += invSlope2;

        c0 += incC0;
        c1 += incC1;
    }
}

void DrawTopFlatTriangle(Vertex v0, Vertex v1, Vertex v2) {
    if (v0.pos.x > v1.pos.x)
        std::swap(v0.pos, v1.pos);

    //Convert from NDC to screen coordinates
    PixelCoord p0 = convertCoord(v0.pos);
    PixelCoord p1 = convertCoord(v1.pos);
    PixelCoord p2 = convertCoord(v2.pos);

    float invSlope1 = ((float)p2.x - p0.x) / ((float)p2.y - p0.y);
    float invSlope2 = ((float)p2.x - p1.x) / ((float)p2.y - p1.y);

    float currX1 = p2.x;
    float currX2 = p2.x;

    glm::vec4 c0 = v2.color;
    glm::vec4 c1 = v2.color;

    //Linear color interpolation from v2 (lowermost vertex)
    //to v0 (leftmost vertex) and to v1(rightmost vertex)
    float dy = p2.y - p0.y;
    glm::vec4 incC0 = (v0.color - v2.color) / dy;
    glm::vec4 incC1 = (v1.color - v2.color) / dy;

    //Draw a scanline between two endpoints (currX1 and currX2)
    //from bottom to top
    for (unsigned int scanLineY = p2.y; scanLineY > p0.y; scanLineY--) {
        DrawHorizontalLine({(unsigned int) currX1, scanLineY}, 
                           {(unsigned int) currX2, scanLineY}, c0, c1);

        //Decrement the endpoint by their respective lines slopes
        //making the scanline bigger, thus creating a top flat triangle
        currX1 -= invSlope1;
        currX2 -= invSlope2;

        c0 += incC0;
        c1 += incC1;
    }
}

void DrawTriangleFill(Vertex v0, Vertex v1, Vertex v2) {
    //Sorting the vertices
    //v0 the topmost
    //v2 the lowermost
    //v1 in between
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
        //If it is a bottom flat triangle
        DrawBottomFlatTriangle(v0, v1, v2);

    } else if (v0.pos.y == v1.pos.y) {
        //If it is a top flat triangle
        DrawTopFlatTriangle(v0, v1, v2);
        
    } else {
        //If is neither, split the triangle in two, a top flat, and a bottom flat
        Vertex v3;

        //Calculate the intersection point
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

