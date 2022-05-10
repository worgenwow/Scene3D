#pragma once

class ImageLoader {
  private:
  public:
    static void freeImage(unsigned char *data);
    static unsigned char* loadImage(const char *path, int *width, int *height, int *nrChannels);
};