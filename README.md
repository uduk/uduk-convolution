# uduk-convolution
Written in pure C with inline assembly (intel), uduk-convolution is a great tool to tailor your guitar sounds to be far more realistic. Currently, it only supports WAV both mono and stereo (PCM 16 bit) file. -- (beta version)

## Fast direct linear convolution of two audio sources
$ ./uduk-convolution input.wav cabinet.wav output.wav

## Compile
### Linux

```
$ gcc -Wall -O2 -fopenmp -lsndfile -std=c99 uduk-convolution.c -o uduk-convolution
```

## Macro
Default compilation without any macro is UDUK Zuper-Fast IS™

> pass macro to gcc:

> -DUSE_SLOW_TURTLE_CONVOLUTION for slow turtle convolution

## Theory
- (http://www.indiana.edu/~emusic/etext/synthesis/chapter4_convolution.shtml)
- (http://allsignalprocessing.com/convolution-of-signals-why/)
- (http://www.voxengo.com/impulses/)
- (http://www.iowahills.com/A4IIRBilinearTransform.html)
- (http://www.barrgroup.com/Embedded-Systems/How-To/Digital-Filters-FIR-IIR)
- (http://www.musicdsp.org/)
- (http://www.mathworks.com/examples/signal/)

## Requirements
- GCC (https://gcc.gnu.org/)
- pkg-config (https://www.freedesktop.org/wiki/Software/pkg-config/)
- OpenMP (http://openmp.org/wp/)
- libsndfile (http://www.mega-nerd.com/libsndfile/)

## TODO List
- tweaks++

## LICENSE
NO License, FREE as an AIR

## E-mail
Feel free to drop me some bugs and issues iherlambang@gmail.com

![alt text] (http://icons.iconarchive.com/icons/kearone/comicons/128/linux-icon.png)
