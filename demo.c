#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
/*
static const char *SDL_PIXELFORMAT_STRINGS[] = {
        "UNKNOWN",
        "INDEX1LSB",
        "INDEX1MSB",
        "INDEX4LSB",
        "INDEX4MSB",
        "INDEX8",
        "RGB332",
        "RGB444",
        "RGB555",
        "BGR555",
        "ARGB4444",
        "RGBA4444",
        "ABGR4444",
        "BGRA4444",
        "ARGB1555",
        "RGBA5551",
        "ABGR1555",
        "BGRA5551",
        "RGB565",
        "BGR565",
        "RGB24",
        "BGR24",
        "RGB888",
        "RGBX8888",
        "BGR888",
        "BGRX8888",
        "ARGB8888",
        "RGBA8888",
        "ABGR8888",
        "BGRA8888",
        "ARGB2101010",
        "RGBA32",
        "ARGB32",
        "BGRA32",
        "ABGR32",
        "YV12",
        "IYUV",
        "YUY2",
        "UYVY",
        "YVYU",
        "NV12",
        "NV21"
};
*/
static const char *SDL_PIXELTYPE_STRINGS[] = {
        "UNKNOWN",
        "INDEX1",
        "INDEX4",
        "INDEX8",
        "PACKED8",
        "PACKED16",
        "PACKED32",
        "ARRAYU8",
        "ARRAYU16",
        "ARRAYU32",
        "ARRAYF16",
        "ARRAYF32",
};

static const char *SDL_BITMAPORDER_STRINGS[] = {"NONE", "4321", "1234"};

static const char *SDL_PACKEDORDER_STRINGS[] = {
        "NONE",
        "XRGB",
        "RGBX",
        "ARGB",
        "RGBA",
        "XBGR",
        "BGRX",
        "ABGR",
        "BGRA"
};

static const char *SDL_ARRAYORDER_STRINGS[] = {
        "NONE",
        "RGB",
        "RGBA",
        "ARGB",
        "BGR",
        "BGRA",
        "ABGR"
};

static const char *SDL_PACKEDLAYOUT_STRINGS[] = {
        "NONE",
        "332",
        "4444",
        "1555",
        "5551",
        "565",
        "8888",
        "2101010",
        "1010102"
};
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
        const char *type, *order, *layout;

        /* Basic info */
        printf(
                "%5d %5d %5d",
                info->image->w,
                info->image->h,
                info->image->pitch
                );

        /* Pixel format info */
        type = SDL_PIXELTYPE_STRINGS[SDL_PIXELTYPE(format->format)];
        order = "---";
        layout = "---";

        if (SDL_ISPIXELFORMAT_INDEXED(format->format)) {
                order = SDL_BITMAPORDER_STRINGS[SDL_PIXELORDER(format->format)];
        } else if (SDL_ISPIXELFORMAT_PACKED(format->format)) {
                order = SDL_PACKEDORDER_STRINGS[SDL_PIXELORDER(format->format)];
                layout = SDL_PACKEDLAYOUT_STRINGS[SDL_PIXELLAYOUT(format->format)];
        } else if (SDL_ISPIXELFORMAT_ARRAY(format->format)) {
                order = SDL_ARRAYORDER_STRINGS[SDL_PIXELORDER(format->format)];
                layout = SDL_PACKEDLAYOUT_STRINGS[SDL_PIXELLAYOUT(format->format)];
        };

        printf(" %10s %10s %10s", type, order, layout);
        printf(" %5d %5d", format->BitsPerPixel, format->BytesPerPixel);
        printf(" %5s", SDL_ISPIXELFORMAT_ALPHA(format->format) ? "y" : "n");
        printf(
                 " %08x %08x %08x %08x",
                 format->Rmask,
                 format->Gmask,
                 format->Bmask,
                 format->Amask
                );
        printf(" %10s", SDL_GetPixelFormatName(format->format));

        
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
                "%5s %5s %5s %10s %10s %10s %5s %5s %3s %8s %8s %8s %8s %10s %s\n",
                "w",
                "h",
                "pitch",
                "pxtype",
                "pxord",
                "pxlay",
                "pxbit",
                "pxbyt",
                "alpha",
                "rmask",
                "gmask",
                "bmask",
                "amask",
                "format",
                "name"
                );

        /* Print image info for all images */
        for (int i = 0; i < count; i++) {
                dump_imageinfo(&images[i]);
        }
}
