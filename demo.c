#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


struct args {
        char **filenames;
        int filecount;
        bool sort, debug;
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
        printf("Usage:  demo [options] <filename>\n"
               "Options: \n"
               "    -h:	help\n"
                );
}


/**
 * Parse command-line args.
 */
static void parse_args(int argc, char **argv, struct args *args)
{
        int c = 0;

        memset(args, 0, sizeof(*args));

        while ((c = getopt(argc, argv, "dhs")) != -1) {
                switch (c) {
                case 'd':
                        args->debug = true;
                        break;
                case 'h':
                        print_usage();
                        exit(0);
                case 's':
                        args->sort = true;
                        break;
                case '?':
                default:
                        print_usage();
                        exit(-1);
                        break;
                }
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

static void dump_imageinfo(struct imageinfo *info)
{
        printf(
                "%5d %5d %5d %s\n",
                info->image->w,
                info->image->h,
                info->image->pitch,
                info->filename
                );
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

        if (args.sort) {
                qsort(images, count, sizeof(images[0]), cmp_imageinfo);
        }
        
        /* Print header */
        printf("%5s %5s %5s %s\n", "w", "h", "pitch", "name");

        /* Print image info for all images */
        for (int i = 0; i < count; i++) {
                dump_imageinfo(&images[i]);
        }
}
