#include <imageLoader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void ImageLoader::freeImage(unsigned char *data) {
  stbi_image_free(data);
}

unsigned char* ImageLoader::loadImage(const char *path, int *width, int *height, int *nrChannels) {
  return stbi_load(path, width, height, nrChannels, 0);
}