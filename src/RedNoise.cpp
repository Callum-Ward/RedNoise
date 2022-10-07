#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <Colour.h>
#include <glm/glm.hpp>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <cmath>
#include <math.h> 
#include <stdlib.h>

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
	for (float i = 0; i < numberOfSteps; i++)
	{
		float x = from.x + xStepSize*i;
		float y = from.y + yStepSize*i;
		window.setPixelColour(round(x),round(y), colour_32);
	}

}

void drawStrokedTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {

	//window.clearPixels();
	drawLine(window,triangle.v0(),triangle.v1(),colour);
	drawLine(window,triangle.v1(),triangle.v2(),colour);
	drawLine(window,triangle.v2(),triangle.v0(),colour);
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
		}
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
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
