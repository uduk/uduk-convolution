# uduk-convolution
Written in pure C with inline assembly, uduk-convolution is a great tool to tailor your guitar sounds to be far more realistic. Currently, it only supports WAV both mono and stereo (PCM 16 bit) file. -- (beta version)

## Fast direct linear convolution of two audio sources
$ ./uduk-convolution input.wav cabinet.wav output.wav

## Compile
### Linux

```
$ gcc -Wall -O2 -fopenmp -std=c99 uduk-convolution.c -o uduk-convolution `pkg-config --cflags --libs glib-2.0` -lsndfile
```

### Mac

```
$ clang-omp -Wall -O2 -fopenmp -std=c99 uduk-convolution.c -o uduk-convolution `pkg-config --cflags --libs glib-2.0` -lsndfile
```
> pass macro to gcc

> default without any macro is linasm

> -DUSE_UDUK_INLINE_CONVOLUTION for average turtle-ish UDUK

> -DUSE_SLOW_TURTLE_CONVOLUTION for slow turtle convolution

## Free Impulse Response
- (http://www.indiana.edu/~emusic/etext/synthesis/chapter4_convolution.shtml)
- (http://www.voxengo.com/impulses/)

## Requirements
- GCC (https://gcc.gnu.org/)
- OpenMP (http://openmp.org/wp/)
- Linasm (http://linasm.sourceforge.net/)
- Glib 2.0+ (http://ftp.gnome.org/pub/gnome/sources/glib/)
- libsndfile (http://www.mega-nerd.com/libsndfile/)

## TODO List
- tweaks++

## E-mail
Feel free to drop me some bugs and issues iherlambang@gmail.com

![alt text] (http://icons.iconarchive.com/icons/kearone/comicons/128/linux-icon.png)
