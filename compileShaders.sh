rm shaders/*.spv
glslangValidator -V shaders/main.vert -o shaders/main.vert.spv
glslangValidator -V shaders/main.frag -o shaders/main.frag.spv
glslangValidator -V shaders/test.vert -o shaders/test.vert.spv
glslangValidator -V shaders/test.frag -o shaders/test.frag.spv
