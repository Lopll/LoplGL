#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
Model *model = NULL;
const int width  = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage &image, const TGAColor &color)
{
	bool steep = false;
    if (std::abs(x0-x1)<std::abs(y0-y1)) // if the line is steep, we transpose the image
    { 
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) // make it left-to-right
    { 
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    
    int dx = x1-x0;
    int dy = y1-y0;
    float derror2 = std::abs(dy)*2;
    float error2 = 0;
    int y = y0;

    for (int x=x0; x<=x1; x++) 
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
        	y += (y1>y0?1:-1);
        	error2 -= dx*2;
        }
    }
}

int main() {
    model = new Model("obj/african_head.obj");

	// img set up
	TGAImage image(width, height, TGAImage::RGB);

	// graphic
	// line(13, 20, 80, 40, image, white);
	// line(20, 13, 40, 80, image, red);
	// line(80, 40, 13, 20, image, red);
	
	for(int i = 0; i < model->)
	{
		
	}
	
	// render
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	delete model;
	return 0;
}

