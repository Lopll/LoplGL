#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"



const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green   = TGAColor(0, 255,   0,   255);
const TGAColor blue   = TGAColor(0, 0,   255,   255);

Model *model = NULL;
int* zBuffer = NULL;
Vec3f lightDir(0,0,-1);

const int width  = 800;
const int height = 800;
const int depth = 255;

int unZoomLevel = 2;
int unZoomX = width/unZoomLevel;
int unZoomY = height/unZoomLevel;
int unZoomZ = depth/unZoomLevel;
int deltaPos = unZoomLevel/2;

// line for 3D!
void rasterize(Vec3i p0, Vec3i p1, TGAImage &image, TGAColor color, int zBuffer[])
{
    if (p0.x > p1.x) // left-to-right rule
    {
        std::swap(p0, p1);
    }
    for (int x=p0.x; x<=p1.x; x++) 
    {
        float t = (x-p0.x)/(float)(p1.x-p0.x);
        int y = p0.y*(1.-t) + p1.y*t;
        int z = p0.z*(1.-t) + p1.z*t;
        int idx = x + y*width;
        if (zBuffer[idx]<z) 
        {
            zBuffer[idx] = z;
            image.set(x, y, color);
        }
    }
}


void filledTriangle(Vec3i p0, Vec3i p1, Vec3i p2, TGAImage &image, Vec2i uv0, Vec2i uv1, Vec2i uv2, float intensity, int* zBuffer)
{
    if (p0.y==p1.y && p0.y == p2.y) return; // degenerate triangle
    
    // bubble sort triangles by y coordinate
    if (p0.y>p1.y) 
    {
        std::swap(p0, p1);
        std::swap(uv0, uv1);
    }
    if (p0.y>p2.y) 
    {
        std::swap(p0, p2);
        std::swap(uv0, uv2);
    }
    if (p1.y>p2.y) 
    {
        std::swap(p1, p2);
        std::swap(uv1, uv2);
    }
    
    int totalH = p2.y-p0.y;
    for (int i=0; i<totalH; i++) 
    {
        bool secondHalf = i>p1.y-p0.y || p1.y==p0.y;
        int segmentH = secondHalf ? p2.y-p1.y : p1.y-p0.y;
        float alpha = (float) i / totalH;
        float beta  = (float)(i-(secondHalf ? p1.y-p0.y : 0))/segmentH; // be careful: with above conditions no division by zero here
        
        //interpolation inside the triangle
        Vec3i A =              p0 + Vec3f(p2-p0  )*alpha;
        Vec3i B = secondHalf ? p1 + Vec3f(p2-p1  )*beta : p0 + Vec3f(p1-p0)*beta;
        Vec2i uvA =            uv0+      (uv2-uv0)*alpha;
        Vec2i uvB = secondHalf?uv1+      (uv2-uv1)*beta : uv0 +     (uv1-uv0)*beta;
        
        if (A.x>B.x) 
        {
            std::swap(A, B); 
            std::swap(uvA, uvB);
        }
        
        for (int j=A.x; j<=B.x; j++) 
        {
            float phi = B.x==A.x ? 1. : (float)(j-A.x)/(float)(B.x-A.x);
            
            Vec3i P = Vec3f(A) + Vec3f(B-A)*phi;
            Vec2i uvP =   uvA  +  (uvB-uvA)*phi;
            
            int idx = P.x+P.y*width;
            if (zBuffer[idx]<P.z) 
            {
                zBuffer[idx] = P.z;
                TGAColor color = model->diffuse(uvP);
                image.set(P.x, P.y, TGAColor(color.r * intensity, color.g * intensity, color.b * intensity, 255));
            }
        }
    }
}

// rendering model.obj
void renderModel(Model *model, TGAImage &image, Vec3f &lightDir, int* zBuffer)
{
    for(int i = 0; i < model->nfaces(); i++)
	{ 
		std::vector<int> face = model->face(i);
		Vec3i screenCoords[3];
		Vec3f worldCoords[3];
		Vec2i uv[3];
        for(int j = 0; j < 3; j++)
        {
            Vec3f v = model->vert(face[j]);
            screenCoords[j] = Vec3i((v.x+deltaPos)*unZoomX, (v.y+deltaPos)*unZoomY, (v.z+deltaPos)*unZoomZ);
            worldCoords[j] = v;
            uv[j] = model->uv(i,j);
        }
        Vec3f n = (worldCoords[2]-worldCoords[0])^(worldCoords[1]-worldCoords[0]);
        n.normalize();
        float intensity = n*lightDir;
        if (intensity>0) 
        {
            filledTriangle(screenCoords[0], screenCoords[1], screenCoords[2], image, uv[0], uv[1], uv[2], intensity, zBuffer);
        }
	}
}

int main() {
    model = new Model("obj/african_head.obj");

	// img set up
	TGAImage render(width, height, TGAImage::RGB);
	
    // TGAImage texture;
    // texture.read_tga_file("obj/african_head_diffuse.tga");

	zBuffer = new int[width*height];
	for(int i = 0; i < width*height; i++)
	{
	   zBuffer[i] = std::numeric_limits<int>::min();
	}
	
	
	
	
    renderModel(model, render, lightDir, zBuffer);
	
	{ // dump z-buffer (debugging purposes only)
        TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
        for (int i=0; i<width; i++) {
            for (int j=0; j<height; j++) {
                zbimage.set(i, j, TGAColor(zBuffer[i+j*width], 1));
            }
        }
        zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        zbimage.write_tga_file("zbuffer.tga");
    }
	
	// end render
	render.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	render.write_tga_file("render.tga");
	
	delete model;
	delete [] zBuffer;
	return 0;
}

