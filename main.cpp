#include <cassert>
#include <cstdlib>
#include <cstdio>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_GLEXT
#endif
#include <GLFW/glfw3.h>

struct BCBlock {
	union {
		uint8_t raw[8];
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
				uint32_t texels;
				struct {
					uint8_t x0: 2;
					uint8_t x1: 2;
					uint8_t x2: 2;
					uint8_t x3: 2;
				} y[4];
			};
		} bc1;
		union {
			struct {
				uint32_t _pad: 16;
				uint32_t texels: 24;
			};
			struct {
				uint8_t endpt[2];
				struct __attribute__((packed)) {
					uint8_t y0x0: 3;
					uint8_t y0x1: 3;
					uint8_t y0x2: 3;
					uint8_t y0x3: 3;

					uint8_t y1x0: 3;
					uint8_t y1x1: 3;
					uint8_t y1x2: 3;
					uint8_t y1x3: 3;
						
					uint8_t y2x0: 3;
					uint8_t y2x1: 3;
					uint8_t y2x2: 3;
					uint8_t y2x3: 3;
						
					uint8_t y3x0: 3;
					uint8_t y3x1: 3;
					uint8_t y3x2: 3;
					uint8_t y3x3: 3;
				};
			};
		} bc4;
		uint64_t data;
	};
};

static_assert(sizeof(BCBlock) == 8, "BC block should be 8 bytes");

typedef float Rf4x4[4][4];
typedef uint16_t Rs4x4[4][4];
typedef uint8_t Rb4x4[4][4];

static uint32_t u32Bits(float val) {
	union {
		float    f;
		uint32_t u;
	} bits = {
		val
	};
	return bits.u;
}

void dumpBits(const Rf4x4& block, bool newline = true) {
	printf("Float bits\n0x%08X\n0x%08X\n0x%08X\n0x%08X\n%s",
		u32Bits(block[0][0]),
		u32Bits(block[0][1]),
		u32Bits(block[0][2]),
		u32Bits(block[0][3]), (newline) ? "\n" : "");
}

void dump(const Rf4x4& block, bool newline = true) {
	printf("Floats\n%0.9f\n%0.9f\n%0.9f\n%0.9f\n%s",
		block[0][0],
		block[0][1],
		block[0][2],
		block[0][3], (newline) ? "\n" : "");
}

void dump(const Rs4x4& block, bool newline = true) {
	printf("Shorts\n0x%04X\n0x%04X\n0x%04X\n0x%04X\n%s",
		block[0][0],
		block[0][1],
		block[0][2],
		block[0][3], (newline) ? "\n" : "");
}

void dump(const Rb4x4& block, bool newline = true) {
	printf("Bytes\n0x%02X\n0x%02X\n0x%02X\n0x%02X\n%s",
		block[0][0],
		block[0][1],
		block[0][2],
		block[0][3], (newline) ? "\n" : "");
}

void dumpBoundRedChannel() {
	Rf4x4 f32blk;
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, f32blk);
	Rs4x4 u16blk;
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_SHORT, u16blk);
	Rb4x4 u08blk;
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, u08blk);
	GLenum err = glGetError();
	assert(err == 0);

	dumpBits(f32blk);
	dump(f32blk);
	dump(u16blk);
	dump(u08blk);
}

void bc3RedTest() {
	BCBlock block[2] = {};
	// block[0] is the alpha
	block[1].bc1.endpt[0].r = 31;
	block[1].bc1.endpt[1].r = 0;
	block[1].bc1.texels = 0xEE44EE44;
	GLuint txName = 0;
	glGenTextures(1, &txName);
	glBindTexture(GL_TEXTURE_2D, txName);
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 4, 4, 0, 16, block);

	dumpBoundRedChannel();
	
	glDeleteTextures(1, &txName);
}

void bc4RedTest() {
	BCBlock block[1] = {};
	block[0].bc4.endpt[0] = 0xFF;
	block[0].bc4.endpt[1] = 0x00;
	block[0].bc4.y0x0 = 0;
	block[0].bc4.y0x1 = 1;
	block[0].bc4.y0x2 = 2;
	block[0].bc4.y0x3 = 3;
	block[0].bc4.y1x0 = 4;
	block[0].bc4.y1x1 = 5;
	block[0].bc4.y1x2 = 6;
	block[0].bc4.y1x3 = 7;
	
	GLuint txName = 0;
	glGenTextures(1, &txName);
	glBindTexture(GL_TEXTURE_2D, txName);
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RED_RGTC1, 4, 4, 0, 8, block);
	
	dumpBoundRedChannel();
	
	glDeleteTextures(1, &txName);
}

void setup() {
	//bc3RedTest();
	bc4RedTest();
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
	// No need to show the window, but we do need to create it for GLFW to work
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(512, 512, "Test", NULL, NULL);
	if (!window) {
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);

	setup();

#if 0
	while (!glfwWindowShouldClose(window)) {
		draw(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
#endif

	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;
}
