# uduk-convolution
Fast convolution of two audio sources

## Usage
$ uduk-convolution -i cabinet.wav -r input.wav -f -o output.wav

## Compiling

```
$ gcc -Wall -O2 -fopenmp -std=c99 uduk-convolution.c -o uduk-convolution `pkg-config --cflags --libs glib-2.0` -lsndfile
```

## Requirements
- OpenMP (http://openmp.org/wp/)
- Glib 2.0+ (http://ftp.gnome.org/pub/gnome/sources/glib/)
- libsndfile (http://www.mega-nerd.com/libsndfile/)

![alt text] (http://icons.iconarchive.com/icons/kearone/comicons/128/linux-icon.png)
