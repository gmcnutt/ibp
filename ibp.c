#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "str.h"


struct args {
        char **filenames;
        char *destdir;
        int filecount;
        bool crop, debug, sort;
};

struct imageinfo {
        SDL_Surface *image;
        char *filename;
};

/**
 * Print a command-line usage message.
 */
static void print_usage(void)
{
        printf("Usage:  ibp [options] <files>\n"
               "Options: \n"
               "    -c: crop transparency\n"
               "    -d: debug\n"
               "    -h:	help\n"
               "    -s: sort list output by image size\n"
               "    -D <dir>: write modified images as .png here\n"
               "The default is to list the image attributes.\n"
                );
}


/**
 * Parse command-line args.
 */
static void parse_args(int argc, char **argv, struct args *args)
{
        int c = 0;

        memset(args, 0, sizeof(*args));

        while ((c = getopt(argc, argv, "D:cdhs")) != -1) {
                switch (c) {
                case 'c':
                        args->crop = true;
                        break;
                case 'd':
                        args->debug = true;
                        break;
                case 'h':
                        print_usage();
                        exit(0);
                case 's':
                        args->sort = true;
                        break;
                case 'D':
                        args->destdir = optarg;
                        break;
                case '?':
                default:
                        print_usage();
                        exit(-1);
                        break;
                }
        }

        if (args->crop && !args->destdir) {
                printf("The -c option requires -D.");
                print_usage();
                exit(-1);
        }

        if (optind < argc) {
                args->filecount = argc - optind;
                args->filenames = &argv[optind];
        } else {
                print_usage();
                exit(-1);
        }
}

/**
 * Compare `a` to `b` for sorting.
 *
 * Both `a` and `b` should be pointers to `struct imageinfo` objects.
 *
 * Returns
 *     -1 if a < b
 *      0 if a == b
 *      1 if a > b
 */
static int cmp_imageinfo(const void *pa, const void *pb)
{
        struct imageinfo *a = (struct imageinfo*)pa,
                *b = (struct imageinfo*)pb;
        return (a->image->w * a->image->h) - (b->image->w * b->image->h);
}

/**
 * Print info about image to stdout.
 */
static void dump_imageinfo(struct imageinfo *info)
{
        SDL_PixelFormat *format = info->image->format;

        /* Basic info */
        printf(
                "%5d %5d %5d",
                info->image->w,
                info->image->h,
                info->image->pitch
                );

        /* Pixel format info */
        printf(" %28s", SDL_GetPixelFormatName(format->format));


        /* File info */
        printf(" %s\n", info->filename);
}

/**
 * Crop off any transparency on the borders.
 */
static SDL_Surface *crop(SDL_Surface *image)
{
        return image; /* TODO */
}

/**
 * Save the image as .png to the directory.
 */
static void save(struct imageinfo *info, const char *destdir)
{
        char *last, *filename = info->filename;

        /* Get the filename part of the path. */
        while ((last = strchr(filename, '/'))) {
                filename = last + 1;
        }

        /* Replace the suffix with .png */
        last = strchr(filename, '.');
        if (last) {
                *last = '\0';
        }

        /* Build a new path to the dest dir. */
        filename = str_printf("%s/%s.png", destdir, filename);
        IMG_SavePNG(info->image, filename);
        str_deref(filename);

        if (last) {
                *last = '.';
        }
}

int main(int argc, char **argv)
{
        struct args args;
        struct imageinfo *images;
        int count = 0;


        parse_args(argc, argv, &args);

        /* Init SDL */
        if (SDL_Init(SDL_INIT_VIDEO)) {
                printf("SDL_Init: %s\n", SDL_GetError());
                return -1;
        }

        /* Cleanup SDL on exit. */
        atexit(SDL_Quit);

        /* Allocate our image table */
        if (args.filecount) {
                if (! (images = calloc(args.filecount, sizeof(images[0])))) {
                        perror("calloc");
                        return -1;
                }
        }

        /* Load the image table */
        for (int i = 0; i < args.filecount; i++) {
                SDL_Surface *image;

                if (! (image = IMG_Load(args.filenames[i]))) {
                        if (args.debug) {
                                printf("%s: %s\n", args.filenames[i],
                                       SDL_GetError());
                        }
                        continue;
                }

                images[count].image = image;
                images[count].filename = args.filenames[i];
                count++;
        }

        if (args.crop) {
                for (int i = 0; i < count; i++) {
                        crop(images[i].image);
                }
        }

        if (args.destdir) {
                for (int i = 0; i < count; i++) {
                        save(&images[i], args.destdir);
                }
        }

        if (args.sort) {
                qsort(images, count, sizeof(images[0]), cmp_imageinfo);
        }

        /* Print header */
        printf(
                "%5s %5s %5s %28s %s\n",
                "w",
                "h",
                "pitch",
                "pixfmt",
                "name"
                );

        /* Print image info for all images */
        for (int i = 0; i < count; i++) {
                dump_imageinfo(&images[i]);
        }
}
