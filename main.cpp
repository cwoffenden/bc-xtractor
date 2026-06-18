#include <cassert>
#include <cstdlib>

#if TARGET_OS_MAC && !TARGET_OS_IPHONE
#define GL_SILENCE_DEPRECATION
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
#define GL3_PROTOTYPES
#include <OpenGL/gl3.h>
#else
#include <OpenGL/gl.h>
#endif
#include <OpenGL/glext.h>
#endif

#include <GLFW/glfw3.h>

struct BCBlock {
	union {
		uint8_t bytes[8];
		struct {
			union {
				uint16_t rgb565;
				struct {
					uint16_t b: 5;
					uint16_t g: 6;
					uint16_t r: 5;
				};
			} endpt[2];
			union {
				uint32_t bits;
				struct {
					uint8_t x0: 2;
					uint8_t x1: 2;
					uint8_t x2: 2;
					uint8_t x3: 2;
				} y[4];
			} texels;
		} bc1;
		uint64_t data;
	};
};

static_assert(sizeof(BCBlock) == 8, "BC block should be 8 bytes");

BCBlock block[2] = {};

GLuint txName = 0;

void setup() {
	block[1].bc1.endpt[0].r = 31;
	block[1].bc1.endpt[1].r = 9;
	block[1].bc1.texels.bits = 0xBB44BB44;
	glGenTextures(1, &txName);
	glBindTexture(GL_TEXTURE_2D, txName);
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 4, 4, 0, 16, &block);

	float f32[4][4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, f32);
	uint16_t u16[4][4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_SHORT, u16);
	uint8_t u08[4][4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, u08);
	GLenum err = glGetError();
	assert(err == 0);
}

void draw(GLFWwindow* window) {
	int fbW, fbH;
	glfwGetFramebufferSize(window, &fbW, &fbH);
	glViewport(0, 0, fbW, fbH);
	glClearColor(0.3f, 0.4f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

int main(int /*argc*/, char* /*argv*/[]) {
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(512, 512, "Test", NULL, NULL);
	if (!window) {
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);

	setup();

	while (!glfwWindowShouldClose(window)) {
		draw(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;
}
