# uduk-convolution
Fast direct convolution of two audio sources. Uduk Convolution is a great tool to tailor your guitar sounds.

## Usage
$ uduk-convolution -i cabinet.wav -r input.wav -f -o output.wav

## Compiling

```
$ gcc -Wall -O2 -fopenmp -std=c99 uduk-convolution.c -o uduk-convolution `pkg-config --cflags --libs glib-2.0` -lsndfile
```

## Requirements
- GCC (https://gcc.gnu.org/)
- OpenMP (http://openmp.org/wp/)
- Glib 2.0+ (http://ftp.gnome.org/pub/gnome/sources/glib/)
- libsndfile (http://www.mega-nerd.com/libsndfile/)

![alt text] (http://icons.iconarchive.com/icons/kearone/comicons/128/linux-icon.png)
