#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <iomanip>
using namespace std;

//change width and height to add padding to different photos!

#define W 1280         // Image width...
#define H 720          // Image height...
#define MAX W*H        // Image Resolution

#define sig 2.0         // good values 2.0, 5, 8500
#define kernel 7      // row/columns number

#define P (kernel/2)   // Padding in pixels
#define S (2*P + W)    // Stride = 2*3 + 1280
#define HP (2*P + H)   // Height with Padding = 2*3 + 720
#define B 1            // unsigned...
#define MAXP S*HP      // New Resolution With Padding

#define A 1 //sharpen amount
#define T 3000	//noise threshold

unsigned short photo[MAX];
unsigned short paddedPhoto[MAXP];
unsigned short gaussedPhoto[MAX];
unsigned short sharpenPhoto[MAX];
unsigned short edgedPhoto[MAX];
unsigned char RGBedgedPhoto[MAX*3];
unsigned short FL[S];
unsigned short LL[S];
unsigned short ML[S];
unsigned short angleArray[MAX];

void convertGrayToRGB();
void gaussian_blur();
void applyEdgeDetection();
void addSharpen();
void addPad();

int main()
{
	FILE *f, *g;

	f = fopen("IMG_1280x720_16bpp.raw", "rb");
	g = fopen("abc1.raw", "wb");

	fread(photo, sizeof(photo), 1, f);

	printf("Image resolution: %dx%d - %d pixels.\n", W, H, MAX);
	printf("Stride ( width with padding ) = %d pixels.\n", S);
	printf("Height with padding = %d pixels.\n", HP);
	printf("Image resolution with padding : %dx%d - %d pixels.\n", S, HP, MAXP);

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

	unsigned short thisAngle;
    unsigned short newAngle;
    unsigned short valX, valY = 0; unsigned short GX [3][3]; unsigned short GY [3][3];
    unsigned short val1;

    //Sobel Horizontal Mask
    GX[0][0] = 1; GX[0][1] = 0; GX[0][2] = -1;
    GX[1][0] = 2; GX[1][1] = 0; GX[1][2] = -2;
    GX[2][0] = 1; GX[2][1] = 0; GX[2][2] = -1;

    //Sobel Vertical Mask
    GY[0][0] =  1; GY[0][1] = 2; GY[0][2] =   1;
    GY[1][0] =  0; GY[1][1] = 0; GY[1][2] =   0;
    GY[2][0] = -1; GY[2][1] =-2; GY[2][2] =  -1;

    for(int i=0; i < H; i++)
        {
            for(int j=0; j < W; j++) // i = y, j = x
            { //IMG[Y * (B * W) + B*X)
              //setting the pixels around the border to 0,
              //because the Sobel kernel cannot be allied to them
                if ((i==0)||(i==H-1)||(j==0)||(j==W-1))
                {
                   valX=0;
                   valY=0;
                }
                else
                {
                    valX=0;
                    valY=0;
                    //calculating the X and Y convolutions
                    for (int x = -1; x <= 1; x++)
                    {
                        for (int y = -1; y <= 1; y++)
                        {
                            valX = valX + gaussedPhoto[(i+x)*W+j+y] * GX[1+x][1+y];
                            valY = valY + gaussedPhoto[(i+x)*W+j+y] * GY[1+x][1+y];
                        }
                    }
                }
                //Gradient magnitude
                val1 = sqrt(valX*valX + valY*valY);

                //setting the new pixel value
                edgedPhoto[i*W+j] = val1;

                thisAngle = atan2(valY,valX)* 180.0 / 3.14159;		// Calculate actual direction of edge

                if ((thisAngle < 15) && (thisAngle > -15))
                    newAngle = 0;
                if (((thisAngle > 15) && (thisAngle < 30)) || ((thisAngle > -15) && (thisAngle < -30)))
                    newAngle = 30;
                if (((thisAngle > 30) && (thisAngle < 45)) || ((thisAngle > -30) && (thisAngle < -45)))
                    newAngle = 60;
                if (((thisAngle > 45) && (thisAngle < 60)) || ((thisAngle > -45) && (thisAngle < -60)))
                    newAngle = 90;
                if (((thisAngle > 60) && (thisAngle < 75)) || ((thisAngle > -60) && (thisAngle < -75)))
                    newAngle = 120;
                if (((thisAngle > 75) && (thisAngle < 90)) || ((thisAngle > -75) && (thisAngle < -90)))
                    newAngle = 150;

                angleArray[i*W+j] = newAngle;
            }
        }

    convertGrayToRGB();

    for(int i = 0; i < MAX; i++){

            if(angleArray[i] == 0 && edgedPhoto[i] > 9500){ // red
                RGBedgedPhoto[i] = 0;
                RGBedgedPhoto[i+MAX] = 0;
                RGBedgedPhoto[i+MAX*2] = 255;
                //printf("RED\n");
            }else if (angleArray[i] == 30 && edgedPhoto[i] > 9500){ // orange
                RGBedgedPhoto[i] = 153;
                RGBedgedPhoto[i+MAX] = 153;
                RGBedgedPhoto[i+MAX*2] = 255;
                //printf("Orange\n");
            }else if (angleArray[i] == 60 && edgedPhoto[i] > 9500) { // green
                RGBedgedPhoto[i] = 0;
                RGBedgedPhoto[i+MAX] = 255;
                RGBedgedPhoto[i+MAX*2] = 0;
                //printf("Yellow\n");
            }else if (angleArray[i] == 90 && edgedPhoto[i] > 9500){ // yellow
                RGBedgedPhoto[i] = 255;
                RGBedgedPhoto[i+MAX] = 165;
                RGBedgedPhoto[i+MAX*2] = 0;
                //printf("Green\n");
            }else if (angleArray[i] == 120 && edgedPhoto[i] > 9500){ // blue
                RGBedgedPhoto[i] = 0;
                RGBedgedPhoto[i+MAX] = 244;
                RGBedgedPhoto[i+MAX*2] = 244;
                //printf("Blue\n");
            }else if (angleArray[i] == 150 && edgedPhoto[i] > 9500){ // red
                RGBedgedPhoto[i] = 255;
                RGBedgedPhoto[i+MAX] = 0;
                RGBedgedPhoto[i+MAX*2] = 127;
                //printf("Purple\n");
            }else{
                RGBedgedPhoto[i] = 0;
                RGBedgedPhoto[i+MAX] = 0;
                RGBedgedPhoto[i+MAX*2] = 0;
            }
    }

	fwrite(RGBedgedPhoto, sizeof(RGBedgedPhoto), 1, g);
	fclose(f);
	fclose(g);
	printf("\nDONE!");
	return 0;
}

void addSharpen()
{
    for (int i = 0; i < MAX; i++) {
		if (abs((photo[i] + A * (photo[i] - gaussedPhoto[i])) > T)) {
            if ((photo[i] + A * (photo[i] - gaussedPhoto[i])) > 65535) {
                sharpenPhoto[i] = 65535;
            } else {
                sharpenPhoto[i] = (photo[i] + A * (photo[i] - gaussedPhoto[i]));
			}
		}
	}
}

void applyEdgeDetection()
{
    // write misery here
    unsigned short thisAngle;
    unsigned short newAngle;
    unsigned short valX, valY = 0; unsigned short GX [3][3]; unsigned short GY [3][3];
    unsigned short val1;
    //Sobel Horizontal Mask
    GX[0][0] = 1; GX[0][1] = 0; GX[0][2] = -1;
    GX[1][0] = 2; GX[1][1] = 0; GX[1][2] = -2;
    GX[2][0] = 1; GX[2][1] = 0; GX[2][2] = -1;

    //Sobel Vertical Mask
    GY[0][0] =  1; GY[0][1] = 2; GY[0][2] =   1;
    GY[1][0] =  0; GY[1][1] = 0; GY[1][2] =   0;
    GY[2][0] = -1; GY[2][1] =-2; GY[2][2] =  -1;

    for(int i=0; i < H; i++)
        {
            for(int j=0; j < W; j++) // i = y, j = x
            {  //IMG[Y * (B * W) + B*X)
               //setting the pixels around the border to 0,
               //because the Sobel kernel cannot be allied to them
                if ((i==0)||(i==H-1)||(j==0)||(j==W-1))
                {
                   valX=0;
                   valY=0;
                }
                else
                {
                    valX=0;
                    valY=0;
                    //calculating the X and Y convolutions
                    for (int x = -1; x <= 1; x++)
                    {
                        for (int y = -1; y <= 1; y++)
                        {
                            valX = valX + gaussedPhoto[(i+x)*W+j+y] * GX[1+x][1+y];
                            valY = valY + gaussedPhoto[(i+x)*W+j+y] * GY[1+x][1+y];
                        }
                    }
                }

                //Gradient magnitude
                val1 = sqrt(valX*valX + valY*valY);

                //setting the new pixel value
                edgedPhoto[i*W+j] = val1;

                thisAngle = atan2(valY,valX)* 180.0 / 3.14159;		// Calculate actual direction of edge

                //printf("thisAngle = %d\n", thisAngle);

                if ((thisAngle < 15) && (thisAngle > -15))
                    newAngle = 0;
                if (((thisAngle > 15) && (thisAngle < 30)) || ((thisAngle > -15) && (thisAngle < -30)))
                    newAngle = 30;
                if (((thisAngle > 30) && (thisAngle < 45)) || ((thisAngle > -30) && (thisAngle < -45)))
                    newAngle = 60;
                if (((thisAngle > 45) && (thisAngle < 60)) || ((thisAngle > -45) && (thisAngle < -60)))
                    newAngle = 90;
                if (((thisAngle > 60) && (thisAngle < 75)) || ((thisAngle > -60) && (thisAngle < -75)))
                    newAngle = 120;
                if (((thisAngle > 75) && (thisAngle < 90)) || ((thisAngle > -75) && (thisAngle < -90)))
                    newAngle = 150;

                angleArray[i*W+j] = newAngle;

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

	unsigned short newPixelValue = 0;
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

void convertGrayToRGB()
{
    for (int i = 0; i < MAX; i++){
            RGBedgedPhoto[i] = edgedPhoto[i]/256;
            RGBedgedPhoto[i+MAX] = edgedPhoto[i]/256;
            RGBedgedPhoto[i+MAX*2] = edgedPhoto[i]/256;
        }

}
