#include <assert.h>
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
        size_t size;
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
static int compare(const void *pa, const void *pb)
{
        struct imageinfo *a = (struct imageinfo*)pa,
                *b = (struct imageinfo*)pb;
        return a->size - b->size;
}

/**
 * Print info about image to stdout.
 */
static void dump(struct imageinfo *info)
{
        SDL_PixelFormat *format = info->image->format;
        const char *fmtname = SDL_GetPixelFormatName(format->format);

        /* Basic info */
        printf(
                "%5d %5d %5d %10zu",
                info->image->w,
                info->image->h,
                info->image->pitch,
                info->size
                );

        /* Pixel format info */
        printf(" %10s", &fmtname[16]);


        /* File info */
        printf(" %s\n", info->filename);
}

/**
 * Convert the image to a supported format.
 */
static void normalize(struct imageinfo *info)
{
        if (info->image->format->format == SDL_PIXELFORMAT_ABGR8888) {
                return;
        }

        SDL_Surface *original = info->image;
        info->image = SDL_ConvertSurfaceFormat(
                original,
                SDL_PIXELFORMAT_ABGR8888,
                0
                );
        assert(info->image);

        SDL_FreeSurface(original);
}

/**
 * Crop off any transparency on the borders.
 */
static void crop(struct imageinfo *info)
{
        /* Assert image has a supported format. */
        assert(info->image->format->format == SDL_PIXELFORMAT_ABGR8888);

        /* TODO */
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

        /* Build a new path to the dest dir. Note that I am not freeing the
         * original info->filename because it is probably from the argv
         * vector. */
        info->filename = str_printf("%s/%s.png", destdir, filename);
        IMG_SavePNG(info->image, info->filename);

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
                images[count].size = image->w * image->h;
                count++;
        }

        if (args.crop) {
                for (int i = 0; i < count; i++) {
                        normalize(&images[i]);
                        crop(&images[i]);
                }
        }

        if (args.destdir) {
                for (int i = 0; i < count; i++) {
                        save(&images[i], args.destdir);
                }
        }

        if (args.sort) {
                qsort(images, count, sizeof(images[0]), compare);
        }

        /* Print header */
        printf(
                "%5s %5s %5s %10s %10s %s\n",
                "w",
                "h",
                "pitch",
                "size",
                "pixfmt",
                "name"
                );

        /* Print image info for all images */
        for (int i = 0; i < count; i++) {
                dump(&images[i]);
        }
}
