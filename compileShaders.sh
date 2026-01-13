rm shaders/*.spv
glslangValidator -V shaders/main.vert -o shaders/main.vert.spv
glslangValidator -V shaders/main.frag -o shaders/main.frag.spv
