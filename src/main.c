#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "types.h"
#include "lexer.h"
#include "parser.h"
#include "config.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(void)
{
  const char *configPath = "../unchair.un";
  Position *position = InitializePostion();
  if (position == NULL)
  {
    return 1;
  }

  TokenStream *tokenStream = InitializeStream();

  if (tokenStream == NULL)
  {
    free(position);
    return 1;
  }

  Config *config = InitializeConfig();

  if (config == NULL)
  {
    FreeStream(tokenStream);
    free(position);
    return 1;
  }

  FILE *fp = fopen(configPath, "r");

  if (fp == NULL)
  {
    printf("Error: Unable to open file 'unchair.un'.\n");
    FreeStream(tokenStream);
    free(position);
    FreeConfig(config);
    return 1;
  }

  int lexed = Lex(fp, tokenStream, position);
  fclose(fp);

  if (lexed == 0)
  {
    CleanupTokens(tokenStream);
    FreeStream(tokenStream);
    free(position);
    FreeConfig(config);
    return 1;
  }

  // call parser here after closing the file.
  int res = ParseTokens(tokenStream, config);

  if (res == 0)
  {
    CleanupTokens(tokenStream);
    FreeStream(tokenStream);
    free(position);
    FreeConfig(config);
    return 1;
  }

  const char *imagePath = "../reactor.jpeg";
  int width, height, channels_in_file;

  unsigned char *image = stbi_load(
      imagePath,         // filename
      &width,            // output for width
      &height,           // output for height
      &channels_in_file, // output for original channels in file
      3                  // desired number of channels
  );
  printf("Width : %d\n", width);
  printf("Height : %d\n", height);
  printf("Original channels : %d\n", channels_in_file);

  // Resize dimensions, preserving aspect ratio
  const int resize_width = 256;
  const int resize_height =
      (int)((double)height * resize_width / width);

  const double scaleX = (double)width / resize_width;
  const double scaleY = (double)height / resize_height;

  printf("Ratio Width : %lf\n", scaleX);
  printf("Ratio Height : %lf\n", scaleY);

  // Three bytes per destination pixel (RGB)
  unsigned char *output_img =
      malloc(resize_width * resize_height * 3);

  if (output_img == NULL)
  {
    printf("Memory allocation failed\n");
    stbi_image_free(image);
    return 1;
  }

  int index = 0;

  // Nearest-neighbour resizing
  for (int i = 0; i < resize_height; i++)
  {
    for (int j = 0; j < resize_width; j++)
    {
      int sourceX = (int)floor(j * scaleX);
      int sourceY = (int)floor(i * scaleY);

      int offset =
          (sourceY * width + sourceX) * 3;

      output_img[index] = image[offset];
      output_img[index + 1] = image[offset + 1];
      output_img[index + 2] = image[offset + 2];

      index += 3;
    }
  }

  // Write resized RGB image
  int success = stbi_write_png(
      "../output.png",
      resize_width,
      resize_height,
      3,
      output_img,
      resize_width * 3);

  if (!success)
  {
    printf("Failed to write output image\n");
  }
  else
  {
    printf("Resized image saved successfully\n");
  }

  free(output_img);
  CleanupTokens(tokenStream);
  FreeStream(tokenStream);
  free(position);
  FreeConfig(config);
  stbi_image_free(image);
  return 0;
}