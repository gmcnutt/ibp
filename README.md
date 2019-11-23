# Image Batch Processor

IBC performs batch operations on images.

## Dependencies

Depends on a `libcu`, a C utility lib available at
git@github.com:gmcnutt/cutils.git.

## Build

    make

## Usage

    Usage:  ibp [options] <files>
    Options:
    -c <alpha>: crop out transparent pixels
    -d: debug
    -h:	help
    -s: sort list output by image size
    -D <dir>: write modified images as .png here
    The default is to list the image attributes.

The default behavior is to list the image attributes of files, for example:

    $ ./ibp ~/Pictures/*
        w     h pitch       size     pixfmt name
     1253    92  3760     115276      RGB24 /home/gmcnutt/Pictures/100x1x1.png
     1394  1041  5576    1451154   ABGR8888 /home/gmcnutt/Pictures/reilly_abstraction_front.png
      450   450   452     202500     INDEX8 /home/gmcnutt/Pictures/Zygomatic_process_of_maxilla_-_animation02.gif

The `-c` option will crop transparency off the edge of images, using the given
`alpha` to determine what is transparent, writing modified copies of the images
as .png files to the directory given by `-D <dir>`.
