#include "allheads.h"


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

/*
void sdl_dump_pixel_format(const SDL_PixelFormat *format)
{
        printf(" format: 0x%x\n", format->format);
        if (format->palette) {
                printf(" palette: %d colors\n", format->palette->ncolors);
        }
        printf(" BitsPerPixel: %d\n", format->BitsPerPixel);
        printf(" BytesPerPixel: %d\n", format->BytesPerPixel);
        printf(" RGBA masks: %08x %08x %08x %08x\n", format->Rmask,
               format->Gmask, format->Bmask, format->Amask);
}
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
