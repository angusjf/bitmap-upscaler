#include <stdlib.h>
#include <stdio.h> 

int getWidth(FILE * file);
int getHeight(FILE * file);
int getOffset(FILE * file);
int getBitsPerPixel(FILE * file);
char *** loadImage(FILE * file, int width, int height);
void saveBitmap(char *** image, int width, int height, FILE * inputFile, int scale);
void freeImage(char *** image, int width, int height);

int main(int argc, char ** argv) {
	if (argc < 4) {
		fprintf(stderr, "error: command usage %s <scale> <inputfile> <outputfile>\n", argv[0]);
		exit(1);
	}

	int scale = argv[1][0] - '0';
	if (scale < 1) {
		fprintf(stderr, "error: attemped to scale by %i (scale must be greater than 0)\n", scale);
		exit(1);
	}

	FILE * inputFile = fopen(argv[2], "rb");

	if (inputFile == NULL) {
		fprintf(stderr, "error: input file %s could not be opened\n", argv[2]);
		exit(1);
	}

	int width = getWidth(inputFile);
	int height = getHeight(inputFile);

	char *** image = loadImage(inputFile, width, height);

	// make height always positive
	height = height > 0 ? height : -height;

	fclose(inputFile);

	FILE * outputFile = fopen(argv[3], "wb");
	if (outputFile == NULL) {
		fprintf(stderr, "error: output file %s could not be opened\n", argv[3]);
		freeImage(image, width, height);
		exit(1);
	}

	saveBitmap(image, width, height, outputFile, scale);

	freeImage(image, width, height);

	return 0;
}

int getWidth(FILE * file) {
	int width;
	fseek (file, 18, SEEK_SET);
	fread(&width, sizeof(int), 1, file);
	return width;
}

int getHeight(FILE * file) {
	int height;
	fseek (file, 22, SEEK_SET);
	fread(&height, sizeof(int), 1, file);
	return height;
}

int getOffset(FILE * file) {
	int offset;
	fseek (file, 10, SEEK_SET);
	fread(&offset, sizeof(int), 1, file);
	return offset;
}

int getBitsPerPixel(FILE * file) {
	int bpp;
	fseek (file, 28, SEEK_SET);
	fread(&bpp, sizeof(int), 1, file);
	return bpp;
}

char *** loadImage(FILE * imageFile, int width, int height) {

	int upsideDown = height < 0 ? 0 : 1;
	height = height > 0 ? height : -height;

	int bitsPerPixel = getBitsPerPixel(imageFile);

	char *** output = (char ***)malloc(sizeof(char **) * height);

	int offset = getOffset(imageFile);
	fseek (imageFile, offset, SEEK_SET);
	
	for (int i = 0; i < height; i++) {
		int y = upsideDown ? height - i - 1 : i;
		output[y] = (char **)malloc(sizeof(char *) * width);
		for (int j = 0; j < width; j++) {
			int x = j;
			output[y][x] = (char *)malloc(sizeof(char) * 3);
			unsigned char red, green, blue, alpha;
			fread(&blue, sizeof(char), 1, imageFile); // BLUE
			fread(&green, sizeof(char), 1, imageFile); // GREEN
			fread(&red, sizeof(char), 1, imageFile); // RED
			if (bitsPerPixel == 32) {
				fread(&alpha, sizeof(char), 1, imageFile); // ALPHA
			}
			output[y][x][0] = red;
			output[y][x][1] = green;
			output[y][x][2] = blue;
		}
	}
	
	return output;
}

void saveBitmap(char *** image, int oldWidth, int oldHeight, FILE * outputFile, int scale) {
	const char B = 'B', M = 'M';
	int newWidth = oldWidth * scale;
	int newHeight = oldHeight * scale;
	int one = 1, zero = 0, offset = 54, bpp = 32, headerSize = 40, thousand = 1000; 
	char tff = 0xff;

	//FILE
	fwrite(&B, sizeof(char), 1, outputFile); // B
	fwrite(&M, sizeof(char), 1, outputFile); // M
	fwrite(&one, sizeof(int), 1, outputFile); // SIZE
	fwrite(&zero, sizeof(int), 1, outputFile); // ZERO
	fwrite(&offset, sizeof(int), 1, outputFile); // OFFSET

	//IMAGE
	fwrite(&headerSize, sizeof(int), 1, outputFile); // HEADER SIZE
	fwrite(&newWidth, sizeof(int), 1, outputFile); // WIDTH
	newHeight = -newHeight; // height is negative as top to bottom
	fwrite(&newHeight, sizeof(int), 1, outputFile); // HEIGHT height = -height;
	newHeight = -newHeight; // put it back to positive
               
	fwrite(&one, sizeof(char) * 2, 1, outputFile); // ONE (2 BYTES)
	fwrite(&bpp, sizeof(char) * 2, 1, outputFile); // BITS PER PIXEL  (2 BYTES)
               
	fwrite(&zero, sizeof(int), 1, outputFile); // ZERO
	fwrite(&zero, sizeof(int), 1, outputFile); // ZERO
	fwrite(&thousand, sizeof(int), 1, outputFile); // THOUSAND
	fwrite(&thousand, sizeof(int), 1, outputFile); // THOUSAND
               
	fwrite(&one, sizeof(int), 1, outputFile); // ??
	fwrite(&one, sizeof(int), 1, outputFile); // ??

	char *** newImage = malloc(sizeof(char **) * oldHeight * scale);
	for (int i = 0; i < oldHeight * scale; i++) {
		newImage[i] = (char **)malloc(sizeof(char *) * oldWidth * scale);
		for (int j = 0; j < oldWidth * scale; j++) {
			//newImage[i][j] = image[i/scale][j/scale]; TODO SHOULD WORK
			newImage[i][j] = (char *)malloc(sizeof(char) * 3);
			newImage[i][j][0] = image[i/scale][j/scale][0];
			newImage[i][j][1] = image[i/scale][j/scale][1];
			newImage[i][j][2] = image[i/scale][j/scale][2];
		}
	}

	for (int i = 0; i < oldHeight * scale; i++) {
		for (int j = 0; j < oldWidth * scale; j++) {
			fwrite(&newImage[i][j][2], sizeof(char), 1, outputFile); // b
			fwrite(&newImage[i][j][1], sizeof(char), 1, outputFile); // g
			fwrite(&newImage[i][j][0], sizeof(char), 1, outputFile); // r
			fwrite(&tff, sizeof(char), 1, outputFile); // a
		}
	}

	freeImage(newImage, oldWidth * scale, oldHeight * scale);
	
	fclose(outputFile);
}

void freeImage(char *** image, int width, int height) {
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			free(image[i][j]);
		}
		free(image[i]);
	}
	free(image);
}
