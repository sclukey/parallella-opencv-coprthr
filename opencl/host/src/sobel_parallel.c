// hello_stdcl.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <stdcl.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

int main()
{
	struct timeval* start = (struct timeval*)malloc(sizeof(struct timeval));
	struct timeval* end   = (struct timeval*)malloc(sizeof(struct timeval));
	double* elapsed       = (double*)malloc(sizeof(double));
	unsigned int devnum;
	IplImage* colorImage;
	IplImage* grayImage;
	IplImage* outputImage;

	// ---------------------------
	// Initialize the OpenCL stuff
	// ---------------------------

	// Use default contexts, if no ACCELERATOR use CPU
	CLCONTEXT* cp = (stdacc) ? stdacc : stdcpu;

	cl_kernel krn = clsym(cp, 0, "sobel_kern", 0);

	// Define the computational domain and workgroup size
	clndrange_t ndr = clndrange_init1d(0, 16, 16);

	// ---------------------------
	// Initialize the OpenCV stuff
	// ---------------------------

	CvCapture* cap = cvCaptureFromCAM(0);
	if (!cap) {
		fprintf(stderr, "Could not open the camera\n");
		exit(2);
	}

	// Get an image from the camera to determine its size
	colorImage = cvQueryFrame(cap);
	if (!colorImage) {
		fprintf(stderr, "Could not read from the camera\n");
		exit(3);
	}

	cl_uint width    = colorImage->width;
	cl_uint height   = colorImage->height;
	cl_uint channels = 1;
	cl_uint n        = height*width*channels;
	cl_uint line     = width*channels;

	// Allocate OpenCL device-sharable memory
	cl_char* aa = (char*)clmalloc(cp, n*sizeof(cl_char), 0);
	cl_char* bb = (char*)clmalloc(cp, n*sizeof(cl_char), 0);

	// Create OpenCV IplImage headers to hold the data
	grayImage   = cvCreateImageHeader(cvGetSize(colorImage), colorImage->depth, 1);
	outputImage = cvCreateImageHeader(cvGetSize(colorImage), colorImage->depth, 1);

	// Use the shared buffers for the image data
	cvSetData(grayImage,   aa, line);
	cvSetData(outputImage, bb, line);

	cvNamedWindow("Input", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Output", CV_WINDOW_AUTOSIZE);

	while(1) {
		gettimeofday(start, NULL);

		// Getting the image from the camera
		colorImage = cvQueryFrame(cap);

		// Converting to grayscale;
		cvCvtColor(colorImage, grayImage, CV_BGR2GRAY);

		// Non-blocking sync vectors a and b to device memory (copy to Epiphany)
		clmsync(cp, devnum, aa, CL_MEM_DEVICE | CL_EVENT_NOWAIT);

		// Non-blocking for of the OpenCL kernel to execute on the Epiphany
		clforka(cp, devnum, krn, &ndr, CL_EVENT_NOWAIT, n, line, aa, bb);

		// Non blocking sync vector c to host memory (copy back to host)
		clmsync(cp, devnum, bb, CL_MEM_HOST | CL_EVENT_NOWAIT);

		// Force execution of operations in command queue (non-blocking call)
		//clflush(cp, devnum, 0);

		// Block on completion of operations in command queue
		clwait(cp, devnum, CL_ALL_EVENT);

		// Showing the images
		cvShowImage("Input", colorImage);
		cvShowImage("Output", outputImage);

		gettimeofday(end, NULL);

		*elapsed = end->tv_sec + (end->tv_usec/1000000.0) - start->tv_sec - (start->tv_usec/1000000.0);
		printf("This frame took: %0.6lf seconds for a rate of %0.6lf fps\n", (*elapsed), 1/(*elapsed));

		if (cvWaitKey(30) >= 0) break;
	}

	cvReleaseCapture(&cap);

	clfree(aa);
	clfree(bb);

	cvWaitKey(0);

}
