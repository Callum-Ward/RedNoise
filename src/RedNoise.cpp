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
#include <algorithm>
#include <unordered_map>

//#define WIDTH 960
//#define HEIGHT 720
#define WIDTH 640
#define HEIGHT 480
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
	//get a depth value for a point based on its position along the line connecting traingle vertices

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
	//std::cout << "drawLineDepth\n";

	if (from.x == to.x && from.y == to.y && from.x>=0 && from.x < window.width && from.y >=0 && from.y < window.height) {
		if ((1 / (1+from.depth)) > depthBuffer[from.x][from.y]){
			depthBuffer[from.x][from.y] = 1/ (1+from.depth);
			//std::cout << "colour.istexture: " << colour.isTexture << "\n";
			if (colour.isTexture) {
				window.setPixelColour(from.x,from.y, colour.rowTexture[0]);
			} else window.setPixelColour(from.x,from.y, colour_32);
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
				float depth = getPointDepth(from,to,x,y) ; 
				if ( 1/ (1+ depth) > depthBuffer[x][y]){
					depthBuffer[x][y] = (1/(1+ depth));
					if (colour.isTexture) {
						window.setPixelColour(x,y, colour.rowTexture[i]);
					} else window.setPixelColour(x,y, colour_32);

				} 
			}
		}
	}
}

void drawLine(DrawingWindow &window, CanvasPoint &from, CanvasPoint &to, Colour &colour) {
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
			if (x>=0 && x < window.width && y>=0 && y < window.height ) {
				window.setPixelColour(x,y, colour_32);
			}
			
		}
	}
}

void drawStrokedTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {
	//std::cout << "drawStroked\n";
	//window.clearPixels();
	drawLine(window,triangle.v0(),triangle.v1(),colour);
	drawLine(window,triangle.v1(),triangle.v2(),colour);
	drawLine(window,triangle.v2(),triangle.v0(),colour);
	//std::cout << "drawStroked2\n";

}

std::vector<uint32_t> getRowTexture(TextureMap texture,TexturePoint from, TexturePoint to){
	//std::cout << "getRowTexture\n";

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

std::vector<uint32_t> getScaledRowTexture(CanvasTriangle &triangle,CanvasPoint start,CanvasPoint end) {
	//get texture row vector based on start and end texture points
	//determine diffrence in length x and y
	//scale vector by interpolation
	std::vector<uint32_t> rowTexture = getRowTexture(triangle.textureMap,start.texturePoint,end.texturePoint);
	//std::cout << "getScaledRowTexture\n";

	std::vector<std::vector<int>> imgLineCoords = getLineCoords(start,end);

	if (rowTexture.size() > imgLineCoords.size() ) { //scale down by removing pixels periodically
		//std::cout << "size down\n";

		int pixelsToLose = rowTexture.size() - imgLineCoords.size();
		float stepToErase = float(rowTexture.size()) / float(pixelsToLose);
		//std::cout << "size down scale: " << (pixelsToLose / (float)rowTexture.size()) << "\n";

		for (int i = 0; i < pixelsToLose; i++) { //start erasing from 0	
			rowTexture.erase(rowTexture.begin()+ round(i*stepToErase)-i);
		} 

	} else if (rowTexture.size() < imgLineCoords.size()) {//scale up by interpolating between 2 neighbouring pixels periodically

		int pixelsToGain = imgLineCoords.size()-rowTexture.size();
		float stepToInsert = float(rowTexture.size()) / float(pixelsToGain);
		
		for (int i = 0; i < pixelsToGain; i++) {
			int insertIndex = round(stepToInsert*i)+i;
			uint32_t pixel = (255<<24) + (255<<16) + (255<<8) + 255; 
			rowTexture.insert(rowTexture.begin()+insertIndex,rowTexture[insertIndex]);
		}

		return rowTexture;
	}
	return rowTexture;
}

TexturePoint getCorrectedTexturePoint(CanvasTriangle triangle,CanvasPoint point,int vertex) {
	//interpolate texture point along edge between vertices
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
	float z0 = smallV.depth; //Z depth furthest vertex from camera
	float z1 = bigV.depth; //Z depth closest vertex from camera
	float c0 = smallV.texturePoint.y; //texture y coord furthest from camera
	float c1 = bigV.texturePoint.y; //texture y coord closest to camera
	float q = (point.y - smallV.y) / (bigV.y -smallV.y); // ratio along triangle from smallest y

 	if (smallV.y == bigV.y) { //edge case when both y are the same
		if(vertex == 0 || vertex == 1) q=0; //favour smaller y vertices when choosing starting point of line
		if (vertex ==2) q=1; //favour v2 when choosing a line endpoint
	}  

	float c; //row of the texture image we should use


	c = ( (c0*( 1-q) )/z0 + (c1*q)/z1 ) / ( (1-q)/z0 + q/z1 );

	if (c<0 || c > triangle.textureMap.height) {
		std::cout << "vertex: " << vertex << "\n";
		std::cout << "smallV.z: " << smallV.z << " bigV.z: " << bigV.z << "\n";
		std::cout << "smallV.texturePoint.y: " << smallV.texturePoint.y << "\n";
		std::cout << "bigV.texturePoint.y: " << bigV.texturePoint.y << "\n";
		std::cout << "q: " << q << "\n"; 

		int x; 
		std::cin >> x;
	}


	int xTexture = smallV.texturePoint.x+ round((bigV.texturePoint.x - smallV.texturePoint.x)*q);
	int yTexture = smallV.texturePoint.y+ round((bigV.texturePoint.y - smallV.texturePoint.y)*q);

	//std::cout << "yTexture: " << yTexture << "\n";
	//std::cout << "c: " << c << "\n"; 
	//std::cout << "xTexture: " << xTexture << " yTexture: " << yTexture << "\n";

	
	if (xTexture > triangle.textureMap.width ) {
		std::cout << "vertex: " << vertex << "\n";
		std::cout << "smallV.y: " << smallV.y << " bigV.y: " << bigV.y << "\n";
		std::cout << "point.y: " << point.y << "\n";
		std::cout << "ratio: " << q << "\n";
		std::cout << "xTexture: " << xTexture << "\n";
		
		std::cout << "this is rly messed upggg\n";
	}

	if (yTexture > triangle.textureMap.height) {
		std::cout << "vertex: " << vertex << "\n";
		std::cout << "smallV.y: " << smallV.y << " bigV.y: " << bigV.y << "\n";
		std::cout << "point.y: " << point.y << "\n";
		std::cout << "ratio: " << q << "\n";
		std::cout << "xTexture: " << xTexture << "\n";

		std::cout << "this is rly messed upggggg 2\n";
	}

	textureP = TexturePoint(xTexture,round(c));
	return textureP;
}

void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour &colour,std::vector<std::vector<float>> &depthBuffer) {

	if (triangle.v0().y > triangle.v1().y) {
		CanvasPoint v0 = triangle.v0();
		triangle.vertices[0] = triangle.vertices[1];
		triangle.vertices[1] = v0;
	}
	if (triangle.v1().y > triangle.v2().y) {
		CanvasPoint v1 = triangle.v1();
		triangle.vertices[1] = triangle.vertices[2];
		triangle.vertices[2] = v1;
		if (triangle.v0().y > triangle.v1().y) {
			CanvasPoint v0 = triangle.v0();
			triangle.vertices[0] = triangle.vertices[1];
			triangle.vertices[1] = v0;
		}
	} 

	float x[3] = {triangle.v0().x,triangle.v1().x,triangle.v2().x}; //moving all x and y to arrays to make future statements shorter
	float y[3] = {triangle.v0().y,triangle.v1().y,triangle.v2().y};
	float xStepSize[3],yStepSize[3];
	CanvasPoint lineStart;
	CanvasPoint lineEnd;	
	for (int c=0;c<2;++c){
		xStepSize[c] = (x[c+1] - x[0])/ fmax(abs(x[c+1] - x[0]),abs(y[c+1] - y[0])); //step[0] top
		yStepSize[c] = (y[c+1] - y[0]) / fmax(abs(y[c+1] - y[0]),abs(x[c+1] - x[0]));//step[1] full length line
	}
	xStepSize[2] = (x[2]-x[1]) / fmax(abs(x[2] - x[1]),abs(y[2] - y[1])); //step[2] second stage line
	yStepSize[2] = (y[2]-y[1]) / fmax(abs(x[2] - x[1]),abs(y[2] - y[1]));
	float curX[2] = {x[0],x[0]};
	float curY[2] = {y[0],y[0]};

	for (size_t row = y[0]; row <= y[2]; row++){
		if (row == y[1]){ //if v0.y == v1.y == v2.y then only v1 to v2 is drawn
			xStepSize[0] = xStepSize[2];
			yStepSize[0] = yStepSize[2];
			curX[0] = x[1];
			curY[0] = y[1];		
		}
		
		for (size_t i = 0; i < 2; i++){
			while(round(curY[i]) == row){	

				if (row < y[1]) {
					lineStart = CanvasPoint(round(curX[0]),row, getPointDepth(triangle.v0(),triangle.v1(), round(curX[0]),row)); 
					if (triangle.isTexture) lineStart.texturePoint = getCorrectedTexturePoint(triangle,lineStart,0);
				} else {
					lineStart = CanvasPoint(round(curX[0]),row, getPointDepth(triangle.v1(),triangle.v2(), round(curX[0]),row)); 
					if (triangle.isTexture) lineStart.texturePoint = getCorrectedTexturePoint(triangle, lineStart,1);
				}

				lineEnd = CanvasPoint(round(curX[1]),row, getPointDepth(triangle.v0(),triangle.v2(), round(curX[1]),row));
				if (triangle.isTexture) lineEnd.texturePoint = getCorrectedTexturePoint(triangle, lineEnd,2);

				//if (lineStart.depth < 0 && lineEnd.depth < 0) continue;

				if (triangle.isTexture) {
					std::vector<uint32_t> rowTexture = getScaledRowTexture(triangle,lineStart,lineEnd); //get the exact number of pixels required to draw in image by scaling fetched texture
					colour.rowTexture = rowTexture;
					colour.isTexture = 1;
				}

				drawLineDepth(window,lineStart,lineEnd,colour,depthBuffer);

				//drawLineDepthOriginal(window,lineStart,lineEnd,colour,depthBuffer);
				//if (yStepSize[i] == 0) break;
				if (round(curX[i]) == x[2] && round(curY[i]) == y[2]) break;
				curX[i]+=xStepSize[i];
				curY[i]+=yStepSize[i];
			}
		}
		
	}
/* 	std::cout << "traingle.v0.x: " << triangle.v0().x << "  y: " << triangle.v0().y  << "\n"; 
	std::cout << "traingle.v1.x: " << triangle.v1().x << "  y: " << triangle.v1().y  << "\n"; 
	std::cout << "traingle.v2.x: " << triangle.v2().x << "  y: " << triangle.v2().y  << "\n"; 
	std::cout << "traingle.v0.texture.x: " << triangle.v0().texturePoint.x << "  y: " << triangle.v0().texturePoint.y  << "\n"; 
	std::cout << "traingle.v1.texture.x: " << triangle.v1().texturePoint.x << "  y: " << triangle.v1().texturePoint.y  << "\n"; 
	std::cout << "traingle.v2.texture.x: " << triangle.v2().texturePoint.x << "  y: " << triangle.v2().texturePoint.y  << "\n"; 
	int f;
	std::cin >> f; */

	//drawStrokedTriangle(window,triangle,Colour(255,255,255));
	//window.setPixelColour(round(x),round(y), colour_32);
}

std::vector<ModelTriangle> loadObj(std::string objFilename, std::string mtlFilename, float scale) {
	std::string line;
	std::vector<ModelTriangle> triangles;
	std::vector<glm::vec3> vertices;
	std::vector<TexturePoint> txtrPoints;
	std::unordered_map<std::string, Colour> objColours;
	std::string colourName;
	std::string objectName; 
	std::vector<std::string> sections;

 	/* std::ifstream ReadFile("cornell-box copy.obj");
	std::vector<std::string> newFaces;
	std::vector<std::string> newVertices;

	while (getline (ReadFile, line)) {
		sections = split(line, ' ');
		if (sections.size()>0) {

			if (sections[0] == "f" && objectName == "robot") {
				std::vector<std::string> sec1 = split(sections[1], '/');
				std::vector<std::string> sec2 = split(sections[2], '/');
				std::vector<std::string> sec3 = split(sections[3], '/');
				std::string newFace = "f " +  std::to_string(std::stoi(sec1[0])+ 122) + "/ " +  std::to_string(std::stoi(sec2[0])+ 122) + "/ " +  std::to_string(std::stoi(sec3[0])+ 122) + "/";
				if (sec1[1] != "") { //if texture points exist
					int t1Pos = std::stoi(sec1[1]) +52;
					int t2Pos = std::stoi(sec2[1]) + 52;
					int t3Pos = std::stoi(sec3[1]) + 52;
					std::string vtPair = std::to_string(std::stoi(sec1[0]) + 238) + "/" + std::to_string(t1Pos);
					std::string vtPair1 = std::to_string(std::stoi(sec2[0]) + 238 ) + "/" + std::to_string(t2Pos);
					std::string vtPair2 = std::to_string(std::stoi(sec3[0])+ 238) + "/" + std::to_string(t3Pos);
					newFace = "f " +  vtPair + " " +   vtPair1 + " " +  vtPair2;
				}
				newFaces.push_back(newFace);

			} else if (sections[0] == "o") {
				objectName = sections[1];
				std::cout << objectName << "\n";
			}    else if (sections[0] == "v") {
				std::string newFace = "v " +  std::to_string(std::stof(sections[1])/4) + " " +  std::to_string(std::stof(sections[2])/4) + " " + std::to_string(std::stof(sections[3])/4) ;
				newVertices.push_back(newFace);
			}   
		}
	}
	ReadFile.close();

	
    std::ofstream myfile("logo updated.obj");

    if(myfile.is_open())
    {	
		for (std::string newFace : newFaces) {
			myfile<<newFace<< std::endl;
		}
		myfile << std::endl;

	 	for (std::string newVertex : newVertices) {
			myfile<<newVertex<< std::endl;
		}  
        myfile.close(); 
    }
	int x;
	std::cin >> x;   */


	std::ifstream MyReadFileMtl(mtlFilename);
	Colour colour;
	while (getline (MyReadFileMtl, line)) {
		sections = split(line, ' ');
		if (sections.size()>0) {
			if (sections[0] == "Kd") {
				objColours.insert({colourName, Colour(colourName,std::stof(sections[1]),std::stof(sections[2]),std::stof(sections[3]))}); 
			} else if (sections[0] == "map_Kd") {
				objColours.insert({colourName, Colour(sections[1],std::stof(sections[2]),std::stof(sections[3]),std::stof(sections[4]))}); 
			} else if (sections[0] == "newmtl") {
				colourName = sections[1];
			} 
		}
		sections.clear();
	}
	MyReadFileMtl.close();
	std::cout << "Read material file succesfully\n";
	
	Colour curCol;
	std::string surfaceType;
	std::ifstream MyReadFile(objFilename);
	glm::vec3 sphereV = glm::vec3(0,3,0);
	while (getline (MyReadFile, line)) {
		sections = split(line, ' ');
		if (sections.size()>0) {

			if (sections[0] == "v") {
				glm::vec3 newV = glm::vec3(-scale*std::stof(sections[1]),scale*std::stof(sections[2]),scale*std::stof(sections[3]));
				vertices.push_back(newV);		
			} else if (sections[0] == "vt") {
				TexturePoint tp = TexturePoint(std::stof(sections[1]),std::stof(sections[2]));
  				txtrPoints.push_back(tp);

			} else if (sections[0] == "f") {
				std::vector<std::string> sec1 = split(sections[1], '/');
				std::vector<std::string> sec2 = split(sections[2], '/');
				std::vector<std::string> sec3 = split(sections[3], '/');
				int v0Pos = std::stoi(sec1[0])-1;
				int v1Pos = std::stoi(sec2[0])-1;
				int v2Pos = std::stoi(sec3[0])-1;

				ModelTriangle tri;
				if (sec1[1] != "") { //if texture points exist
					tri = ModelTriangle(vertices[v0Pos],vertices[v1Pos],vertices[v2Pos],curCol.name,surfaceType);//curCol.name is texture filename
					int t1Pos = std::stoi(sec1[1]) -1;
					int t2Pos = std::stoi(sec2[1]) -1;
					int t3Pos = std::stoi(sec3[1]) -1;
	
					TexturePoint tp1 = TexturePoint(tri.textureMap.width * (txtrPoints[t1Pos].x), tri.textureMap.height *(txtrPoints[t1Pos].y));
					TexturePoint tp2 = TexturePoint(tri.textureMap.width *(txtrPoints[t2Pos].x), tri.textureMap.height *(txtrPoints[t2Pos].y));;
					TexturePoint tp3 = TexturePoint(tri.textureMap.width *(txtrPoints[t3Pos].x), tri.textureMap.height *(txtrPoints[t3Pos].y));
					if (tp1.x > 0) tp1.x -= 1;
					if (tp2.x > 0) tp2.x -= 1;
					if (tp3.x > 0) tp3.x -= 1;
					if (tp1.y > 0) tp1.y -= 1;
					if (tp2.y > 0) tp2.y -= 1;
					if (tp3.y > 0) tp3.y -= 1;
				
					std::array<TexturePoint, 3> triTexturePoints = {tp1,tp2,tp3};
					tri.texturePoints = triTexturePoints; 

				} else tri = ModelTriangle(vertices[v0Pos],vertices[v1Pos],vertices[v2Pos],curCol,surfaceType); 

				glm::vec3 normal = -glm::cross(tri.vertices[1] - tri.vertices[0], tri.vertices[2]-tri.vertices[0]);
				tri.normal = normal;
				

				triangles.push_back(tri);

			} else if (sections[0] == "usemtl") {
				curCol = objColours[sections[1]];
			} else if (sections[0] == "surface") {
				surfaceType = sections[1];
			} else if (sections[0] == "o") {
				objectName = sections[1];
				
			}
		}
	}
	MyReadFile.close();
	std::cout << "textpoint read: " << txtrPoints.size() << "\n";
	std::cout << "Loaded " << triangles.size() << " triangles\n";
	std::cout << "sphere bottom: " << sphereV.x << ", " << sphereV.y << ", " << sphereV.z << "\n";


	std::array<glm::vec3, 3> normals;
	for (size_t outerTri = 0; outerTri < triangles.size(); outerTri++) {
		if (triangles[outerTri].surfaceType == "flat") continue;
		int index = 0;
		for (size_t outerV = 0; outerV < 3; outerV++) {
			std::vector<glm::vec3> nbrNormals; //vector to store all neighbouring vertex normals
			for (size_t innerTri = 0; innerTri < triangles.size(); innerTri++) {
				for (size_t innerV = 0; innerV < 3; innerV++) {
					if (triangles[outerTri].vertices[outerV] == triangles[innerTri].vertices[innerV] ) {
						nbrNormals.push_back(triangles[innerTri].normal);
						break;
					}
				}
			}
			glm::vec3 vNormal = glm::vec3(0,0,0);
			for (glm::vec3 normal : nbrNormals){
				vNormal += normal;
			}
			vNormal /= nbrNormals.size();
			normals[index] = vNormal;
			index++;
		}
		triangles[outerTri].normals = normals;
	}

	return triangles;
}

CanvasPoint getCanvasIntersectionPoint(DrawingWindow &window, glm::vec3 cameraPosition,glm::mat3 camOrientation, glm::vec3 vertexPosition,float focalLength, float planeScaler) {
	//std::cout << "we made it canvas intersection\n";
	int x = round(planeScaler * focalLength * ((vertexPosition  - cameraPosition)*camOrientation).x / ((vertexPosition  - cameraPosition)*camOrientation).z + (window.width/2));
	int y = round(planeScaler * focalLength * ((vertexPosition  - cameraPosition)*camOrientation).y / ((vertexPosition  - cameraPosition)*camOrientation).z + (window.height/2));

	//float updatedDepth = 1 / ((vertexPosition  - cameraPosition)*camOrientation).z; //smaller distance to vertex results in large updated depth
	float updatedDepth = sqrt(pow(((vertexPosition  - cameraPosition)*camOrientation).z,2)); //smaller distance to vertex results in large updated depth

	//updatedDepth = 1 / (1 + exp(updatedDepth)); //closer objects results in a smaller depth
	CanvasPoint point = CanvasPoint(x,y,updatedDepth);
	return point;

}

glm::vec3 refract(const glm::vec3 &I, const glm::vec3 &N, const float &ior) 
{ 
	glm::vec3 none= glm::vec3(0,0,0);
    float cosi = glm::clamp (-1.0f, 1.0f, glm::dot(I, N)); 
	
    float etai = 1, etat = ior; 
    glm::vec3 n = N; 
    if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); n= -N; } 
    float eta = etai / etat; 
    float k = 1 - eta * eta * (1 - cosi * cosi); 
    return k < 0 ? none : eta * I + (eta * cosi - sqrtf(k)) * n; 
} 

float fresnel(const glm::vec3 &I, const glm::vec3 &N, const float &ior) 
{ 
	float kr;
    float cosi = glm::clamp(-1.0f, 1.0f, glm::dot(I, N)); 
    float etai = 1, etat = ior; 
    if (cosi > 0) { std::swap(etai, etat); } 
    // Compute sini using Snell's law
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi)); 
    // Total internal reflection
    if (sint >= 1) { 
        kr = 1; 
    } 
    else { 
        float cost = sqrtf(std::max(0.f, 1 - sint * sint)); 
        cosi = fabsf(cosi); 
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost)); 
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost)); 
        kr = (Rs * Rs + Rp * Rp) / 2; 
    } 
	return kr;
    // As a consequence of the conservation of energy, transmittance is given by:
    // kt = 1 - kr;
} 

RayTriangleIntersection getClosestIntersection(glm::vec3 ray, glm::vec3 cameraPos,std::vector<ModelTriangle> &triangles, int shadowRay,int fresnelCount,int findMapping,int phong) {
	RayTriangleIntersection theRay =  RayTriangleIntersection();
	float bestT ;
	float bestU ;
	float bestV ;
	theRay.distanceFromCamera = 1000;
	uint32_t pixel;
	for (size_t i=0;i<triangles.size();i++) {
		//if (triangles[i].colour.name == "Glass" && shadowRay) continue;
		if (triangles[i].surfaceType == "map" && !findMapping) continue;
		//if (triangles[i].surfaceType != "map" && findMapping) continue;
		glm::vec3 e0 = triangles[i].vertices[1] - triangles[i].vertices[0];
		glm::vec3 e1 = triangles[i].vertices[2] - triangles[i].vertices[0];
		glm::vec3 SPVector = (cameraPos - triangles[i].vertices[0]);
		glm::mat3 DEMatrix(-ray, e0, e1);
		const glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector; //t,u,v
		float t = possibleSolution[0];
		float u = possibleSolution[1];
		float v = possibleSolution[2];
	
		if (t < theRay.distanceFromCamera && t > 0.0005 ) { 
			if ((u >= 0.0) && (u <= 1.0) && (v >= 0.0) && (v <= 1.0) && (u + v) <= 1.0) {
				theRay.intersectionPoint = triangles[i].vertices[0] + u * e0 + v * e1;
				theRay.distanceFromCamera = t;
				theRay.intersectedTriangle = triangles[i];
				theRay.triangleIndex = i;
				float bestT = t;
				float bestU = u;
				float bestV = v;
				if (phong && !shadowRay) {
					if (theRay.intersectedTriangle.isTexture==1 && theRay.intersectedTriangle.surfaceType == "flat" || theRay.intersectedTriangle.surfaceType == "map") {
						float w = 1 - (bestU+ bestV);
						float xTxtPoint = (w * theRay.intersectedTriangle.texturePoints[0].x)+(bestU* theRay.intersectedTriangle.texturePoints[1].x)+(bestV*theRay.intersectedTriangle.texturePoints[2].x);
						float yTxtPoint = (w * theRay.intersectedTriangle.texturePoints[0].y)+(bestU* theRay.intersectedTriangle.texturePoints[1].y)+(bestV*theRay.intersectedTriangle.texturePoints[2].y);
						pixel = theRay.intersectedTriangle.textureMap.pixels[theRay.intersectedTriangle.textureMap.width*(round(yTxtPoint)) + (round(xTxtPoint))];
						theRay.colour = Colour(((pixel>>16) & 255), ((pixel>>8)&255), (pixel&255));
					} else theRay.colour = theRay.intersectedTriangle.colour;

					if (theRay.intersectedTriangle.surfaceType == "flat") {
						theRay.normal = theRay.intersectedTriangle.normal;
					} else if (theRay.intersectedTriangle.isTexture && theRay.intersectedTriangle.surfaceType == "smooth") {
						float w = 1 - (bestU+ bestV);
						float xTxtPoint = (w * theRay.intersectedTriangle.texturePoints[0].x)+(bestU* theRay.intersectedTriangle.texturePoints[1].x)+(bestV*theRay.intersectedTriangle.texturePoints[2].x);
						float yTxtPoint = (w * theRay.intersectedTriangle.texturePoints[0].y)+(bestU* theRay.intersectedTriangle.texturePoints[1].y)+(bestV*theRay.intersectedTriangle.texturePoints[2].y);
						pixel = theRay.intersectedTriangle.textureMap.pixels[theRay.intersectedTriangle.textureMap.width*(round(yTxtPoint)) + (round(xTxtPoint))];
						theRay.normal = glm::vec3( float((pixel>>16) & 255) / 255, float((pixel>>8)&255)/255,float(pixel&255)/255 );
						theRay.colour = Colour(0,255,0);
					} else {
						float w = 1 - (bestU+ bestV);
						theRay.normal = glm::normalize((w * theRay.intersectedTriangle.normals[0])+(bestU* theRay.intersectedTriangle.normals[1])+(bestV* theRay.intersectedTriangle.normals[2]));
					} 
				} else if (!shadowRay) {
					float w = 1 - (bestU+ bestV);
					uint32_t v1Col = (255 << 24) + (theRay.intersectedTriangle.colours[0].red << 16) + (theRay.intersectedTriangle.colours[0].green << 8) + theRay.intersectedTriangle.colours[0].blue;
					uint32_t v2Col = (255 << 24) + (theRay.intersectedTriangle.colours[1].red << 16) + (theRay.intersectedTriangle.colours[1].green << 8) + theRay.intersectedTriangle.colours[1].blue;
					uint32_t v3Col = (255 << 24) + (theRay.intersectedTriangle.colours[2].red << 16) + (theRay.intersectedTriangle.colours[2].green << 8) + theRay.intersectedTriangle.colours[2].blue;
					uint32_t pixel = uint32_t(w*int(v1Col) + bestU*int(v2Col) + bestV*int(v3Col));
					Colour c1 = theRay.intersectedTriangle.colours[0];
					Colour c2 = theRay.intersectedTriangle.colours[1];
					Colour c3 = theRay.intersectedTriangle.colours[2];
					Colour newColour = Colour(c1.red*w+c2.red*bestU+c3.red*bestV,c1.green*w+c2.green*bestU+c3.green*bestV,c1.blue*w+c2.blue*bestU+c3.blue*bestV); 
					theRay.colour = newColour;
				}
				

			}
		}
	}	

	if (shadowRay == 0 && theRay.distanceFromCamera < 1000 && phong) {
		Colour colour;
		if (theRay.intersectedTriangle.colour.name == "Reflective"){			
			glm::vec3 reflection = glm::normalize(ray) - (2.0f * (glm::normalize(theRay.normal)) * (glm::dot(glm::normalize(ray),glm::normalize(theRay.normal))));
			theRay = getClosestIntersection(reflection,theRay.intersectionPoint,triangles,0,0,1,1);
		} else if (theRay.intersectedTriangle.colour.name == "Metal") {

			float reflecMul = 0.2;
			glm::vec3 reflection = (glm::normalize(ray)) - (2.0f * glm::normalize(theRay.normal) * (glm::dot(glm::normalize(ray), glm::normalize(theRay.normal) ) ) );
			RayTriangleIntersection reflectRay = getClosestIntersection(glm::normalize(reflection),theRay.intersectionPoint,triangles,0,0,1,1);	
			Colour tC = theRay.colour; //transmission colour
			Colour rC = reflectRay.colour;
			Colour updatedColour = Colour((1-reflecMul)*tC.red + reflecMul*rC.red, (1-reflecMul)*tC.green + reflecMul*rC.green,(1-reflecMul)*tC.blue + reflecMul*rC.blue );
			theRay.colour = updatedColour;
			
		} else if (theRay.intersectedTriangle.colour.name == "Glass" ){

			if (glm::dot(glm::normalize(ray), glm::normalize(theRay.normal)) > 0) { //if ray inside glass


				glm::vec3 refractedRay = refract(glm::normalize(ray),glm::normalize(theRay.normal),0.645);
				
				if (refractedRay == glm::vec3(0,0,0)){ //total internal reflection
					glm::vec3 reflection = (glm::normalize(ray)) - (2.0f * glm::normalize(-theRay.normal) * (glm::dot(glm::normalize(ray), glm::normalize(-theRay.normal) ) ) );	
					theRay = getClosestIntersection(glm::normalize(reflection),theRay.intersectionPoint,triangles,0,fresnelCount,0,1);
				} else {

					float reflecMul = 0;
					Colour rC = theRay.colour;
					if (fresnelCount < 2) {
						reflecMul = fresnel(glm::normalize(ray), glm::normalize(theRay.normal), 0.645); //reflection ratio  (transmission = 1 - reflecMul)
						glm::vec3 reflection = (glm::normalize(ray)) - (2.0f * glm::normalize(-theRay.normal) * (glm::dot(-glm::normalize(ray), glm::normalize(-theRay.normal) ) ) );
						RayTriangleIntersection reflectRay = getClosestIntersection(glm::normalize(reflection),theRay.intersectionPoint,triangles,0,fresnelCount++,1,1);	
						rC = reflectRay.colour;
					}


					theRay = getClosestIntersection(glm::normalize(refractedRay),theRay.intersectionPoint,triangles,0,fresnelCount,0,1);
					Colour tC = theRay.colour; //transmission colour
					Colour updatedColour = Colour((1-reflecMul)*tC.red + reflecMul*rC.red, (1-reflecMul)*tC.green + reflecMul*rC.green,(1-reflecMul)*tC.blue + reflecMul*rC.blue );
					theRay.colour = updatedColour;
				}

			} else {

				glm::vec3 refractedRay = refract(glm::normalize(ray),glm::normalize(theRay.normal),1.55);
				
				float reflecMul = fresnel(glm::normalize(ray), glm::normalize(theRay.normal), 1.55); //reflection ratio  (transmission = 1 - reflecMul)

				glm::vec3 reflection = (glm::normalize(ray)) - (2.0f * glm::normalize(theRay.normal) * (glm::dot(glm::normalize(ray), glm::normalize(theRay.normal) ) ) );
				RayTriangleIntersection reflectRay = getClosestIntersection(glm::normalize(reflection),theRay.intersectionPoint,triangles,0,0,1,1);	
			
				theRay = getClosestIntersection(glm::normalize(refractedRay),theRay.intersectionPoint,triangles,0,0,0,1);
				Colour tC = theRay.colour; //transmission colour
				Colour rC = reflectRay.colour;
				Colour updatedColour = Colour((1-reflecMul)*tC.red + reflecMul*rC.red, (1-reflecMul)*tC.green + reflecMul*rC.green,(1-reflecMul)*tC.blue + reflecMul*rC.blue );
				theRay.colour = updatedColour;

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
  	glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0,1,0),forward)); //CP vertical with forward

  	//glm::vec3 right = glm::vec3(1,0,0); //CP vertical with forward
	//std::cout << "right: " << right.x << "," << right.y << "," << right.z << "\n";
  	glm::vec3 up = glm::normalize(glm::cross(forward,right)); //CP vertical with forward
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

void drawRayTrace(DrawingWindow &window,std::vector<ModelTriangle> &triangles, std::vector<glm::vec3> &lightSources,glm::vec3 cameraPos, glm::mat3 cameraOrientation,int phong ) {
	const float focalL = 2;
	const float planeScaler = HEIGHT/focalL + HEIGHT/3;
	float largestBright = 0;
	window.clearPixels();
	if (!phong) {
		for (size_t trianglePos = 0; trianglePos < triangles.size(); trianglePos++) {
			for (size_t vertexPos = 0; vertexPos < 3;vertexPos++){
				float brightness =0;
				for (glm::vec3 lightSource : lightSources) {
					glm::vec3 shadowRay = (lightSource - triangles[trianglePos].vertices[vertexPos]); // / rayInvScalar;
					RayTriangleIntersection shadowInter = getClosestIntersection(shadowRay,triangles[trianglePos].vertices[vertexPos],triangles,1,0,0,1);

					if (shadowInter.distanceFromCamera > 1) {
						//------proximity---------
						brightness += 1 / (20* ( pow(glm::length(shadowRay),2))); 
						//----angle of incidence--
						float incidentDot;
						if (triangles[trianglePos].surfaceType == "smooth"){
							incidentDot= glm::dot(glm::normalize(triangles[trianglePos].normals[vertexPos]) ,glm::normalize(shadowRay));
						}else {
							incidentDot= glm::dot(glm::normalize(triangles[trianglePos].normal),glm::normalize(shadowRay));
						}
						if (incidentDot < 0) incidentDot = 0;
						brightness += pow(incidentDot,2)/1.5;	
						//------specular----------
						glm::vec3 reflection = (-glm::normalize(shadowRay)) - (2.0f * glm::normalize(triangles[trianglePos].normal) * (glm::dot(-glm::normalize(shadowRay),glm::normalize(triangles[trianglePos].normal))));
						glm::vec3 ray = glm::normalize(cameraPos) - glm::normalize(triangles[trianglePos].vertices[vertexPos]);
						float specular = glm::dot(glm::normalize(reflection),glm::normalize(ray));
						if (specular < 0) specular =0;
						if (specular > 1) specular = 1;
						brightness += pow(specular,240);
					}
				}
				brightness = brightness / (lightSources.size());
				brightness += 0.1; //universal suppliment
				if (brightness > 1) brightness =1;
				Colour newColour;
				if (triangles[trianglePos].isTexture) {
					uint32_t pixel = triangles[trianglePos].textureMap.pixels[triangles[trianglePos].textureMap.width * triangles[trianglePos].texturePoints[vertexPos].y + triangles[trianglePos].texturePoints[vertexPos].x];
					newColour = Colour(((pixel>>16) & 255), ((pixel>>8)&255), (pixel&255));
				} else {
					newColour = Colour(triangles[trianglePos].colour.red*brightness,triangles[trianglePos].colour.green*brightness,triangles[trianglePos].colour.blue*brightness);
				}
				triangles[trianglePos].colours[vertexPos] = newColour;
			}
		}
	}
	
	for (float x = 0; x < window.width; x++) {
		//std::cout << "x: " << x << "\n";
		for (float y = 0; y < window.height; y++){
		
			//std::cout << "y: " << y << "\n";
			float newX = (x-(window.width/2)) / (planeScaler* focalL); 
			float newY = (y-window.height/2) / (planeScaler*focalL);
			glm::vec3 ray = glm::vec3(-newX,-newY,-1.0f); //ray from camera to object
			ray = ray * glm::inverse(cameraOrientation);

			RayTriangleIntersection inter = getClosestIntersection(ray,cameraPos,triangles,0,0,0,phong);
			//std::cout << "made it out first ray trace\n";
			if (inter.distanceFromCamera < 1000){
				
				float brightness =0;
				for (glm::vec3 lightSource : lightSources) {
					glm::vec3 shadowRay = (lightSource - inter.intersectionPoint); // / rayInvScalar;
					RayTriangleIntersection shadowInter = getClosestIntersection(shadowRay,inter.intersectionPoint,triangles,1,0,0,1);
					if (shadowInter.distanceFromCamera > 1) {
						if (phong) {
							//------proximity---------
							brightness += 1 / (10* ( pow(glm::length(shadowRay),2))); 
							//----angle of incidence--
							float incidentDot= glm::dot(glm::normalize(inter.normal),glm::normalize(shadowRay));
							//float incidentDot = glm::dot(glm::normalize(inter.intersectedTriangle.normal),-glm::normalize(shadowRay)) ;
							if (incidentDot < 0) incidentDot = 0;
							brightness += pow(incidentDot,2)/1.5;
							
							//------specular----------
							glm::vec3 reflection = (-glm::normalize(shadowRay)) - (2.0f * glm::normalize(inter.normal) * (glm::dot(-glm::normalize(shadowRay),glm::normalize(inter.normal))));
							//glm::vec3 reflection = (-shadowRay) - (2.0f * inter.intersectedTriangle.normal * (glm::dot(-shadowRay,inter.intersectedTriangle.normal)));
							float specular = glm::dot(glm::normalize(reflection),glm::normalize(-ray));
							if (specular < 0) specular =0;
							brightness += pow(specular,240);
						} else brightness += 1;
					}
				}
				brightness = brightness / lightSources.size();
				brightness += 0.1; //universal suppliment
				if (brightness > 1) brightness =1;

				//std::cout << brightness << "\n";
				Colour colour;
				colour = inter.colour;
				
				uint32_t colour_32 = (255 << 24) + (int(round(colour.red * brightness)) << 16) + (int(round(colour.green * brightness)) << 8) + int(round(colour.blue * brightness));
				window.setPixelColour(x,y,colour_32);
			
				
			} 
			//std::cout << "made it end of drawRayTrace trace\n";
		}
	}
}

void drawWireframeScene(DrawingWindow &window,std::vector<ModelTriangle> &triangles, glm::vec3 &cameraPos, glm::mat3 &cameraOrientation ) {
	const float focalL = 2;
	const float planeScaler = HEIGHT/focalL + HEIGHT/3;
	window.clearPixels();
	for (ModelTriangle triangle : triangles) {
		if (triangle.surfaceType == "map") continue;
		CanvasPoint v[3];
		int behind = 0;
		for (size_t i = 0; i <3; i++){
			v[i] = getCanvasIntersectionPoint(window, cameraPos,cameraOrientation,triangle.vertices[i],focalL,planeScaler);
		}
		Colour colour = Colour(255,255,255);
		if (!triangle.isTexture) {
			colour = triangle.colour;
		} 
		drawStrokedTriangle(window,CanvasTriangle(v[0],v[1],v[2]),colour);

	}
}

void drawRasterisedScene(DrawingWindow &window,std::vector<ModelTriangle> &triangles, glm::vec3 &cameraPos, glm::mat3 &cameraOrientation ) {
	std::vector<std::vector<float>> depthBuffer(window.width,std::vector<float>(window.height));
	const float focalL = 2;
	const float planeScaler = HEIGHT/focalL + HEIGHT/3;

	//glm::vec3 lookAtPoint = glm::vec3(0,0,0);
	//orbit(cameraPos,cameraOrientation,lookAtPoint);

	window.clearPixels();
	for (ModelTriangle triangle : triangles) {
		if (triangle.surfaceType == "map") continue;		
		CanvasPoint v[3];
		for (size_t i = 0; i <3; i++){
			v[i] = getCanvasIntersectionPoint(window, cameraPos,cameraOrientation,triangle.vertices[i],focalL,planeScaler);
			if (triangle.isTexture) {
				//std::cout << "texture found in drawRasterisedScene\n";
				v[i].texturePoint = triangle.texturePoints[i] ; //transfer texture point from ModelTriangle array to CanvasPoint 
			}	
		}

		CanvasTriangle tri = CanvasTriangle(v[0],v[1],v[2]);
		if (triangle.isTexture) {
			//std::cout << "texture found in drawRasterisedScene1\n";
			tri.textureMap = triangle.textureMap; //texture map transfered from model triangle to canvas triangle
			tri.isTexture = 1;
		}
		drawFilledTriangle(window,tri,triangle.colour,depthBuffer);
	}
	//drawDepth(window,depthBuffer);
}

void handleEvent(SDL_Event event, DrawingWindow &window,std::vector<ModelTriangle> &triangles,std::vector<glm::vec3> &lightSources, glm::vec3 &cameraPos,glm::mat3 &cameraOrientation, int &renderTypeIndex) {
	const float xStep = 0.0125; //objects coords scaled 0-1
	const float yStep = 0.05;
	const float zStep = 0.05;
	const float lightIncrement = 0.25;
	const float theta = M_PI/36 ; // 9 degree increments
	int sphereIndex;
	int moving = 0;
	
	
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) { 
			std::cout << "LEFT" << std::endl;
			
			if (moving) {
				for (size_t i = 0; i < triangles.size(); i++) 
				{
					if (triangles[i].surfaceType == "move") {
						triangles[i].vertices[0].x -= xStep;
						triangles[i].vertices[1].x -= xStep;
						triangles[i].vertices[2].x -= xStep;
					}
				}
			} else cameraPos.x = cameraPos.x -xStep;

		} else if (event.key.keysym.sym == SDLK_v) {
			for (size_t i = 0; i < triangles.size(); i++) 
			{
				if (triangles[i].surfaceType == "move") {
					triangles[i].vertices[0]  /= 1.1;
					triangles[i].vertices[1] /= 1.1;
					triangles[i].vertices[2] /= 1.1;
				}
			}
		} else if (event.key.keysym.sym == SDLK_b) {
			for (size_t i = 0; i < triangles.size(); i++) 
			{
				if (triangles[i].surfaceType == "move") {
					triangles[i].vertices[0]  *= 1.1;
					triangles[i].vertices[1] *= 1.1;
					triangles[i].vertices[2] *= 1.1;
				}
			}
		} else if (event.key.keysym.sym == SDLK_n) {
			glm::mat3 countClock_rot = glm::mat3(
 			  	cos(theta), 0, -sin(theta), // first column (not row!)
   				0, 1, 0, // second column
   				sin(theta),0, cos(theta)  // third column
			);
			for (size_t i = 0; i < triangles.size(); i++) 
			{
				if (triangles[i].surfaceType == "move") {
					triangles[i].vertices[0] =  triangles[i].vertices[0] * countClock_rot;
					triangles[i].vertices[1] =  triangles[i].vertices[1] * countClock_rot;
					triangles[i].vertices[2] =  triangles[i].vertices[2] * countClock_rot;
				}
			}
		} else if (event.key.keysym.sym == SDLK_m) {
			glm::mat3 clock_rot = glm::mat3(
 			  	1, 0, 0, // first column (not row!)
   				0, cos(-theta), sin(-theta), // second column
   				0, -sin(-theta), cos(-theta)  // third column
			);
			for (size_t i = 0; i < triangles.size(); i++) 
			{
				if (triangles[i].surfaceType == "move") {
					triangles[i].vertices[0] =  triangles[i].vertices[0] * clock_rot;
					triangles[i].vertices[1] =  triangles[i].vertices[1] * clock_rot;
					triangles[i].vertices[2] =  triangles[i].vertices[2] * clock_rot;
				}
			}
		} else if (event.key.keysym.sym == SDLK_RIGHT) {
			if (moving) {
				for (size_t i = 0; i < triangles.size(); i++) 
				{
					if (triangles[i].surfaceType == "move") {
						triangles[i].vertices[0].x += xStep;
						triangles[i].vertices[1].x += xStep;
						triangles[i].vertices[2].x += xStep;
					}
				}
			} else cameraPos.x = cameraPos.x +xStep;
			std::cout << "RIGHT" << std::endl;
		} else if (event.key.keysym.sym == SDLK_UP) {
			if (moving) {
				for (size_t i = 0; i < triangles.size(); i++) 
				{
					if (triangles[i].surfaceType == "move") {
						triangles[i].vertices[0].y -= xStep;
						triangles[i].vertices[1].y -= xStep;
						triangles[i].vertices[2].y -= xStep;
					}
				}
			} else cameraPos.y = cameraPos.y -xStep;
			
		} else if (event.key.keysym.sym == SDLK_DOWN) {
			if (moving) {
				for (size_t i = 0; i < triangles.size(); i++) 
				{
					if (triangles[i].surfaceType == "move") {
						triangles[i].vertices[0].y += xStep;
						triangles[i].vertices[1].y += xStep;
						triangles[i].vertices[2].y += xStep;
					}
				}
			} else cameraPos.y = cameraPos.y +xStep;

			std::cout << "DOWN" << std::endl;
		} else if (event.key.keysym.sym == SDLK_w) {
			if (moving) {
				for (size_t i = 0; i < triangles.size(); i++) 
				{
					if (triangles[i].surfaceType == "move") {
						triangles[i].vertices[0].z -= xStep;
						triangles[i].vertices[1].z -= xStep;
						triangles[i].vertices[2].z -= xStep;
					}
				}
			} else cameraPos.z = cameraPos.z -xStep;
			
		} else if (event.key.keysym.sym == SDLK_e) {
			std::string line;
			std::vector<std::string> sections;
			std::string surfaceType;
			std::ifstream MyReadFile("cornell-box copy.obj");
			std::vector<int> faceOrder;
			int foundVertexIndex =0;
			int initialIndex;
			int vertexCount =0;
			while (getline(MyReadFile, line)) {
				sections = split(line, ' ');
				if (sections.size()>0) {
					if (sections[0] == "v") {
						if (surfaceType == "move") vertexCount++;
					} else if (sections[0] == "vt") {

					} else if (sections[0] == "f" && surfaceType == "move") {

						//std::cout << "f\n";
						std::vector<std::string> sec1 = split(sections[1], '/');
						std::vector<std::string> sec2 = split(sections[2], '/');
						std::vector<std::string> sec3 = split(sections[3], '/');
						if (!foundVertexIndex) {
							initialIndex = (std::min(std::stoi(sec1[0]),std::min(std::stoi(sec2[0]),std::stoi(sec3[0])) ));
							foundVertexIndex =1;
						}
						int v0Pos = std::stoi(sec1[0]) -initialIndex; //subtract index of first vertex of shape
						int v1Pos = std::stoi(sec2[0]) -initialIndex;
						int v2Pos = std::stoi(sec3[0]) -initialIndex;
						faceOrder.push_back(v0Pos);
						faceOrder.push_back(v1Pos);
						faceOrder.push_back(v2Pos);
						

						if (sec1[1] != "") { //if texture points exist
					
						} 
				

					} else if (sections[0] == "usemtl") {
					} else if (sections[0] == "surface") {
						surfaceType = sections[1];
					}
				}
			}
			MyReadFile.close();

			std::vector<glm::vec3> vertices;
			for (ModelTriangle triangle : triangles) {
				if (triangle.surfaceType == "move") {
					for (glm::vec3 vertex : triangle.vertices) {						
						vertices.push_back(vertex);
					}
				} 
			}

			std::vector<glm::vec3> verticesOrdered(vertexCount);
			vertexCount = 0;
			for (int facePos : faceOrder) {
				verticesOrdered[facePos] = vertices[vertexCount];
				vertexCount++;
			}
			std::ofstream myfile("sphere updated.obj");
			if(myfile.is_open())
			{	
				for (glm::vec3 vertex : verticesOrdered) {
					myfile<< "v " <<  -(vertex.x / 0.35) << " " << vertex.y /0.35 << " " << vertex.z /0.35 << std::endl;
				}
				
				myfile.close();
			}
			//for (int vertex)
			
		
		} else if (event.key.keysym.sym == SDLK_s) {
			//cameraPos.z = cameraPos.z + zStep;
			if (moving) {
				for (size_t i = 0; i < triangles.size(); i++) 
				{
					if (triangles[i].surfaceType == "move") {
						triangles[i].vertices[0].z += xStep;
						triangles[i].vertices[1].z += xStep;
						triangles[i].vertices[2].z += xStep;
					}
				}
			} else cameraPos.z = cameraPos.z +xStep;
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
			//cameraOrientation = cameraOrientation * countClock_rot;
			std::cout << "cameraPosBefore " << cameraPos.x << "," << cameraPos.y << "," << cameraPos.z << "\n";
			cameraPos = cameraPos * countClock_rot;
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
			for (size_t i = 0; i < 3; i++){
				for (size_t j = 0; j < 3; j++){
					std::cout << cameraOrientation[i][j] << ",";
				}
				std::cout<<"\n";
			}
			std::cout << "ROTATE Y COUNTER-CLOCKWISE" << std::endl;
		} else if (event.key.keysym.sym == SDLK_r) {

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

		}  else if (event.key.keysym.sym == SDLK_j) {
			for (size_t i = 0; i < lightSources.size(); i++) {
				lightSources[i].x += lightIncrement;
			}
		}  else if (event.key.keysym.sym == SDLK_l) {
			for (size_t i = 0; i < lightSources.size(); i++) {
				lightSources[i].x -= lightIncrement;
			}
		}  else if (event.key.keysym.sym == SDLK_i) {
			for (size_t i = 0; i < lightSources.size(); i++) {
				lightSources[i].z -= lightIncrement;
			}
		}  else if (event.key.keysym.sym == SDLK_k) {
			for (size_t i = 0; i < lightSources.size(); i++) {
				lightSources[i].z += lightIncrement;
			}
		}  else if (event.key.keysym.sym == SDLK_0) {
			for (size_t i = 0; i < lightSources.size(); i++) {
				lightSources[i].y += lightIncrement;
			}
		}  else if (event.key.keysym.sym == SDLK_o) {
			for (size_t i = 0; i < lightSources.size(); i++) {
				lightSources[i].y -= lightIncrement;
			}
		} else if (event.type == SDL_MOUSEBUTTONDOWN) {
			window.savePPM("output.ppm");
			window.saveBMP("output.bmp"); 
		} 
	}

}

void saveCamera(std::string filename, glm::vec3 cameraPos, glm::mat3 cameraOrientation){
	std::ofstream myfile(filename);
    if(myfile.is_open())
    {	
		myfile<< "o ";
		for (size_t y = 0; y < 3; y++){
			for (size_t x = 0; x < 3; x++){
				myfile << cameraOrientation[x][y]  << " ";
			}
		}
		myfile<< std::endl;
		myfile << "c " << cameraPos.x << " " << cameraPos.y << " " << cameraPos.z;
	}
        myfile.close(); 
}

void loadCamera(std::string filename,glm::vec3 &cameraPos, glm::mat3 &cameraOrientation){
	std::ifstream MyReadFileMtl(filename);
	std::vector<std::string> sections;
	std::string line;
	Colour colour;
	while (getline (MyReadFileMtl, line)) {
		sections = split(line, ' ');
		if (sections.size()>0) {
			if (sections[0] == "o") {
				int count = 1;
				for (size_t y = 0; y < 3; y++){
					for (size_t x = 0; x < 3; x++){
						cameraOrientation[x][y] = stof(sections[count]);
						count++;
					}
				}
			} else if (sections[0] == "c") {
				cameraPos.x = stof(sections[1]);
				cameraPos.y = stof(sections[2]);
				cameraPos.z = stof(sections[3]);
 			}
		}
		sections.clear();
	}
	MyReadFileMtl.close();
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
	std::string cameraFilename = "camera";
	const float objScaler = 0.35;
	
	/* std::vector<ModelTriangle> triangles = loadObj("sphere.obj","sphere.mtl",objScaler); 
	glm::vec3 cameraPos = glm::vec3(0,0.5,2); //position for sphere
	glm::vec3 lightSource = glm::vec3(0,0.5,2); //sphere light location  */

	//std::vector<ModelTriangle> triangles = loadObj("cornell-box-texture.obj","cornell-box-texture.mtl",objScaler);
	std::vector<ModelTriangle> triangles = loadObj("cornell-box copy.obj","cornell-box.mtl",objScaler);
	//std::vector<ModelTriangle> triangles = loadObj("triangle-texture.obj","triangle-texture.mtl",objScaler);

	glm::vec3 cameraPos = glm::vec3(0,0,5); //position for box
	glm::vec3 lightSource = glm::vec3(0,0.75,0); //box light location

	std::vector<glm::vec3> lightSources;
	lightSources.push_back(lightSource);
	float lightIncrement =  0.05;
	for (int x = -2; x < 3; x++) {
		for (int z = -2; z < 3; z++) {
			lightSources.push_back(glm::vec3(lightSource[0] + (x * lightIncrement), lightSource[1], lightSource[2] + (z * lightIncrement)));
		}
	}   

	glm::mat3 camOrientation = glm::mat3(
		//									   | Right | Up  | Forward |
		1, 0, 0, // first column (not row!)  x |   1   ,  0  ,    0    |
		0, 1, 0, // second column		     y |   0   ,  1  ,    0    |
		0, 0, 1  // third column			 z |   0   ,  0  ,    1    |
	);

	saveCamera(cameraFilename,cameraPos,camOrientation);
	loadCamera(cameraFilename, cameraPos, camOrientation);
	std::cout << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << "\n";
 	for (size_t i = 0; i < 3; i++){
		for (size_t j = 0; j < 3; j++){
			std::cout << camOrientation[i][j] << ",";
		}
		std::cout<<"\n";
	}
	int renderTypeIndex = 0;

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window,triangles,lightSources ,cameraPos, camOrientation,renderTypeIndex);

		if (renderTypeIndex == 0) {		
			drawWireframeScene(window,triangles,cameraPos,camOrientation);
			/* for (int i = 0; i < 61; i++){
				drawWireframeScene(window,triangles,cameraPos,camOrientation);
				std::string filename = "";
				int zeros = 5- int(std::to_string(i).size());

				for (int i = 0; i < zeros;i++) filename+="0"; 
				filename += std::to_string(i);
				window.savePPM("video/" + filename + ".ppm");
				const float theta = M_PI/30 ; // 9 degree increments
				glm::mat3 clock_rot = glm::mat3(
					cos(-theta), 0, -sin(-theta), // first column (not row!)
					0, 1, 0, // second column
					sin(-theta),0, cos(-theta)  // third column
				);
				cameraPos = cameraPos * clock_rot;
				lookAt(camOrientation,cameraPos, glm::vec3(0,0,0));
				window.renderFrame();
			}
			int x;
			std::cin >> x;  */
		} else if(renderTypeIndex == 1) {
			drawRasterisedScene(window,triangles,cameraPos,camOrientation);
			/*  for (int i = 61; i < 122; i++){
				drawRasterisedScene(window,triangles,cameraPos,camOrientation);
				std::string filename = "";
				int zeros = 5- int(std::to_string(i).size());

				for (int i = 0; i < zeros;i++) filename+="0"; 
				filename += std::to_string(i);
				window.savePPM("video/" + filename + ".ppm");
				const float theta = M_PI/30 ; // 9 degree increments
				glm::mat3 clock_rot = glm::mat3(
					cos(-theta), 0, -sin(-theta), // first column (not row!)
					0, 1, 0, // second column
					sin(-theta),0, cos(-theta)  // third column
				);
				cameraPos = cameraPos * clock_rot;
				lookAt(camOrientation,cameraPos, glm::vec3(0,0,0));
				window.renderFrame();
			}
			int x;
			std::cin >> x;  */
			
		} else if (renderTypeIndex == 2) {
			drawRayTrace(window,triangles,lightSources,cameraPos,camOrientation,0);
		 	/* for (int i = 0; i < 61; i++){
				drawRayTrace(window,triangles,lightSources,cameraPos,camOrientation,0);

				std::string filename = "";
				int zeros = 5- int(std::to_string(i).size());
				for (int i = 0; i < zeros;i++) filename+="0"; 
				filename += std::to_string(i);
				window.savePPM("video/" + filename + ".ppm");
				saveCamera(cameraFilename,cameraPos,camOrientation);

				//const float theta = M_PI/30 ; 
				float xInc = 0.0166667;
				float zInc = 0.0333333;
				cameraPos.z -=zInc;
				cameraPos.x += xInc;
				lookAt(camOrientation,cameraPos, glm::vec3(0,0,0));
				window.renderFrame();
			}
			int x;
			std::cin >> x;    */

		}

		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
		
	}
}
