#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <ModelTriangle.h>
#include <Colour.h>
#include <TextureMap.h>
#include <RayTriangleIntersection.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>
#include <cmath>
#include <Utils.h>
#include <math.h> 
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <unordered_map>

#define WIDTH 960
#define HEIGHT 720
//#define WIDTH 640
//#define HEIGHT 480
//#define WIDTH 320
//#define HEIGHT 240


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

float getPointDepth(CanvasPoint v0, CanvasPoint v1, int curX,int curY) {

	float ratio = ((curY-v0.y)+ abs(curX-v0.x)) /((v1.y- v0.y) + abs(v1.x - v0.x)); //always pos
	//std::cout << "ratio " << ratio << "\n";
	
	float depthRange = v1.depth - v0.depth;
	//std::cout << "depth range " << depthRange << "\n";
	//std::cout << "v0 " << v0.x << "," << v0.y  << " depth " << v0.depth << "  v1 " << v1.x << "," << v1.y<< " depth " << v1.depth << "  Cur " << curX << "," << curY << " depth " << v0.depth + depthRange*ratio << "\n";  
	return (v0.depth + (depthRange*ratio));
	//return (v0.depth + v1.depth) /2;
}

void drawLineDepth(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour colour,std::vector<std::vector<float>> &depthBuffer) {
	uint32_t colour_32 = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
	//std::cout << "we made it kind of\n";

	if (from.x == to.x && from.y == to.y && from.x>=0 && from.x < window.width && from.y >=0 && from.y < window.height) {
		if (from.depth > depthBuffer[from.x][from.y]){
			depthBuffer[from.x][from.y] = (1/ (1+exp(-from.depth)));
			window.setPixelColour(from.x,from.y, colour_32);
		}
	} else {
		float xDiff = to.x-from.x;
		float yDiff = to.y-from.y;
		float numberOfSteps = fmax(abs(xDiff),abs(yDiff));
		float xStepSize = xDiff / numberOfSteps;
		float yStepSize = yDiff / numberOfSteps;

		for (size_t i = 0; i <= numberOfSteps; i++) {

			int x = round(from.x + xStepSize*i);
			int y = round(from.y + yStepSize*i);
			if (x>=0 && x < window.width && y>=0 && y < window.height ) {
				//std::cout << "----------\n";
				//std::cout << "x " << x << "  y " << y << "\n";
				float depth = getPointDepth(from,to,x,y) ; //sigmoid function outputs 0-1 range
				//std::cout << "   |   parents depth " << from.depth << " & " << to.depth << " child depth " << depth << "   |";
				//if (1/ (1+exp(depth)) > depthBuffer[x][y]){
				if (depth > depthBuffer[x][y]){

					//std::cout << "coord in range\n";
					//std::cout << "depth buffer before " << depthBuffer[x][y] << "\n";
					depthBuffer[x][y] = (depth);
					//std::cout << "depth buffer after " << depthBuffer[x][y] << "\n";
					window.setPixelColour(x,y, colour_32);
				} else {
					//std::cout << "current depth " << depth << "\n"; 
					//std::cout << "depth " << depthBuffer[x][y] << "\n"; 
					//window.setPixelColour(x,y, white);
				}
			}
		}
		//std::cout << "\n";
	}
}

void drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour colour) {
	uint32_t colour_32 = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
	if (from.x == to.x && from.y == to.y) {
		window.setPixelColour(from.x,from.y, colour_32);
	} else {
		float xDiff = to.x-from.x;
		float yDiff = to.y-from.y;
		float numberOfSteps = fmax(abs(xDiff),abs(yDiff));
		float xStepSize = xDiff / numberOfSteps;
		float yStepSize = yDiff / numberOfSteps;
		for (size_t i = 0; i <= numberOfSteps; i++)
		{
			int x = round(from.x + xStepSize*i);
			int y = round(from.y + yStepSize*i);
			window.setPixelColour(x,y, colour_32);
		}
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


		for (size_t i = 0; i <= numberOfSteps; i++)
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

TexturePoint getTexturePoint(CanvasTriangle triangle,CanvasPoint point,int vertex) {
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
					if (round(curX[i]) == x[2] && round(curY[i]) == y[2]) break;
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

void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour,std::vector<std::vector<float>> &depthBuffer) {
	if (triangle.v0().y > triangle.v1().y) {
		std::swap(triangle.v0().y,triangle.v1().y); //sorting vertices in order of y coord smallest to largest
		std::swap(triangle.v0().x,triangle.v1().x); 
		std::swap(triangle.v0().depth,triangle.v1().depth); 
	}
	if (triangle.v0().y > triangle.v2().y) {
		std::swap(triangle.v0().y,triangle.v2().y);
		std::swap(triangle.v0().x,triangle.v2().x);
		std::swap(triangle.v0().depth,triangle.v2().depth);
		std::swap(triangle.v2().y,triangle.v1().y);
		std::swap(triangle.v2().x,triangle.v1().x);
		std::swap(triangle.v2().depth,triangle.v1().depth);
	} else if (triangle.v1().y > triangle.v2().y) {
		std::swap(triangle.v1().y,triangle.v2().y);
		std::swap(triangle.v1().x,triangle.v2().x);
		std::swap(triangle.v1().depth,triangle.v2().depth);
	}
	//std::cout << "we made it draw filled tri\n";
	

	float x[3] = {triangle.v0().x,triangle.v1().x,triangle.v2().x};
	float y[3] = {triangle.v0().y,triangle.v1().y,triangle.v2().y};
	float xStepSize[3],yStepSize[3];
	CanvasPoint lineStart;
	CanvasPoint lineEnd;	
	//std::cout << "target: " << x[2] << "," << y[2] <<"\n"; 
	for (int c=0;c<2;++c){
		xStepSize[c] = (x[c+1] - x[0])/ fmax(abs(x[c+1] - x[0]),abs(y[c+1] - y[0])); //step[0] top
		yStepSize[c] = (y[c+1] - y[0]) / fmax(abs(y[c+1] - y[0]),abs(x[c+1] - x[0]));//step[1] full length line
	}
	xStepSize[2] = (x[2]-x[1]) / fmax(abs(x[2] - x[1]),abs(y[2] - y[1])); //step[3] second stage line
	yStepSize[2] = (y[2]-y[1]) / fmax(abs(x[2] - x[1]),abs(y[2] - y[1]));
	float curX[2] = {x[0],x[0]};
	float curY[2] = {y[0],y[0]};
	//std::cout << "y[0] = " << y[0] << "  y[1] = " << y[1] << "  y[2] = " << y[2] << "\n"; 

	for (size_t row = y[0]; row <= y[2]; row++){
		if (row == y[1]){ //if v0.y == v1.y == v2.y then only v1 to v2 is drawn
			xStepSize[0] = xStepSize[2];
			yStepSize[0] = yStepSize[2];
			curX[0] = x[1];
			curY[0] = y[1];		
		}
		//std::cout << "row " << row << " of " << y[2] << "\n";
		for (size_t i = 0; i < 2; i++){

			while(round(curY[i]) == row){	

				if (row <= y[1]) {
					//std::cout << "made it to lineStart\n";
					lineStart = CanvasPoint(round(curX[0]),row, getPointDepth(triangle.v0(),triangle.v1(), round(curX[0]),row)); 
					//std::cout << "lineStart depths v0 " << triangle.v0().depth << "  v1 " << triangle.v1().depth << "  new depth  " << lineStart.depth << "\n";
				} else {
					lineStart = CanvasPoint(round(curX[0]),row, getPointDepth(triangle.v1(),triangle.v2(), round(curX[0]),row)); 
				}

				//std::cout << "made it to lineEnd\n";

				lineEnd = CanvasPoint(round(curX[1]),row, getPointDepth(triangle.v0(),triangle.v2(), round(curX[1]),row));
				//std::cout << "draw line: " << lineStart.x << "," << lineStart.y << " to " << lineEnd.x << "," << lineEnd.y << "\n"; 
				//std::cout << "we made it get depth point\n";

				if (lineStart.depth < 0 && lineEnd.depth < 0) {
					//std::cout << "continue\n";
					continue;
				}
				//std::cout << "made it to draw line depth\n";
				drawLineDepth(window, lineStart,lineEnd,colour,depthBuffer);
				//drawLine(window,lineStart,lineEnd,colour);
				if (yStepSize[i] == 0) break;
				if (round(curX[i]) == x[2] && round(curY[i]) == y[2]) break;
				curX[i]+=xStepSize[i];
				curY[i]+=yStepSize[i];
			}
		}
	}
	//drawStrokedTriangle(window,triangle,Colour(255,255,255));
	//window.setPixelColour(round(x),round(y), colour_32);
}

std::vector<ModelTriangle> loadObj(std::string objFilename, std::string mtlFilename, float scale) {
	std::string line;
	std::vector<ModelTriangle> triangles;
	std::vector<glm::vec3> vertices;
	std::unordered_map<std::string, Colour> objColours;
	std::string colourName;
	std::vector<std::string> sections;

	std::ifstream MyReadFileMtl(mtlFilename);
	while (getline (MyReadFileMtl, line)) {
		sections = split(line, ' ');
		if (sections.size()>0) {
			if (sections[0] == "Kd") {
				objColours.insert({colourName, Colour(colourName,std::stof(sections[1]),std::stof(sections[2]),std::stof(sections[3]))});
			} else if (sections[0] == "newmtl") {
				colourName = sections[1];
			} 
		}
		sections.clear();
	}
	MyReadFileMtl.close();
	Colour curCol;
	std::ifstream MyReadFile(objFilename);
	while (getline (MyReadFile, line)) {
		sections = split(line, ' ');
		if (sections.size()>0) {
			if (sections[0] == "v") {
				glm::vec3 newV = glm::vec3(-scale*std::stof(sections[1]),scale*std::stof(sections[2]),scale*std::stof(sections[3]));
				//std::cout << -1*newV.x << ","<< newV.y << "," << newV.z<< "\n";
				vertices.push_back(newV);
			} else if (sections[0] == "f") {
				int v0Pos = std::stoi(split(sections[1], '/')[0]) -1;
				int v1Pos = std::stoi(split(sections[2], '/')[0]) -1;
				int v2Pos = std::stoi(split(sections[3], '/')[0]) -1;
				ModelTriangle tri = ModelTriangle(vertices[v0Pos],vertices[v1Pos],vertices[v2Pos],curCol);
				glm::vec3 normal = glm::cross(tri.vertices[1] - tri.vertices[0], tri.vertices[2]-tri.vertices[0]);
				tri.normal = normal;
				triangles.push_back(tri);
			} else if (sections[0] == "usemtl") {
				curCol = objColours[sections[1]];
			} 	
		}
	}
	MyReadFile.close();


	return triangles;
}

CanvasPoint getCanvasIntersectionPoint(DrawingWindow &window, glm::vec3 cameraPosition,glm::mat3 camOrientation, glm::vec3 vertexPosition,float focalLength, float planeScaler) {
	//std::cout << "we made it canvas intersection\n";

	int x = round(planeScaler * focalLength * ((vertexPosition  - cameraPosition)*camOrientation).x / ((vertexPosition  - cameraPosition)*camOrientation).z + (window.width/2));
	int y = round(planeScaler * focalLength * ((vertexPosition  - cameraPosition)*camOrientation).y / ((vertexPosition  - cameraPosition)*camOrientation).z + (window.height/2));
	//std::cout << "camera pos " << (cameraPosition*camOrientation).x << "," << (cameraPosition*camOrientation).y << "," << (cameraPosition*camOrientation).z << "\n";
	//std::cout << "vertexPosition " << vertexPosition.z << "\n";
	float updatedDepth = 1 / ((vertexPosition  - cameraPosition)*camOrientation).z;
	//std::cout << "depth " << updatedDepth << "we made it\n";
	updatedDepth = 1 / (1 + exp(updatedDepth)); //sigmoid to allow depth printing function
	return CanvasPoint(x,y,updatedDepth);
}

RayTriangleIntersection getClosestIntersection(glm::vec3 ray, glm::vec3 cameraPos,std::vector<ModelTriangle> triangles) {

	RayTriangleIntersection theRay =  RayTriangleIntersection();

	theRay.distanceFromCamera = 1000;

	for (size_t i=0;i<triangles.size();i++) {
		glm::vec3 e0 = triangles[i].vertices[1] - triangles[i].vertices[0];
		glm::vec3 e1 = triangles[i].vertices[2] - triangles[i].vertices[0];
		glm::vec3 SPVector = (cameraPos - triangles[i].vertices[0]);
		glm::mat3 DEMatrix(-ray, e0, e1);
		const glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector; //t,u,v
		
		if (possibleSolution.x < theRay.distanceFromCamera && possibleSolution.x > 0.0005 ) { 
			if ((possibleSolution.y >= 0.0) && (possibleSolution.y <= 1.0) && (possibleSolution.z >= 0.0) && (possibleSolution.z <= 1.0) && (possibleSolution.y + possibleSolution.z) <= 1.0) {
				//std::cout << "smallest yet " << possibleSolution.x << "\n";
				theRay.intersectionPoint = triangles[i].vertices[0] + possibleSolution.y * e0 + possibleSolution.z * e1;
				theRay.distanceFromCamera = possibleSolution.x;
				theRay.intersectedTriangle = triangles[i];
				theRay.triangleIndex = i;
				//std::cout << "theRay triangle index: " << int(theRay.triangleIndex) <<"\n";
			}
		}
	}
	return theRay;
}

void drawDepth(DrawingWindow &window,std::vector<std::vector<float>> depthBuffer) {
	//std::cout <<  "depthBuffer.size" <<depthBuffer.size()<< "  depthBuffer[].size " << depthBuffer[0].size() << "\n";
	for (size_t x = 0; x < depthBuffer.size(); x++)
	{
		for (size_t y = 10; y < depthBuffer[0].size(); y++)
		{
			uint32_t colour_32 = (255 << 24) + ( int(round(255 * depthBuffer[x][y])) << 16) + (int(round(255 * depthBuffer[x][y])) << 8) + (int(round(255 * depthBuffer[x][y])));
			window.setPixelColour(x,y, colour_32);
		}
	}
}

void lookAt(glm::mat3 &cameraOrientation,glm::vec3 cameraPos ,glm::vec3 point) {
  	//glm::vec3 forward = glm::vec3(0,0,0.5); //CP vertical with forward
	glm::vec3 forward = cameraPos-point;
	forward = glm::normalize(forward);
	//std::cout << "foward: " << forward.x << "," << forward.y << "," << forward.z << "\n";
  	glm::vec3 right = glm::cross(glm::vec3(0,1,0),forward); //CP vertical with forward

  	//glm::vec3 right = glm::vec3(1,0,0); //CP vertical with forward
	//std::cout << "right: " << right.x << "," << right.y << "," << right.z << "\n";
  	glm::vec3 up = glm::vec3(0,1,0); //CP vertical with forward
	//glm::vec3 up = glm::cross(forward,right);
	//std::cout << "up: " << up.x << "," << up.y << "," << up.z << "\n";


	cameraOrientation = glm::mat3(right,up,forward);

}

void orbit(glm::vec3 &cameraPos, glm::mat3 &cameraOrientation, glm::vec3 &lookAtPoint) {
	const float orbitStep = M_PI/30;
	glm::mat3 clock_rot = glm::mat3(
			cos(-orbitStep), 0, -sin(-orbitStep), // first column (not row!)
			0, 1, 0, // second column
			sin(-orbitStep),0, cos(-orbitStep)  // third column
	);
	cameraPos = cameraPos * clock_rot; 

	lookAt(cameraOrientation,cameraPos,lookAtPoint);


}

void drawRayTrace(DrawingWindow &window,std::vector<ModelTriangle> triangles, glm::vec3 cameraPos, glm::mat3 cameraOrientation ) {

	const float focalL = 2;
	//const float planeScaler = HEIGHT/focalL + HEIGHT/3;
	const float planeScaler = 200 ;
	


	//glm::vec3 lookAtPoint = glm::vec3(0,0,0);
	//orbit(cameraPos,cameraOrientation,lookAtPoint);

	/* glm::vec3 testCamPos = glm::vec3(0,0,4);
	glm::vec3 rayDirection = glm::vec3(-0.1,-0.1,-2);
	RayTriangleIntersection test = getClosestIntersection(rayDirection,testCamPos, triangles);
	std::cout << "closest distance: " << test.distanceFromCamera << "\n";
	int ok;
	std::cin >> ok; */

	window.clearPixels();
	for (float x = 0; x < window.width; x++) {
		for (float y = 0; y < window.height; y++){
			//std::cout << "image plane coord " << x << "," << y<<"\n";
			//float newX = (x-(window.width/2)) / (focalL*planeScaler) + ((cameraPos*cameraOrientation).x/(cameraPos*cameraOrientation).z); 
			float newX = (x-(window.width/2)) / (focalL*planeScaler); 
			//newX = base x value + x/z
			//float newY = (y-window.height/2) / (focalL*planeScaler) + ((cameraPos*cameraOrientation).y/(cameraPos*cameraOrientation).z);
			float newY = (y-window.height/2) / (focalL*planeScaler);
	
			glm::vec3 ray = glm::vec3(-newX,-newY,-1.0f); //ray from camera to object
			ray = ray * glm::inverse(cameraOrientation);
			RayTriangleIntersection inter = getClosestIntersection(ray,cameraPos,triangles);
			if (inter.distanceFromCamera < 1000 ) { //default max distance 1000 if no object is hit by ray
				
				glm::vec3 lightSource = glm::vec3(0,0.75,0);
				glm::vec3 shadowRay = (lightSource - inter.intersectionPoint); // / rayInvScalar;
				RayTriangleIntersection shadowInter = getClosestIntersection(shadowRay,inter.intersectionPoint,triangles);
			
				if (shadowInter.distanceFromCamera > 1) {
					Colour colour = triangles[inter.triangleIndex].colour;
					float brightness=0;
					//------proximity---------
					brightness += 1 / (0.1+  pow(glm::length(shadowRay),2)); 
					//----angle of incidence--
					brightness *= glm::dot(glm::normalize(inter.intersectedTriangle.normal),-glm::normalize(shadowRay)) ;
					//------specular----------
					glm::vec3 reflection = (-shadowRay) - (2.0f * inter.intersectedTriangle.normal * (glm::dot(-shadowRay,inter.intersectedTriangle.normal)));
					float specular = glm::dot(glm::normalize(reflection),glm::normalize(-ray));
					if (specular < 0) specular =0;
					brightness += pow(specular,250);

					if (brightness > 1) brightness =1;
					if (brightness < 0) brightness =0;

					//proxBrit =1 ;
					uint32_t colour_32 = (255 << 24) + (int(round(colour.red * brightness)) << 16) + (int(round(colour.green * brightness)) << 8) + int(round(colour.blue * brightness));
					window.setPixelColour(x,y,colour_32);
				}
			}
		}
	}

}

void drawWireframeScene(DrawingWindow &window,std::vector<ModelTriangle> triangles, glm::vec3 &cameraPos, glm::mat3 &cameraOrientation ) {
	const float focalL = 2;
	const float planeScaler = HEIGHT/focalL + HEIGHT/3;
	window.clearPixels();
	for (ModelTriangle triangle : triangles) {
		CanvasPoint v[3];
		for (size_t i = 0; i <3; i++){
			v[i] = getCanvasIntersectionPoint(window, cameraPos,cameraOrientation,triangle.vertices[i],focalL,planeScaler);
		}
		drawStrokedTriangle(window,CanvasTriangle(v[0],v[1],v[2]),Colour(255,255,255));
	}
}

void drawRasterisedScene(DrawingWindow &window,std::vector<ModelTriangle> triangles, glm::vec3 &cameraPos, glm::mat3 &cameraOrientation ) {
	std::vector<std::vector<float>> depthBuffer(window.width,std::vector<float>(window.height));
	const float focalL = 2;
	const float planeScaler = HEIGHT/focalL + HEIGHT/3;

	//glm::vec3 lookAtPoint = glm::vec3(0,0,0);
	//orbit(cameraPos,cameraOrientation,lookAtPoint);

	window.clearPixels();
	for (ModelTriangle triangle : triangles) {
				
		CanvasPoint v[3];
		for (size_t i = 0; i <3; i++){
			v[i] = getCanvasIntersectionPoint(window, cameraPos,cameraOrientation,triangle.vertices[i],focalL,planeScaler);
			//std::cout << "New z: " << v[i].depth << "\n";
		}
		if (v[0].x < 0 && v[1].x < 0 && v[2].x < 0) continue; //early validation to save time processing objects outside the image plane
		if (v[0].y < 0 && v[1].y < 0 && v[2].y < 0) continue;
		if (v[0].x > window.width && v[1].x > window.width && v[2].x < window.width) continue;
		if (v[0].y > window.height && v[1].y > window.height && v[2].x < window.height) continue;

		drawFilledTriangle(window,CanvasTriangle(v[0],v[1],v[2]),triangle.colour,depthBuffer);
		//drawStrokedTriangle(window,CanvasTriangle(v[0],v[1],v[2]),Colour(255,255,255));


	}
	//drawDepth(window,depthBuffer);
}

void handleEvent(SDL_Event event, DrawingWindow &window, glm::vec3 &cameraPos,glm::mat3 &cameraOrientation, int &renderTypeIndex) {
	const float xStep = 0.05; //objects coords scaled 0-1
	const float yStep = 0.05;
	const float zStep = 0.05;
	const float theta = M_PI/13 ; // 9 degree increments
	
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) { 
			std::cout << "LEFT" << std::endl;
			cameraPos.x = cameraPos.x -xStep;
		} else if (event.key.keysym.sym == SDLK_RIGHT) {
			cameraPos.x = cameraPos.x +xStep;
			std::cout << "RIGHT" << std::endl;
		} else if (event.key.keysym.sym == SDLK_UP) {
			std::cout << "camera y pos before " << cameraPos[1] << "\n";
			cameraPos.y = cameraPos.y-yStep; //y grows down
			std::cout << "camera y pos after " << cameraPos[1] << "\n";
			std::cout << "UP" << std::endl;
		} else if (event.key.keysym.sym == SDLK_DOWN) {
			cameraPos.y = cameraPos.y +yStep;
			std::cout << "DOWN" << std::endl;
		} else if (event.key.keysym.sym == SDLK_w) {
			cameraPos.z = cameraPos.z - zStep;
			std::cout << "z " << cameraPos.z << std::endl;
			std::cout << "FORWARD" << std::endl;
		} else if (event.key.keysym.sym == SDLK_s) {
			cameraPos.z = cameraPos.z + zStep;
			std::cout << "BACKWARD" << std::endl;
		} else if (event.key.keysym.sym == SDLK_d) {	
			glm::mat3 countClock_rot = glm::mat3(
 			  	1, 0, 0, // first column (not row!)
   				0, cos(theta), sin(theta), // second column
   				0, -sin(theta), cos(theta)  // third column
			);
			cameraPos = cameraPos * countClock_rot;
			std::cout << "ROTATE X COUNTER-CLOCKWISE" << std::endl;

		} else if (event.key.keysym.sym == SDLK_a) {
			glm::mat3 clock_rot = glm::mat3(
 			  	1, 0, 0, // first column (not row!)
   				0, cos(-theta), sin(-theta), // second column
   				0, -sin(-theta), cos(-theta)  // third column
			);
			cameraPos = cameraPos * clock_rot;
			std::cout << "ROTATE X CLOCKWISE" << std::endl;
		} else if (event.key.keysym.sym == SDLK_c) {

			glm::mat3 countClock_rot = glm::mat3(
 			  	cos(theta), 0, -sin(theta), // first column (not row!)
   				0, 1, 0, // second column
   				sin(theta),0, cos(theta)  // third column
			);
			std::cout << "cameraPosBefore " << cameraPos.x << "," << cameraPos.y << "," << cameraPos.z << "\n";
			cameraPos = cameraPos * countClock_rot;
			//cameraPos = cameraPos * cameraOrientation;
			std::cout << "cameraPosAfter " << cameraPos.x << "," << cameraPos.y << "," << cameraPos.z << "\n";
			lookAt(cameraOrientation,cameraPos, glm::vec3(0,0,0));


			std::cout << "ROTATE Y CLOCKWISE" << std::endl;

		} else if (event.key.keysym.sym == SDLK_z) {

			glm::mat3 clock_rot = glm::mat3(
 			  	cos(-theta), 0, -sin(-theta), // first column (not row!)
   				0, 1, 0, // second column
   				sin(-theta),0, cos(-theta)  // third column
			);

			cameraPos = cameraPos * clock_rot;
			lookAt(cameraOrientation,cameraPos, glm::vec3(0,0,0));

			std::cout << "cameraPos: " << cameraPos.x<<", " <<cameraPos.y<<", " <<cameraPos.z << std::endl;
			std::cout << "camera Orientation: " << std::endl;
			for (size_t i = 0; i < 3; i++)
			{
				for (size_t j = 0; j < 3; j++)
				{
					std::cout << cameraOrientation[i][j] << ",";
				}
				std::cout<<"\n";
			}
			


		
			std::cout << "ROTATE Y COUNTER-CLOCKWISE" << std::endl;

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

		} else if (event.key.keysym.sym == SDLK_o) {

			glm::vec3 lookAtPoint = glm::vec3(0,0,0);
			lookAt(cameraOrientation,cameraPos,lookAtPoint);

			for (size_t i = 0; i < 3; i++){
				for (size_t j = 0; j < 3; j++)
				{
					std::cout << cameraOrientation[i][j] << " , ";
				}
				std::cout << "\n";
			}
			
			
		} else if (event.key.keysym.sym == SDLK_f) {
			renderTypeIndex = 0;
			std::cout << "render type changed to draw wireframe scene\n";
		} else if (event.key.keysym.sym == SDLK_g) {
			renderTypeIndex = 1;
			std::cout << "render type changed to draw rasterised scene\n";

		} else if (event.key.keysym.sym == SDLK_h) {
			renderTypeIndex = 2;
			/* std::cout << "cameraPos: " << cameraPos.z << std::endl;
			std::cout << "camera Orientation: "<< std::endl;
			for (size_t i = 0; i < 3; i++)
			{
				for (size_t j = 0; j < 3; j++)
				{
					std::cout << cameraOrientation[i][j] << ",";
				}
				std::cout<<"\n";
			} */
			std::cout << "render type changed to draw ray trace scene\n";

		} else if (event.type == SDL_MOUSEBUTTONDOWN) {
			window.savePPM("output.ppm");
			window.saveBMP("output.bmp"); 
		}
	}

}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

	const float objScaler = 0.35;
	std::vector<ModelTriangle> triangles = loadObj("cornell-box.obj","cornell-box.mtl",objScaler);
	glm::vec3 cameraPos = glm::vec3(0,0,2);
	glm::mat3 camOrientation = glm::mat3(
		//									   | Right | Up  | Forward |
		1, 0, 0, // first column (not row!)  x |   1   ,  0  ,    0    |
		0, 1, 0, // second column		     y |   0   ,  1  ,    0    |
		0, 0, 1  // third column			 z |   0   ,  0  ,    1    |
	);
	
	int renderTypeIndex = 0;

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window, cameraPos, camOrientation,renderTypeIndex);
		//drawRayTrace(window, triangles, cameraPos,camOrientation);
		//drawRasterisedScene(window, triangles, cameraPos,camOrientation);
		if (renderTypeIndex == 0) {
			drawWireframeScene(window,triangles,cameraPos,camOrientation);
		} else if(renderTypeIndex == 1) {
			drawRasterisedScene(window,triangles,cameraPos,camOrientation);
		} else if (renderTypeIndex == 2) {
			drawRayTrace(window,triangles,cameraPos,camOrientation);
			window.renderFrame();
			std::cout<< "Ray trace scene rendered! Enter Y to continue...\n";
			char ok;
			std::cin >> ok;
			renderTypeIndex = 0;
		}

		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
		/* std::cout << "Render complete!\n";
		int x;
		std::cin >> x; */
	}
}
