#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <ModelTriangle.h>
#include <Colour.h>
#include <TextureMap.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>
#include <cmath>
#include <Utils.h>
#include <math.h> 
#include <stdlib.h>
#include <iostream>
#include <fstream>

#define WIDTH 320
#define HEIGHT 240

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, float numberOfValues) {
	std::vector<glm::vec3> fade;
	float step = (to[0]- from[0]) / (numberOfValues-1);
	float step1 = (to[1]- from[1]) / (numberOfValues-1);
	float step2 = (to[2]- from[2]) / (numberOfValues-1);
	int pos = 0;
	while(pos < numberOfValues) {
		glm::vec3 vec(from[0]+pos*step,from[1]+pos*step1,from[2]+pos*step2);
		fade.push_back(vec);
		pos++;
	}

	return fade; 
}

std::vector<std::vector<int>> getLineCoords(CanvasPoint from, CanvasPoint to){
	std::vector<std::vector<int>> line;

	if (from.x == to.x && from.y == to.y) { //case length 0
		int x = round(from.x);
		int y = round(from.y);
		std::vector<int> coord = {x,y};
		line.push_back(coord);
		return line;
	}
	float xDiff = to.x-from.x;
	float yDiff = to.y-from.y;
	float numberOfSteps = fmax(abs(xDiff),abs(yDiff));
	float xStepSize = xDiff / numberOfSteps;
	float yStepSize = yDiff / numberOfSteps;
	for (size_t i = 0; i < numberOfSteps; i++)
	{
		int x = round(from.x + xStepSize*i);
		int y = round(from.y + yStepSize*i);
		std::vector<int> coord = {x,y};
		line.push_back(coord);
	}
	return line;
}

void draw(DrawingWindow &window) {
	window.clearPixels();
	glm::vec3 topLeft(255, 0, 0);        // red 
	glm::vec3 topRight(0, 0, 255);       // blue 
	glm::vec3 bottomRight(0, 255, 0);    // green 
	glm::vec3 bottomLeft(255, 255, 0);   // yellow

	std::vector<glm::vec3> redYellow = interpolateThreeElementValues(topLeft,bottomLeft,window.height);
	std::vector<glm::vec3> blueGreen = interpolateThreeElementValues(topRight,bottomRight,window.height);

	for (size_t y = 0; y < window.height; y++) {
		std::vector<glm::vec3> row = interpolateThreeElementValues(redYellow.at(y),blueGreen.at(y),window.width);
		for (size_t x = 0; x < window.width; x++) {
			float red = row.at(x)[0];
			float green = row.at(x)[1];
			float blue = row.at(x)[2];
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x, y, colour);
		}
	}
}

void drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour colour) {
	float xDiff = to.x-from.x;
	float yDiff = to.y-from.y;
	float numberOfSteps = fmax(abs(xDiff),abs(yDiff));
	float xStepSize = xDiff / numberOfSteps;
	float yStepSize = yDiff / numberOfSteps;
	uint32_t colour_32 = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
	for (size_t i = 0; i < numberOfSteps; i++)
	{
		float x = from.x + xStepSize*i;
		float y = from.y + yStepSize*i;
		window.setPixelColour(round(x),round(y), colour_32);
	}

}

void drawLineTextured(DrawingWindow &window, CanvasPoint from, CanvasPoint to, std::vector<uint32_t> rowTexture) {
	float xDiff = to.x-from.x;
	float yDiff = to.y-from.y;
	if (xDiff == 0 && yDiff == 0) {
		window.setPixelColour(to.x,to.y, rowTexture[0]);
	} else {
		float numberOfSteps = fmax(abs(xDiff),abs(yDiff));
		float xStepSize = xDiff / numberOfSteps;
		float yStepSize = yDiff / numberOfSteps;
		float x,y;


		for (size_t i = 0; i < numberOfSteps; i++)
		{
			x = from.x + xStepSize*i;
			y = from.y + yStepSize*i;

			window.setPixelColour(round(x),round(y), rowTexture[i]);
		}
	}

}

void drawStrokedTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {

	//window.clearPixels();
	drawLine(window,triangle.v0(),triangle.v1(),colour);
	drawLine(window,triangle.v1(),triangle.v2(),colour);
	drawLine(window,triangle.v2(),triangle.v0(),colour);
}

std::vector<uint32_t> getRowTexture(TextureMap texture,TexturePoint from, TexturePoint to){

	std::vector<uint32_t> row;
	if (from.x == to.x && from.y == to.y) {
		 row.push_back(texture.pixels[texture.width*(from.y-1) + (from.x-1)]);
		 return row;
	}
	float xDiff = to.x-from.x;
	float yDiff = to.y-from.y;
	float numberOfSteps = fmax(abs(xDiff),abs(yDiff));
	float xStepSize = xDiff / numberOfSteps;
	float yStepSize = yDiff / numberOfSteps;
	
	for (float i = 0; i < numberOfSteps; i++)
	{
		float x = from.x + xStepSize*i;
		float y = from.y + yStepSize*i;

		row.push_back(texture.pixels[texture.width*(round(y)-1) + (round(x)-1)]);
	}
	return row;
}

uint32_t getPixelAvg(uint32_t pixel, uint32_t pixel1) {
	int newRed = (((pixel >> 16) & 255)+((pixel1 >> 24) & 255) / 2);
	int newGreen = (((pixel >> 8) & 255)+((pixel1 >> 16) & 255) / 2);
	int newBlue = ((pixel & 255)+(pixel1 & 255) / 2);
	uint32_t colour = (255 << 24) + (newRed << 16) + (newGreen << 8) + newBlue;
	return colour;
}

std::vector<uint32_t> getScaledRowTexture(CanvasTriangle triangle,CanvasPoint start,CanvasPoint end,TextureMap texture) {
	//get texture row vector
	//determine diffrence in length x and y
	//scale vector by interpolation
	std::vector<uint32_t> rowTexture = getRowTexture(texture,start.texturePoint,end.texturePoint);
	std::vector<std::vector<int>> imgLineCoords = getLineCoords(start,end);

	if (rowTexture.size() > imgLineCoords.size() ) { //scale down by removing pixels periodically
		int pixelsToLose = rowTexture.size() - imgLineCoords.size();
		int stepToErase = rowTexture.size() / pixelsToLose;
		for (size_t i = 0; i < pixelsToLose; i++) { //start erasing from 0	
			rowTexture.erase(rowTexture.begin()+ round(i*stepToErase)-i);
		} 
	} else if (rowTexture.size() < imgLineCoords.size()) {//scale up by interpolating between 2 neighbouring pixels periodically 
		int pixelsToGain = imgLineCoords.size()-rowTexture.size();
		int stepToInsert = imgLineCoords.size() / pixelsToGain;
		for (size_t i = 0; i < pixelsToGain; i++) {
			int insertIndex = round(stepToInsert*pixelsToGain)+i;
			uint32_t newPixel;
			if (insertIndex == 0 || insertIndex == rowTexture.size()-1) {
				newPixel = rowTexture[insertIndex];
			} else {
				newPixel = getPixelAvg(rowTexture[insertIndex]-1,rowTexture[insertIndex]+1);
			}
			rowTexture.insert(rowTexture.begin()+insertIndex,newPixel);
		}
		return rowTexture;
	}
	return rowTexture;
}

TexturePoint getTexturePoint(CanvasTriangle triangle,CanvasPoint point,int vertex){
	TexturePoint textureP;
	CanvasPoint bigV,smallV; //bigV refers to the vertex with larger y, ratio based off y 

	if (vertex == 0) { //point between v0, v1
		bigV = triangle.v1();
		smallV = triangle.v0();
	} else if (vertex == 1) {//point between v1, v2
		bigV = triangle.v2();
		smallV = triangle.v1();
	} else {//point between v0, v2
		bigV = triangle.v2();
		smallV = triangle.v0();
	}
	int yLength = bigV.y -smallV.y;

	float ratio = (point.y - smallV.y) / yLength;
	int xTexture = smallV.texturePoint.x+ round((bigV.texturePoint.x - smallV.texturePoint.x)*ratio);
	int yTexture = smallV.texturePoint.y+ round((bigV.texturePoint.y - smallV.texturePoint.y)*ratio);

	textureP = TexturePoint(xTexture,yTexture);
	return textureP;
}

void drawTexturedTriangle(DrawingWindow &window, CanvasTriangle triangle, TextureMap texture) {
	if (triangle.v0().y > triangle.v1().y) {
		std::swap(triangle.v0().y,triangle.v1().y); //sorting vertices in order of y coord smallest to largest
		std::swap(triangle.v0().x,triangle.v1().x); 
		std::swap(triangle.v0().texturePoint,triangle.v1().texturePoint); 
	}
	if (triangle.v0().y > triangle.v2().y) {
		std::swap(triangle.v0().y,triangle.v2().y);
		std::swap(triangle.v0().x,triangle.v2().x);
		std::swap(triangle.v0().texturePoint,triangle.v2().texturePoint);
		std::swap(triangle.v2().y,triangle.v1().y);
		std::swap(triangle.v2().x,triangle.v1().x);
		std::swap(triangle.v2().texturePoint,triangle.v1().texturePoint);
	} else if (triangle.v1().y > triangle.v2().y) {
		std::swap(triangle.v1().y,triangle.v2().y);
		std::swap(triangle.v1().x,triangle.v2().x);
		std::swap(triangle.v1().texturePoint,triangle.v2().texturePoint);
	}

	float x[3] = {triangle.v0().x,triangle.v1().x,triangle.v2().x};
	float y[3] = {triangle.v0().y,triangle.v1().y,triangle.v2().y};
	float xStepSize[3],yStepSize[3];
	CanvasPoint lineStart;
	CanvasPoint lineEnd;	

	for (int c=0;c<2;++c){
		xStepSize[c] = (x[c+1] - x[0])/ fmax(abs(x[c+1] - x[0]),abs(y[c+1] - y[0])); //determine step sizes for v0-v1 and v0,v2 
		yStepSize[c] = (y[c+1] - y[0]) / fmax(abs(y[c+1] - y[0]),abs(x[c+1] - x[0]));
	}
	xStepSize[2] = (x[2]-x[1]) / fmax(abs(x[2] - x[1]),abs(y[2] - y[1])); //determine step size from v1-v2
	yStepSize[2] = (y[2]-y[1]) / fmax(abs(x[2] - x[1]),abs(y[2] - y[1]));
	float curX[2] = {x[0],x[0]}; //init with v0 as starting point
	float curY[2] = {y[0],y[0]};
	int fillStage =0;

	for (size_t row = y[0]; row < y[2]; row++){ //loop through y values from v0 to v2

		if (row == y[1]){ //if v0-v1 is filled with lines switch step size to v1-v2

			fillStage++;
			xStepSize[0] = xStepSize[2];
			yStepSize[0] = yStepSize[2];
			curX[0] = x[1];
			curY[0] = y[1];		
		}
		for (size_t i = 0; i < 2; i++){ //increment x values on both lines till one step before y increment
			if (yStepSize[i]>0) {
				while(round(curY[i]+ yStepSize[i]) == row ){			
					curX[i]+=xStepSize[i];
					curY[i]+=yStepSize[i];
				}
			}
		}

		lineStart = CanvasPoint(round(curX[0]),row); //create canvas point for drawing line then add the approparie texture point from ratio along line
		lineStart.texturePoint = TexturePoint(getTexturePoint(triangle,lineStart,fillStage)); 

		lineEnd = CanvasPoint(round(curX[1]),row);
		lineEnd.texturePoint = TexturePoint(getTexturePoint(triangle,lineEnd,2));	
	
		std::vector<uint32_t> rowTexture = getScaledRowTexture(triangle,lineStart,lineEnd,texture); //get the exact number of pixels required to draw in image by scaling fetched texture
		drawLineTextured(window, lineStart,lineEnd,rowTexture); //draw horizontal line before y incrmement

		curX[0] += xStepSize[0];
		curY[0] += yStepSize[0];
		curX[1]  += xStepSize[1];
		curY[1] += yStepSize[1];



	}


	//drawLine(window, CanvasPoint(x[2],y[2]),CanvasPoint(x[2],y[2]),colour);
	drawStrokedTriangle(window,triangle,Colour(255,255,255));
	//window.setPixelColour(round(x),round(y), colour_32);
}

void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {
	if (triangle.v0().y > triangle.v1().y) std::swap(triangle.v0().y,triangle.v1().y);
	if (triangle.v0().y > triangle.v2().y) {
		std::swap(triangle.v0().y,triangle.v2().y);
		std::swap(triangle.v2().y,triangle.v1().y);
	} else if (triangle.v1().y > triangle.v2().y) std::swap(triangle.v1().y,triangle.v2().y);

	float x[3] = {triangle.v0().x,triangle.v1().x,triangle.v2().x};
	float y[3] = {triangle.v0().y,triangle.v1().y,triangle.v2().y};
	float xStepSize[3],yStepSize[3];
	CanvasPoint lineStart;
	CanvasPoint lineEnd;	

	for (int c=0;c<2;++c){
		xStepSize[c] = (x[c+1] - x[0])/ fmax(abs(x[c+1] - x[0]),abs(y[c+1] - y[0])); //step[0] top
		yStepSize[c] = (y[c+1] - y[0]) / fmax(abs(y[c+1] - y[0]),abs(x[c+1] - x[0]));//step[1] full length line
	}
	xStepSize[2] = (x[2]-x[1]) / fmax(abs(x[2] - x[1]),abs(y[2] - y[1])); //step[3] second stage line
	yStepSize[2] = (y[2]-y[1]) / fmax(abs(x[2] - x[1]),abs(y[2] - y[1]));

	float curX[2] = {x[0],x[0]};
	float curY[2] = {y[0],y[0]};
	for (size_t row = y[0]; row < y[2]; row++){
		if (row == y[1]){
			xStepSize[0] = xStepSize[2];
			yStepSize[0] = yStepSize[2];
			curX[0] = x[1];
			curY[0] = y[1];		
		}
		for (size_t i = 0; i < 2; i++){
			if (yStepSize[i]>0) {
				while(round(curY[i]+ yStepSize[i]) == row ){			
					curX[i]+=xStepSize[i];
					curY[i]+=yStepSize[i];
				}
			}
		}
		lineStart = CanvasPoint(round(curX[0]),row); 
		lineEnd = CanvasPoint(round(curX[1]),row);	
		drawLine(window, lineStart,lineEnd,colour); //draw horizontal line before y change
		curX[0] += xStepSize[0];
		curY[0] += yStepSize[0];
		curX[1]  += xStepSize[1];
		curY[1] += yStepSize[1];
	}
	drawStrokedTriangle(window,triangle,Colour(255,255,255));
	//window.setPixelColour(round(x),round(y), colour_32);
}

std::vector<ModelTriangle> loadObj(std::string filename) {
	std::string line;
	std::vector<ModelTriangle> triangles;
	std::vector<glm::vec3> vertices;
	Colour colour = Colour(255,0,0);

	std::ifstream MyReadFile(filename);
	while (getline (MyReadFile, line)) {
		std::vector<std::string> sections = split(line, ' ');
		if (sections.size()>0) {
			if (sections[0] == "v") {
				glm::vec3 newV = glm::vec3(std::stof(sections[1]),std::stof(sections[2]),std::stof(sections[3]) );
				vertices.push_back(newV);
			} else if (sections[0] == "f") {
				int v0Pos = std::stoi(split(sections[1], '/')[0]) -1 ;
				int v1Pos = std::stoi(split(sections[2], '/')[0]) -1 ;
				int v2Pos = std::stoi(split(sections[3], '/')[0]) -1 ;
				triangles.push_back(ModelTriangle(vertices[v0Pos],vertices[v1Pos],vertices[v2Pos],colour));
			} else if (sections[0] == "usemtl") {
				
			} 
		}
		


	}

	// Close the file
	MyReadFile.close();
	return triangles;
}


void handleEvent(SDL_Event event, DrawingWindow &window) {
	srand (time(NULL));
	CanvasPoint v0 = CanvasPoint(rand()%(window.width),rand()%(window.height));
	CanvasPoint v1 = CanvasPoint(rand()%(window.width),rand()%(window.height));
	CanvasPoint v2 = CanvasPoint(rand()%(window.width),rand()%(window.height));
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if (event.key.keysym.sym == SDLK_u) {
			std::cout << "u" << std::endl;
			srand (time(NULL));
			Colour triangleColour = Colour(rand()%255,rand()%255,rand()%255);
			CanvasTriangle triangle = CanvasTriangle(v0,v1,v2);
			drawStrokedTriangle(window,triangle,triangleColour);

		} else if (event.key.keysym.sym == SDLK_f) {
			std::cout << "f" << std::endl;
			Colour triangleColour = Colour(rand()%255,rand()%255,rand()%255);
			CanvasTriangle triangle = CanvasTriangle(v0,v1,v2);
			drawFilledTriangle(window,triangle,triangleColour);

		} else if (event.key.keysym.sym == SDLK_t) {

			CanvasPoint v0 = CanvasPoint(160,10);
			v0.texturePoint = TexturePoint(195,5);
			CanvasPoint v1 = CanvasPoint(300,230);
			v1.texturePoint = TexturePoint(395,380);
			CanvasPoint v2 = CanvasPoint(10,150);
			v2.texturePoint = TexturePoint(65,330);
			std::cout << "t" << std::endl;
			CanvasTriangle triangle = CanvasTriangle(v0,v1,v2);
			drawTexturedTriangle(window,triangle,TextureMap("texture.ppm"));
		} else if (event.key.keysym.sym == SDLK_l) {

			std::vector<ModelTriangle> triangles = loadObj("cornell-box.obj");
			for (ModelTriangle tri : triangles) {
				std::cout << tri.vertices[0].x << " , "<< tri.vertices[0].y << " , "<< tri.vertices[0].z  << "\n";
				std::cout << tri.vertices[1].x << " , "<< tri.vertices[1].y << " , "<< tri.vertices[1].z  << "\n";
				std::cout << tri.vertices[2].x << " , "<< tri.vertices[2].y << " , "<< tri.vertices[2].z  << "\n";		
				std::cout << "\n";		
			}
		} else if (event.type == SDL_MOUSEBUTTONDOWN) {
			window.savePPM("output.ppm");
			window.saveBMP("output.bmp"); 
		}
	}
}


int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;


	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
