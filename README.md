Sobel Edge Detection on the Parallella
======================================

This program performs Sobel edge detection on a video feed from OpenCV.


## Implementation

Host side:

* Controlling and synchronizing the eCores
* Managing frame-buffer, OpenCV
* Transfer grayscale image data to emem

Device(Epiphany) side:

* Get the image data from emem
* Apply the Sobel edge detection algorithm
* After rendering a frame, halt until interrupted by the host.

## Building

System requirements:

* [Parallella board Gen 1.1 or later](http://www.parallella.org/)
* Parallella Official Ubuntu
* Epiphany SDK 5 or later
* OpenCV compatible webcam
* [OpenCV](http://opencv.org)

Install dependent packages:

    sudo apt-get update

    sudo apt-get install pkg-config build-essential libopencv-dev

Make sure the pkg-config files are available:

    sudo wget -O /usr/local/lib/pkgconfig/coprthr.pc "https://gist.githubusercontent.com/sclukey/2969d30e33c6eac57ba2/raw/f749fbbf9a804b5e46849dbfe15e484a11f9d10b/coprthr.pc"

The programs should now build and run.

## License

BSD 3-Clause license
