// Std. Includes
#include <string>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include <SOIL.h>

// Windows Libirary (console movement)
#include <windows.h>

// OpenCV (homography)
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

// Properties
GLuint screen_width = GetSystemMetrics(SM_CXSCREEN);
GLuint screen_height = GetSystemMetrics(SM_CYSCREEN);
GLuint WIDTH = 800, HEIGHT = 800;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mousemove_callback(GLFWwindow* window, double xpos, double ypos);
void mouseclick_callback(GLFWwindow* window, int button, int action, int mods);
void Do_Movement();

// Self-defined Function Prototype
void set_model_coord(GLint xpos, GLint ypos, double depth, int index);
void set_texture_coord(GLint xpos, GLint ypos, int index);
void draw_model();

// Coordination
glm::vec4 vertex_color[4];
glm::vec2 model_2D_coord[4];
glm::vec2 texture_2D_coord[4];
glm::mat3 homography_matrix;
int point_selection_index;

// Camera
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;
int current_camera_pos = 0;
glm::vec3 model_correction;
glm::vec3 model_rotate;
glm::vec3 model_translation;

Camera camera(glm::vec3(0.0f, 0.0f, 2000000.0f)); // invalid value, will be replaced in viewSwitch..

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Global pointer
Model* pModel = NULL;
Shader* pShader = NULL;

// Window
GLFWwindow* objwindow;
GLFWwindow* texwindow;

// Filename
string imageFilePath;
// Load and create a texture 
GLuint texture;
// Load OpenCV Image
cv::Mat cvimage;

// The MAIN function, from here we start our application and run our Game loop
int main()
{
	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	HWND console = GetConsoleWindow();
	MoveWindow(console, 0, 0, screen_width, screen_height / 3.5 + 20, true);
	/*=============================================================*/
	/*==================Create Obj Model Window====================*/
	objwindow = glfwCreateWindow(WIDTH, HEIGHT, "OBJ Model", nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(objwindow);
	glfwSetWindowPos(objwindow, (0 + screen_width / 2) / 2 - WIDTH / 2, 200);

	// Set the required callback functions
	glfwSetKeyCallback(objwindow, key_callback);
	glfwSetCursorPosCallback(objwindow, mousemove_callback);
	glfwSetScrollCallback(objwindow, scroll_callback);
	glfwSetMouseButtonCallback(objwindow, mouseclick_callback);

	// Options
	glfwSetInputMode(objwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);

	// Setup and compile our shaders
	Shader shader("obj.vs", "obj.frag");

	// Load models
	Model ourModel("test2.obj"); // nanosuit/nanosuit.obj

	// Model matrix generation
	GLfloat max_bounding_value[6];
	for (int i = 0; i < 6; i++)
	{
		max_bounding_value[i] = ourModel.getMaxBoundingValue(i);
		cout << max_bounding_value[i] << endl;
	}
	
	model_correction.x = -(max_bounding_value[1] + max_bounding_value[3]) / 2;
	model_correction.y = -(max_bounding_value[4] + max_bounding_value[5]) / 2;
	model_correction.z = -(max_bounding_value[0] + max_bounding_value[2]) / 2;
	cout << model_correction.x << " " << model_correction.y << " " << model_correction.z << endl;
	camera.SetPosition(glm::vec3(0.0f, 0.0f, 20.0f));//-model_translation.z + max_bounding_value[2]));
	//camera.SetPosition(glm::vec3(0.0f, 2000.0f, 100000.0f));

	model_translation.x = 0.0f;
	model_translation.y = 0.0f;//-fabs(max_bounding_value[4] - max_bounding_value[5]);
	model_translation.z = 0.0f;

	pModel = &ourModel;
	pShader = &shader;

	/*==================Create Obj Model Window====================*/
	/*=============================================================*/
	

	/*=============================================================*/
	/*===================Create Texture Window=====================*/

	// Create a GLFWwindow object that we can use for GLFW's functions
	texwindow = glfwCreateWindow(WIDTH, HEIGHT, "Texture", nullptr, nullptr);
	glfwMakeContextCurrent(texwindow);
	glfwSetWindowPos(texwindow, (screen_width / 2 + screen_width) / 2 - WIDTH / 2, 200);

	// Set the required callback functions
	glfwSetKeyCallback(texwindow, key_callback);
	glfwSetMouseButtonCallback(texwindow, mouseclick_callback);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);


	// Build and compile our shader program
	Shader ourShader("tex.vs", "tex.frag");
	

	// Set up vertex data (and buffer(s)) and attribute pointers
	GLfloat vertices[] = {
		// Positions          // Colors           // Texture Coords
		1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Top Right
		1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Bottom Right
		-1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Bottom Left
		-1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // Top Left 
	};
	GLuint indices[] = {  // Note that we start from 0!
		0, 1, 3, // First Triangle
		1, 2, 3  // Second Triangle
	};
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// TexCoord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO

	glGenTextures(1, &texture);


	/*===================Create Texture Window=====================*/
	/*=============================================================*/

	// Camera Init
	//current_camera_pos = 0;
	//camera.ViewSwitch(0);
	

	// Game loop
	while (!glfwWindowShouldClose(objwindow) && !glfwWindowShouldClose(texwindow))
	{
		glfwMakeContextCurrent(objwindow);
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events
		glfwPollEvents();
		Do_Movement();

		draw_model();

		glfwMakeContextCurrent(texwindow);
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		//glfwPollEvents();

		// Render
		// Clear the colorbuffer
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Activate shader
		ourShader.Use();

		// Bind Textures using texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 0);

		// Draw container
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Swap the screen buffers
		glfwSwapBuffers(texwindow);
	}
	// Properly de-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	glfwTerminate();
	return 0;
}


void set_model_coord(GLint xpos, GLint ypos, double depth, int index)
{
	model_2D_coord[index] = glm::vec2((double)xpos * 4000.0f / WIDTH, (double)ypos * 4000.0f / HEIGHT); //TODO: Magic Number??
	//glReadPixels(xpos, HEIGHT - ypos, 1, 1, GL_RGBA, GL_FLOAT, &vertex_color[index]);
	cout << model_2D_coord[index].x << " " << model_2D_coord[index].y << endl;
	//cout << "depth = " << vertex_color[index].r << endl;
}

void set_texture_coord(GLint xpos, GLint ypos, int index)
{
	int w, h;
	glfwGetWindowSize(texwindow, &w, &h);
	texture_2D_coord[index] = glm::vec2((double)xpos * 4000.0f / w, (double)ypos * 4000.0f / h); //TODO: Magic Number??
	cout << texture_2D_coord[index].x << " " << texture_2D_coord[index].y << endl;
}

//  Texture_2D ~ H * model_2D <- input
void calc_homography_matrix()
{
	vector<cv::Point2f> pts_src;
	vector<cv::Point2f> pts_dst;
	for (int i = 0; i < 4; ++i)
	{
		pts_src.push_back(cv::Point2f((float)model_2D_coord[i].x, (float)model_2D_coord[i].y));
		pts_dst.push_back(cv::Point2f((float)texture_2D_coord[i].x, (float)texture_2D_coord[i].y));
	}
	cv::Mat m_homography = findHomography(pts_src, pts_dst); // 4x4
	cv::Mat wMat;
	cv::warpPerspective(cvimage, wMat, m_homography, cv::Size(cvimage.cols, cvimage.rows),
		cv::InterpolationFlags::INTER_NEAREST | cv::InterpolationFlags::WARP_INVERSE_MAP);

	string inputImageFileName = imageFilePath.substr(imageFilePath.find_last_of('/') + 1);
	string outputImageFilename = "warpedImage_" + inputImageFileName + ".png";

	cv::imwrite(outputImageFilename, wMat);
	//cv::imshow("aaa", cvimage);
	cv::imshow("Homography", wMat);
	cout << m_homography << endl;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			homography_matrix[i][j] = m_homography.at<double>(j, i);
}

void print_view_matrix()
{
	glm::mat4 model;
	model = glm::translate(model, model_correction);
	model = glm::rotate(model, glm::radians(model_rotate.x), glm::vec3(1.0, 0.0, 0.0));
	model = glm::rotate(model, glm::radians(model_rotate.y), glm::vec3(0.0, 1.0, 0.0));
	model = glm::rotate(model, glm::radians(model_rotate.z), glm::vec3(0.0, 0.0, 1.0));
	model = glm::translate(model, model_translation);
	glm::mat4 view = camera.GetViewMatrix() * model;
	glm::mat3 R = glm::mat3(view);
	glm::vec3 T = glm::vec3(view[3]);

	glm::mat4 result = glm::transpose(R);
	result[3] = glm::vec4(-glm::transpose(R)*T, 1.0f);

	//glm::mat4 inv = glm::inverse(view);
	string inputImageFileName = imageFilePath.substr(imageFilePath.find_last_of('/') + 1);
	string outputImageFilename = "warpedImage_" + inputImageFileName + ".png";
	string viewMatrixfileName = "viewMatrix_" + outputImageFilename + ".txt";

	fstream pfile(viewMatrixfileName, ios::out);
	pfile << outputImageFilename << endl;
	pfile << imageFilePath << endl;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			pfile << result[j][i] << " ";
	pfile << endl;
	pfile.close();
	cout << viewMatrixfileName << endl;
	//cout << R << endl;
	//cout << T << endl;
	//cout << view << endl;
}

void load_texture(string filename, int* w, int* h) {
	glfwMakeContextCurrent(texwindow);
	glBindTexture(GL_TEXTURE_2D, texture); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
										   // Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Load, create texture and generate mipmaps
	int width, height;
	cvimage = cv::imread(filename);
	//cv::imshow("check", cvimage);
	unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
	*w = width;
	*h = height;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
}

#pragma region "User input"

// Moves/alters the camera positions based on user input
void Do_Movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (keys[GLFW_KEY_LEFT_CONTROL])
		camera.ProcessKeyboard(BOTTOM, deltaTime);
	if (keys[GLFW_KEY_SPACE])
		camera.ProcessKeyboard(UP, deltaTime);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;

	if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS)
		glfwSetInputMode(objwindow, GLFW_CURSOR, (glfwGetInputMode(objwindow, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)? GLFW_CURSOR_NORMAL:GLFW_CURSOR_DISABLED);

	if (key == GLFW_KEY_INSERT && action == GLFW_PRESS)
	{
		int width, height;
		imageFilePath = "ttttt.jpg";
		load_texture(imageFilePath, &width, &height);
		//float ratio = (float)width / (float)height;
		//glfwSetWindowSize(texwindow, WIDTH, WIDTH / ratio);
		//glViewport(0, 0, WIDTH, WIDTH / ratio);
	}
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
	{
		model_rotate.y += 45;
	}
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
	{
		model_rotate.y -= 45;
	}
	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
	{
		model_rotate.x += 45;
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	{
		model_rotate.x -= 45;
	}
	if (key == GLFW_KEY_PAGE_UP && action == GLFW_PRESS)
	{
		model_rotate.z += 45;
	}
	if (key == GLFW_KEY_PAGE_DOWN && action == GLFW_PRESS)
	{
		model_rotate.z -= 45;
	}
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
	{
		cout << "[Info] Calculating texCoord..." << endl;
		//double avg_depth = vertex_color[0].z + vertex_color[1].z + vertex_color[2].z + vertex_color[3].z;
		//avg_depth *= 0.25;

		//glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 1000000000.0f);
		//glm::mat4 view = camera.GetViewMatrix();
		//glm::mat4 model;
		//model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		calc_homography_matrix();
		print_view_matrix();
		//pModel->updateNode(avg_depth, glm::mat4(glm::mat3(view)) * model, projection * view * model, homography_matrix, current_camera_pos);
		cout << "[Info] Calculating finished." << endl;
	}
}

void mousemove_callback(GLFWwindow* window, double xpos, double ypos)
{	
	//Move camera direction
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	if (glfwGetInputMode(objwindow, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
		camera.ProcessMouseMovement(xoffset, yoffset);
}

void mouseclick_callback(GLFWwindow* window, int button, int action, int mods) //GLFW_RELEASE
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	//std::cout << xpos << " " << ypos << std::endl;

	if (window == objwindow)  // even number: 0, 2, 4, 6
	{
		if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS) //debug only
		{
			unsigned char* pixel = new unsigned char[3 * 800 * 600];
			glReadPixels(0, 0, 800, 600, GL_RGB, GL_UNSIGNED_BYTE, pixel);
			SOIL_save_image("check.bmp", SOIL_SAVE_TYPE_BMP, 800, 600, 3, pixel);
			delete[] pixel;
		}
		if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
		{
			GLfloat pixel[3];
			glReadPixels((GLint)xpos, HEIGHT - (GLint)ypos, 1, 1, GL_RGB, GL_FLOAT, &pixel);
			
			GLfloat depth = pixel[0];
			if (depth == 0)
			{
				cout << "[Error] " << "Infinite depth!" << endl;
				return;
			}
			//std::cout << xpos << " " << ypos << " " << depth << std::endl;
			
			if (point_selection_index % 2 == 0)
			{
				set_model_coord(xpos, ypos, depth, point_selection_index / 2);
				cout << "[Info] " << point_selection_index / 2 + 1 << "-th model point is selected..." << endl;
				point_selection_index = (point_selection_index + 1) % 8;
			}
			else
			{
				set_model_coord(xpos, ypos, depth, (point_selection_index - 1) / 2);
				cout << "[Info] " << (point_selection_index - 1) / 2 + 1 << "-th model point is changed..." << endl;
			}
		}
	}
	if (window == texwindow) // odd number: 1, 3, 5, 7
	{
		if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
		{
			if (point_selection_index % 2 == 1)
			{
				set_texture_coord(xpos, ypos, point_selection_index / 2);
				cout << "[Info] " << point_selection_index / 2 + 1 << "-th texture point is selected..." << endl;
				point_selection_index = (point_selection_index + 1) % 8;
			}
			else
			{
				set_texture_coord(xpos, ypos, (point_selection_index - 1) / 2);
				cout << "[Info] " << (point_selection_index - 1) / 2 + 1 << "-th texture point is changed..." << endl;
			}
		}
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void draw_model()
{
	// Clear the colorbuffer
	glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	pShader->Use();   // <-- Don't forget this one!
					// Transformation matrices

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 10.0f, 999999999999.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(pShader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(pShader->Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

	// Camera Distance
	GLfloat camera_dist = glm::distance(camera.Position, glm::vec3(0.0f));
	glm::vec3 camera_pos = camera.Position;
	glUniform3f(glGetUniformLocation(pShader->Program, "camera_pos"), camera_pos.x, camera_pos.y, camera_pos.z);
	glUniform1f(glGetUniformLocation(pShader->Program, "camera_dist"), camera_dist);
	glUniform1f(glGetUniformLocation(pShader->Program, "max_bounding_value"), pModel->getMaxBoundingValue(current_camera_pos));

	// Draw the loaded model
	glm::mat4 model;
	model = glm::translate(model, model_correction);
	model = glm::rotate(model, glm::radians(model_rotate.x), glm::vec3(1.0, 0.0, 0.0));
	model = glm::rotate(model, glm::radians(model_rotate.y), glm::vec3(0.0, 1.0, 0.0));
	model = glm::rotate(model, glm::radians(model_rotate.z), glm::vec3(0.0, 0.0, 1.0));
	model = glm::translate(model, model_translation);
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// It's a bit too big for our scene, so scale it down
	glUniformMatrix4fv(glGetUniformLocation(pShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	pModel->Draw(*pShader);

	// Swap the buffers
	glfwSwapBuffers(objwindow);
}

#pragma endregion