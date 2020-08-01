#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <iomanip>
using namespace std;

//change width and height to add padding to different photos!

#define W 1280         // Image width...
#define H 720          // Image height...
#define MAX W*H        // Image Resolution

#define sig 1.0
#define kernel 5      // row/columns number

#define P (kernel/2)   // Padding in pixels
#define S (2*P + W)    // Stride = 2*3 + 1280
#define HP (2*P + H)   // Height with Padding = 2*3 + 720
#define B 1            // unsigned...
#define MAXP S*HP      // New Resolution With Padding

#define A 7 //sharpen amount
#define T 5000	//noise threshold

unsigned short photo[MAX];
unsigned short paddedPhoto[MAXP];
unsigned short gaussedPhoto[MAX];
unsigned short sharpenPhoto[MAX];
unsigned short FL[S];
unsigned short LL[S];
unsigned short ML[S];
// IMG[Y * (B * W) + B*X)   720 * ( 1 * 1280 ) + 1*1
// IMG[(P + Y) * S*B + (P + X) * B]
// P PADDING
// S STRIDE = 2 * P + W
// B Bytes per pixel - 2 ( but for our case actually 1 )
// W Width - 1280
// H Height - 720

void addPad();

void gaussian_blur();



int main()
{
	FILE *f, *g;

	f = fopen("IMG_1280x720_16bpp.raw", "rb");
	g = fopen("outputSharpenedPhoto.raw", "wb");

	fread(photo, sizeof(photo), 1, f);


	printf("Image resolution: %dx%d - %d pixels.\n", W, H, MAX);
	printf("Stride ( width with padding ) = %d pixels.\n", S);
	printf("Height with padding = %d pixels.\n", HP);
	printf("Image resolution with padding : %dx%d - %d pixels.\n", S, HP, MAXP);

	addPad();

	gaussian_blur();

	for (int i = 0; i < MAX; i++) {
		if (abs((photo[i] + A * (photo[i] - gaussedPhoto[i])) > T)) {
            if ((photo[i] + A * (photo[i] - gaussedPhoto[i])) > 65535) {
                sharpenPhoto[i] = 65535;
            } else {
                sharpenPhoto[i] = (photo[i] + A * (photo[i] - gaussedPhoto[i]));
			}
		}
	}

	fwrite(sharpenPhoto, sizeof(sharpenPhoto), 1, g);
	fclose(f);
	fclose(g);
	printf("\nDONE!");
	return 0;
}

void gaussian_blur() {
	int index = 0, row = 0, col = 0;
	double window[kernel*kernel], storageMatrix[kernel*kernel];

	/////////////////////// Kernel creation ///////////////////////////////////

	double gk[kernel][kernel];
	double r, s = 2.0 * sig * sig;  // Assigning standard deviation to 1.0
	double sum = 0.0;   // Initialization of sun for normalization
	for (int x = -P; x <= P; x++) // Loop to generate 5x5 kernel
	{
		for (int y = -P; y <= P; y++)
		{
			r = sqrt(x*x + y * y);
			gk[x + P][y + P] = (exp(-(r*r) / s)) / (3.14 * s);
			sum += gk[x + P][y + P];
		}
	}

	for (int i = 0; i < kernel; ++i) // Loop to normalize the kernel
		for (int j = 0; j < kernel; ++j) {
			gk[i][j] /= sum;
		}

	for (int i = 0; i < kernel; ++i) // loop to display the generated 5 x 5 Gaussian filter
	{
		for (int j = 0; j < kernel; ++j)
			cout << gk[i][j] << "\t";
		cout << endl;
	}

	cout << endl;
	double sum2 = 0;
	for (int i = 0; i < kernel; ++i) {
		for (int j = 0; j < kernel; ++j) {
			sum2 += gk[i][j];
		}
	}

	/////////////////////////// get matrix from the paddedPhoto ///////////////////////////////

	unsigned short newPixelValue = 0, test = 0;

	for (row = P; row <= W; ++row) { // iterate trough the width of the photo
		for (col = P; col <= H; ++col) { // iterate trough the height of the photo
			index = 0;
			for (int c = -P; c <= P; c++) { // collect pixels from the column near the pixel that is filtered
				for (int r = -P; r <= P; r++) { // collect pixels from the row near the pixel that is filtered
					window[index] = paddedPhoto[(col + c + P)*S + P + row + r];
					index++;
				}
			}

			for (int i = 0; i < kernel; i++) {
				for (int j = 0; j < kernel; j++) {
					storageMatrix[i*kernel + j] = window[i*kernel + j] * gk[i][j];
				}
			}

			double sumOfMatrix = 0;

			for (int k = 0; k < kernel*kernel; k++)
			{
				sumOfMatrix += storageMatrix[k];
			}

			newPixelValue = (unsigned short)sumOfMatrix;
			gaussedPhoto[(col - P)*W + row - P] = newPixelValue;
		}
	}
}

void addPad()
{
	for (int i = 0; i < P; i++)
	{
		FL[i] = photo[0];
		FL[i + W + P] = photo[W - 1];
	}

	for (int i = P; i < S - P; i++)
	{
		FL[i] = photo[i - P];
	}

	for (int i = 0; i < P; i++)
	{
		memcpy(paddedPhoto + S * i, FL, sizeof(FL));
	}

	/////////////////////////////// middle fill//////////////////////////////////////////////
	int middle = P;

	do {

		for (int i = 0; i < P; i++) ////IMG[Y * (B * W) + B*X)
		{ // 1286
			ML[i] = photo[(middle - P)*W]; // 0, 1, 2
			ML[i + W + P] = photo[(middle - P)*W + W - 1]; // 1283, 1284, 1285
		}

		for (int i = P; i < S - P; i++) // i - 3 1282, middle 3 - 723
		{
			ML[i] = photo[(i - P) + (middle - P)*W]; // (i-P) - 0 - 1279, 1280 - 2559 |||
		}

		memcpy(paddedPhoto + S * middle, ML, sizeof(ML));

		middle++;
	} while (middle < H + P);

	///////////////////////////// last 3 lines in the padded image/////////////////////////////////

	for (int i = 0; i < P; i++)
	{
		LL[i] = photo[MAX - W + 1];
		LL[i + W + P] = photo[MAX - 1];
	}

	for (int i = P; i < S - P; i++)
	{
		LL[i] = photo[i];
	}

	for (int i = P; i > 0; i--)
	{
		memcpy(paddedPhoto + S * (HP - i), LL, sizeof(LL));
	}
}
