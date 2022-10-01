#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <glm/glm.hpp>
#include <Utils.h>
#include <fstream>
#include <vector>

#define WIDTH 320
#define HEIGHT 240

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, float numberOfValues) {
	std::vector<glm::vec3> fade;

	float value[3];

	float step = (to[0]- from[0]) / (numberOfValues-1);
	float step1 = (to[1]- from[1]) / (numberOfValues-1);
	float step2 = (to[2]- from[2]) / (numberOfValues-1);
	
	int pos = 0;
	std::cout<< "made it"  << "\n"; 
	while(pos < numberOfValues) {
		glm::vec3 vec(from[0]+pos*step,from[1]+pos*step1,from[2]+pos*step2);
		fade.push_back(vec);
		pos++;
	}

	return fade; 
}

void draw(DrawingWindow &window) {
	window.clearPixels();
	glm::vec3 from(255,0,0);
	glm::vec3 to(0,255,0);

	std::vector<glm::vec3> scale = interpolateThreeElementValues(from,to,window.width);

	for (size_t y = 0; y < window.height; y++) {
		for (size_t x = 0; x < window.width; x++) {
			float red = scale.at(x)[0];
			float green = scale.at(x)[1];
			float blue = scale.at(x)[2];
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x, y, colour);
		}
	}
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}


int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
	std::vector<glm::vec3> result;
	glm::vec3 from(1.0, 4.0, 9.2);
	glm::vec3 to(4.0, 1.0, 9.8);
	result = interpolateThreeElementValues(from, to, 4);
	std::cout << "Make it past function" << "\n";
	for (int i =0 ; i< 4;i++){
		for (int x=0;x<3;x++){
			std::cout << result.at(i)[x] << ',';
		}
		std::cout << "\n";
	}
	
	
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		//draw(window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
