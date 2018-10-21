#include "GLFW/glfw3.h"
#include <cstdlib>

int main(int argc, char** argv)
{
        if (!glfwInit()) {
                exit(EXIT_FAILURE);
        }
        return 0;
}
