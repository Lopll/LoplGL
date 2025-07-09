#include <vector>
#include <cmath>
#include <iostream>
#include <cstdlib> // for rand
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green   = TGAColor(0, 255,   0,   255);
const TGAColor blue   = TGAColor(0, 0,   255,   255);

Model *model = NULL;

const int width  = 2000;
const int height = 2000;
int unZoomLevel = 2;
int unZoomX = width/unZoomLevel;
int unZoomY = height/unZoomLevel;
int deltaPos = unZoomLevel/2;

void line(Vec2i p0, Vec2i p1, TGAImage &image, const TGAColor &color)
{
	bool steep = false;
    if (std::abs(p0.x-p1.x)<std::abs(p0.y-p1.y)) // if the line is steep, we transpose the image
    { 
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }
    if (p0.x>p1.x) // make it left-to-right
    { 
        std::swap(p0.x, p1.x);
        std::swap(p0.y, p1.y);
    }
    
    int dx = p1.x-p0.x;
    int dy = p1.y-p0.y;
    float derror2 = std::abs(dy)*2;
    float error2 = 0;
    int y = p0.y;

    for (int x=p0.x; x<=p1.x; x++) 
    {
        if (steep) 
        {
            image.set(y, x, color); // if transposed, de-transpose
        } 
        else 
        {
            image.set(x, y, color);
        }
        error2 += derror2;
        
        if (error2 > dx)
        {
        	y += (p1.y>p0.y?1:-1);
        	error2 -= dx*2;
        }
    }
}

void triangle(const Vec2i p0, const Vec2i p1, const Vec2i p2, TGAImage &image, const TGAColor &color)
{
    line(p0, p1, image, color);
    line(p1, p2, image, color);
    line(p0, p2, image, color);
}

void filledTriangle(Vec2i p0, Vec2i p1, Vec2i p2, TGAImage &image, const TGAColor &color)
{
    if (p0.y>p1.y) std::swap(p0, p1);
    if (p0.y>p2.y) std::swap(p0, p2);
    if (p1.y>p2.y) std::swap(p1, p2);
    
    line(p0, p1, image, color);
    line(p1, p2, image, color);
    line(p0, p2, image, color);
    
    int dx01 = (p1.x - p0.x);
    int dy01 = std::abs(p1.y - p0.y);
    
    int dx02 = (p2.x - p0.x);
    int dy02 = std::abs(p2.y - p0.y);
    
    int dx21 = (p2.x - p1.x);
    int dy21 = std::abs(p2.y - p1.y);
    
    float slope01 = (float)dx01/(float)dy01;
    float slope02 = (float)dx02/(float)dy02;
    float slope21 = (float)dx21/(float)dy21; 
    
    for(int y = p0.y; y < p2.y; y++)
    {
        int x01;
        int x02 = p0.x + slope02 * (y - p0.y);;
        if (y < p1.y)
        {
            x01 = p0.x + slope01 * (y - p0.y);
        }
        else
        {
            x01 = p2.x + slope21 * (y - p2.y); // p1.x - slope12 * (y - p1.y);
        }
        // std::cout << "y: " << y << ", x02: " << x02 << ", x01: " << x01 << std::endl;
        line(Vec2i(x01,y), Vec2i(x02,y), image, color);
        
    }
}

// rendering model.obj
void renderModel(Model *model, TGAImage &image, Vec3f &lightDir)
{
    for(int i = 0; i < model->nfaces(); i++)
	{ 
		std::vector<int> face = model->face(i);
		Vec2i screenCoords[3];
		Vec3f worldCoords[3];
        for(int j = 0; j < 3; j++)
        {
            Vec3f v = model->vert(face[j]);
            screenCoords[j] = Vec2i((v.x+deltaPos)*unZoomX, (v.y+deltaPos)*unZoomY);
            worldCoords[j] = v;
        }
        Vec3f n = (worldCoords[2]-worldCoords[0])^(worldCoords[1]-worldCoords[0]);
        n.normalize();
        float intensity = n*lightDir;
        if (intensity>0) 
        {
            filledTriangle(screenCoords[0], screenCoords[1], screenCoords[2], image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
        }
	}
}

int main() {
    model = new Model("obj/african_head.obj");

	// img set up
	TGAImage image(width, height, TGAImage::RGB);

	// graphic
	Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)};
    Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)};
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
	
	Vec3f lightDir(0,0,-1);
	
    renderModel(model, image, lightDir);
	
	// end render
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	delete model;
	return 0;
}

