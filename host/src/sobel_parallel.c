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
	double* elapsed = (double*)malloc(sizeof(double));

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

	cl_uint width    = image->width;
	cl_uint height   = image->height;
	cl_uint channels = image->nChannels;
	cl_uint n        = height*width*channels;
	cl_uint line     = width*channels;
	
	unsigned int devnum;

	// Use default contexts, if no ACCELERATOR use CPU
	CLCONTEXT* cp = (stdacc) ? stdacc : stdcpu;

	cl_kernel krn = clsym(cp, 0, "matvecmult_kern", 0);

	// Allocate OpenCL device-sharable memory
	cl_char* aa = (char*)clmalloc(cp, width*height*channels*sizeof(cl_char), 0);
	cl_char* bb = (char*)clmalloc(cp, width*height*channels*sizeof(cl_char), 0);

	// Define the computational domain and workgroup size
	clndrange_t ndr = clndrange_init1d(0, 16, 16);

	cvNamedWindow("Input", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Output", CV_WINDOW_AUTOSIZE);

	while(1) {
		gettimeofday(start, NULL);

		printf("Getting the image from the camera\n");
		colorImage = cvQueryFrame(cap);

		printf("Converting to gray\n");
		cvCvtColor(colorImage, image, CV_BGR2GRAY);

		// Initialize vector aa[] and zero bb[]
		printf("Initializing\n");
		int i,j;
		for(i=0; i<n; i++) aa[i] = image->imageData[i];
		//for(i=0; i<n; i++) bb[i] = '\0';
		
		// Non-blocking sync vectors a and b to device memory (copy to Epiphany)
		printf("syncing to device\n");
		clmsync(cp, devnum, aa, CL_MEM_DEVICE | CL_EVENT_NOWAIT);

		// Non-blocking for of the OpenCL kernel to execute on the Epiphany
		printf("Running the process\n");
		clforka(cp, devnum, krn, &ndr, CL_EVENT_NOWAIT, n, line, aa, bb);

		// Non blocking sync vector c to host memory (copy back to host)
		printf("Syncing back to the host\n");
		clmsync(cp, devnum, bb, CL_MEM_HOST | CL_EVENT_NOWAIT);

		// Force execution of operations in command queue (non-blocking call)
		printf("Flushing\n");
		clflush(cp, devnum, 0);

		// Block on completion of operations in command queue
		//clwait(cp, devnum, CL_ALL_EVENT);

		printf("Putting the output into the output image\n");
		for(i=0; i<height*width*channels; i++) outputImage->imageData[i] = bb[i];

		printf("Showing the images\n");
		cvShowImage("Input", image);
		cvShowImage("Output", outputImage);

		gettimeofday(end, NULL);

		*elapsed = end->tv_sec + (end->tv_usec/1000000.0) - start->tv_sec - (start->tv_usec/1000000.0);
		printf("This frame took: %0.6lf seconds for a rate of %0.6lf fps\n", (*elapsed), 1/(*elapsed));

		if (cvWaitKey(30) >= 0) break;
	}

	clfree(aa);
	clfree(bb);

	cvWaitKey(0);

}
