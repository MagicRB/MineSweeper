/*
 * main.cpp
 *
 *  Created on: Dec 30, 2017
 *      Author: Magic_RB
 */

#include <iostream>
#include <vector>
#include <experimental/random>
#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>

#include "bmpread.h"


struct vertex {
	glm::vec2 pos, uv;
};

struct bmp {
	unsigned char* pixels;
	GLuint width;
	GLuint height;
};

struct tile {
	bool bomb = false;
	bool revealed = true ;
	bool flag = false;
	short unsigned int number = 0;
	GLuint vertexbuffer;
	GLuint uvbuffer;
};

tile safe_tile_at(std::vector<std::vector<tile>> vector, short unsigned int x, short unsigned int y)
{
	try {
		return vector.at(x).at(y);
	} catch(const std::exception& e) {
		return tile();
	}
}

void delete_vbo_for_tiles(std::vector<std::vector<tile>>* mine_field)
{
	for (short unsigned int x = 0; x < mine_field->size(); x++) {
		for (short unsigned int y = 0; y < mine_field->at(x).size(); y++) {
			glDeleteBuffers(1, &mine_field->at(x).at(y).vertexbuffer);
		}
	}
}

void clear_mine_field(std::vector<std::vector<tile>>* mine_field)
{
	delete_vbo_for_tiles(mine_field);
	for (short unsigned int x = 0; x < mine_field->size(); x++) {
		for (short unsigned int y = 0; y < mine_field->at(x).size(); y++) {
			mine_field->at(x).at(y) = tile();
		}
	}
}

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open()){
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	
	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

void populate_mine_field(std::vector<std::vector<tile>>* mine_field, short unsigned int number_of_bombs, short unsigned int number_of_collums, short unsigned int number_of_rows)
{
	clear_mine_field(mine_field);

	mine_field->resize(number_of_collums, std::vector<tile>(number_of_rows, tile()));

	for (short unsigned int i = 0; i < number_of_bombs; i++) {
		int x = std::experimental::randint(0, number_of_collums - 1);
	 	int y = std::experimental::randint(0, number_of_rows - 1);
	 	while (mine_field->at(x).at(y).bomb == true) {
			x = std::experimental::randint(0, number_of_collums - 1);
	 		y = std::experimental::randint(0, number_of_rows - 1);
		}
		mine_field->at(x).at(y).bomb = true;		
	}

	for (short unsigned int x = 0; x < mine_field->size(); x++) {
		for (short unsigned int y = 0; y < mine_field->at(x).size(); y++) {
			short unsigned int b = 0;
			if (safe_tile_at(*mine_field, x - 1, y - 1).bomb == true) { b++; }
			if (safe_tile_at(*mine_field, x    , y - 1).bomb == true) { b++; } 
			if (safe_tile_at(*mine_field, x + 1, y - 1).bomb == true) { b++; }
			if (safe_tile_at(*mine_field, x - 1, y    ).bomb == true) { b++; }
			if (safe_tile_at(*mine_field, x + 1, y    ).bomb == true) { b++; }
			if (safe_tile_at(*mine_field, x - 1, y + 1).bomb == true) { b++; }
			if (safe_tile_at(*mine_field, x    , y + 1).bomb == true) { b++; }
			if (safe_tile_at(*mine_field, x + 1, y + 1).bomb == true) { b++; }
			mine_field->at(x).at(y).number = b;
		}
	}
}

void reveal_mine_field(std::vector<std::vector<tile>>* mine_field)
{
	for (short unsigned int x = 0; x < mine_field->size(); x++) {
		for (short unsigned int y = 0; y < mine_field->at(x).size(); y++) {
			mine_field->at(x).at(y).revealed = true;
		}
	}
}

std::vector<std::vector<tile>> tile_matrix;

short unsigned int number_of_bombs = 15;
short unsigned int number_of_rows = 10;
short unsigned int number_of_collums = 10;

short unsigned int revealed_tiles = 0;

// std::vector<vertex> tile_vector = {
// 	{ {  0,   0 }, { 0,   0 } },
// 	{ { 32,   0 }, { 1,   0 } },
// 	{ {  0,  32 }, { 0,   1 } },
// 	{ {  0,  32 }, { 0,   1 } },
// 	{ { 32,   0 }, { 1,   0 } },
// 	{ { 32,  32 }, { 1,   1 } }
// };

std::vector<GLfloat> tile_vector = {
	 0,   0,
	32,   0,
	 0,  32,
	 0,  32,
	32,   0,
	32,  32
};

std::vector<GLfloat> tile_vector_uv = {
	 0,   0,
	 1,   0,
	 0,   1,
	 0,   1,
	 1,   0,
	 1,   1
};

std::vector<GLfloat> translate_vector(const std::vector<GLfloat> tile_v, float x, float y)
{
	std::vector<GLfloat> tile_nv;
	for (auto i = 0; i < tile_v.size(); i) {
		tile_nv.push_back(tile_v.at(i) + x);
		i++;
		tile_nv.push_back(tile_v.at(i) + y);
		i++;
	}
	return tile_nv;
}
void generate_vbo_for_tiles(std::vector<std::vector<tile>>* mine_field)
{
	for (short unsigned int x = 0; x < mine_field->size(); x++) {
		for (short unsigned int y = 0; y < mine_field->at(x).size(); y++) {
			glGenBuffers(1, &mine_field->at(x)[y].vertexbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, mine_field->at(x)[y].vertexbuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * translate_vector(tile_vector, x*32, y*32).size(), translate_vector(tile_vector, x*32, y*32).data(), GL_STATIC_DRAW);
			glGenBuffers(1, &mine_field->at(x)[y].uvbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, mine_field->at(x)[y].uvbuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * tile_vector_uv.size(), tile_vector_uv.data(), GL_STATIC_DRAW);
		}
	}
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS && key == GLFW_KEY_Q) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	} else if (action == GLFW_PRESS && key == GLFW_KEY_R) {
		populate_mine_field(&tile_matrix, number_of_bombs, number_of_collums, number_of_rows);
		generate_vbo_for_tiles(&tile_matrix);
	} else if (action == GLFW_PRESS && key == GLFW_KEY_X) {
		reveal_mine_field(&tile_matrix);
	}
}

inline float layer2coord(int capacity, int layer)
{
    return std::max(0.0f, std::min(float(capacity - 1), std::floor(float(layer) + 0.5f)));
}

int main()
{
	GLuint VertexArrayID;
	

	if (!glfwInit())
	{
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(32*number_of_collums, 32*number_of_rows, "Mine Sweeper", NULL, NULL);
	if (!window) {
		return -1;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = true;

	if (glewInit() != GLEW_OK) {
		return -1;
	}

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	populate_mine_field(&tile_matrix, number_of_bombs, number_of_collums, number_of_rows);

	generate_vbo_for_tiles(&tile_matrix);

	glfwSetKeyCallback(window, key_callback);

	glm::mat4 projectionMatrix = glm::ortho(0.0f, (float)32*number_of_collums,(float)32*number_of_rows,0.0f, -0.1f, 100.0f); // glm::ortho(0.0f, (float)1024/*32*number_of_collums*/, 0.0f, (float)768/*32*number_of_rows*/, -1.0f, 1.0f);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader" );
	glUseProgram(programID);

	glActiveTexture(GL_TEXTURE0 + 5);

	GLuint texture_array;
	glGenTextures(1, &texture_array);

	glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array);
	// glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, 32, 32, 12, GL_FALSE, GL_RGB, GL_UNSIGNED_BYTE, nullptr);


	glTexImage3D(GL_TEXTURE_2D_ARRAY, 5, GL_RGB, 32, 32, 12, GL_FALSE, GL_RGB, GL_UNSIGNED_BYTE, nullptr);


	for (short unsigned int i = 0; i < 12; i++) {
		bmpread_t bitmap;
		if(!bmpread((std::string("Textures/") + std::to_string(i) + std::string(".bmp")).c_str(), 0, &bitmap))
		{
			std::cout << "Could not open texture" << (std::string("Textures/") + std::to_string(i) + std::string(".bmp")) << std::endl;
		}

		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, 32, 32, 1, GL_RGB, GL_UNSIGNED_BYTE, bitmap.rgb_data);
		bmpread_free(&bitmap);
	}

	GLint projectionLoc = glGetUniformLocation(programID, "projectionMatrix" );
	GLuint texLoc = glGetUniformLocation(programID, "tex");
	GLint layerLoc = glGetUniformLocation(programID, "layer");
	GLint posLoc = glGetAttribLocation(programID, "vertexPosition_modelspace");
	GLint uvLoc = glGetAttribLocation(programID, "vertexUV");

	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	glUniform1i(texLoc, 5);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT);

		for (short unsigned int x = 0; x < tile_matrix.size(); x++) {
			for (short unsigned int y = 0; y < tile_matrix.at(x).size(); y++) {
				if (tile_matrix[x][y].flag && !tile_matrix[x][y].revealed) {
					// Set texture to flag
					
				} else if (tile_matrix[x][y].bomb && tile_matrix[x][y].revealed) {
					
					glActiveTexture(GL_TEXTURE0 + 5);
					glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array);

					glUniform1f(layerLoc, layer2coord(12, 1));

					glEnableVertexAttribArray(posLoc);
					glBindBuffer(GL_ARRAY_BUFFER, tile_matrix[x][y].vertexbuffer);
					glVertexAttribPointer(
					0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
					2,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
					);

					glBindBuffer(GL_ARRAY_BUFFER, tile_matrix[x][y].uvbuffer);
					glEnableVertexAttribArray(uvLoc);
					glVertexAttribPointer(
					1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
					2,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0           // array buffer offset
					);

					glDrawArrays(GL_TRIANGLES, 0, 6); 

					//Draw the triangle !
					//Starting from vertex 0; 3 vertices total -> 1 triangle
					glDisableVertexAttribArray(uvLoc);
					glDisableVertexAttribArray(posLoc);
				} else if (tile_matrix[x][y].revealed) {
					if (tile_matrix[x][y].number == 0) {

					} else if (tile_matrix[x][y].number == 1) {

					} else if (tile_matrix[x][y].number == 2) {

					} else if (tile_matrix[x][y].number == 3) {

					} else if (tile_matrix[x][y].number == 4) {

					} else if (tile_matrix[x][y].number == 5) {

					} else if (tile_matrix[x][y].number == 6) {

					} else if (tile_matrix[x][y].number == 7) {
					
					} else if (tile_matrix[x][y].number == 8) {

					}
				} else {

				}
			}
		}

		glfwSwapBuffers(window);
	}

	delete_vbo_for_tiles(&tile_matrix);

	glfwDestroyWindow(window);

	glfwTerminate();
}



	// else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
	// 	if (left_mouse_pressed == false) {
	// 		if (tile_matrix[event.mouseButton.x / 32][event.mouseButton.y  / 32].revealed == false) {
	// 			tile_matrix[event.mouseButton.x / 32][event.mouseButton.y  / 32].revealed = true;
	// 			revealed_tiles++;
	// 			text_revealed_tiles.setString(std::to_string(revealed_tiles));
	// 		}
	// 		left_mouse_pressed = true;

	// 		if (tile_matrix[event.mouseButton.x / 32][event.mouseButton.y  / 32].bomb == true) reveal_mine_field(&tile_matrix);
	// 	}
	// } else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
	// 	left_mouse_pressed = false;
	// } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
	// 	if (right_mouse_pressed == false) {
	// 		tile_matrix[event.mouseButton.x / 32][event.mouseButton.y  / 32].flag = !tile_matrix[event.mouseButton.x / 32][event.mouseButton.y  / 32].flag;
	// 		right_mouse_pressed = true;
	// 	}
	// } else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Right) {
	// 	right_mouse_pressed = false;
	// }