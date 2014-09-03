// hello_stdcl.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

void sobel_image(uint n, uint line, char* aa, char* bb);

int main ()
{
	CvCapture* cap = cvCaptureFromCAM(0);
	if (!cap) {
		fprintf(stderr, "Could not open the camera\n");
		exit(2);
	}

	IplImage* colorImage = cvQueryFrame(cap);
	if (!colorImage) {
		fprintf(stderr, "Could not read from the camera\n");
		exit(3);
	}
	IplImage* image = cvCreateImage(cvGetSize(colorImage), colorImage->depth, 1);
	cvCvtColor(colorImage, image, CV_BGR2GRAY);
	IplImage* outputImage = cvCreateImage(cvGetSize(image), image->depth, image->nChannels);

	uint width    = image->width;
	uint height   = image->height;
	uint channels = image->nChannels;
	uint n        = height*width*channels;
	uint line     = width*channels;
	
	cvNamedWindow("Input", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Output", CV_WINDOW_AUTOSIZE);

	struct timeval start;
	struct timeval end;
	double elapsed;

	while(1) {
		gettimeofday(&start, NULL);
		colorImage = cvQueryFrame(cap);
		cvCvtColor(colorImage, image, CV_BGR2GRAY);

		// Initialize vector aa[] and zero bb[]
		sobel_image(n, line, image->imageData, outputImage->imageData);

		cvShowImage("Input", image);
		cvShowImage("Output", outputImage);

		gettimeofday(&end, NULL);
		elapsed = end.tv_sec + (end.tv_usec/1000000.0) - start.tv_sec - (start.tv_usec/1000000.0);
		printf("This frame took %0.6lf seconds giving a framerate of %0.6lf fps\n", elapsed, 1/elapsed);

		if (cvWaitKey(30) >= 0) break;
	}

	cvWaitKey(0);

}

void sobel_image (uint n, uint line, char* aa, char* bb)
{
	int i, m;
	double tmp;
	for(i=0; i<n; i++) {
		m = i % line;

		if (m == 0 || m == line-1 || i < line || n-i < line)
			bb[i] = 0;
		else {

			tmp = abs(-     aa[i-line-1]
			          - 2 * aa[i-1]
			          -     aa[i+line-1]
			          +     aa[i-line+1]
			          + 2 * aa[i+1]
			          +     aa[i+line+1])
			          + abs(      aa[i-line-1]
			          + 2 * aa[i-line]
			          +     aa[i-line+1]
			          -     aa[i+line-1]
			          - 2 * aa[i+line]
			          -     aa[i+line+1]);
			bb[i] = tmp > 255 ? 255 : tmp;
		}
	}
}
