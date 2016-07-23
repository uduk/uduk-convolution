# uduk-convolution
Written in pure C with inline assembly, uduk-convolution is a great tool to tailor your guitar sounds to be far more realistic. Currently, it only supports mono WAV file. -- (beta version)

## Fast direct convolution of two audio sources
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
> pass -DUSE_SLOW_TURTLE_CONVOLUTION macro to gcc for slow turtle convolution

## Free Impulse Response
- (http://www.voxengo.com/impulses/)

## Requirements
- GCC (https://gcc.gnu.org/)
- OpenMP (http://openmp.org/wp/)
- Glib 2.0+ (http://ftp.gnome.org/pub/gnome/sources/glib/)
- libsndfile (http://www.mega-nerd.com/libsndfile/)

## TODO List
- Support stereo impulse response
- fix glitch

![alt text] (http://icons.iconarchive.com/icons/kearone/comicons/128/linux-icon.png)
