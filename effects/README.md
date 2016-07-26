# UDUK Effects

All parameters are hardcoded, written in a rush - Under development.

## Compile
```
$ gcc -Wall -O2 -fopenmp -std=c99 uduk-$1.c -o uduk-$1 -lsndfile
```

## Usage
$ ./uduk-($effects) input.wav output.wav

Average processing time on 61 MB WAVE audio, Microsoft PCM, 16 bit, stereo 44100 Hz

| Invocation    | Time      | 
| ------------- |:---------:| 
| real          | 0m1.739s  |
| user          | 0m0.982s  |
| sys           | 0m0.126s  |

## LICENSE

```
/*
 * UDUK: All Left Reserved.
 *
 * No Licensed under the UDUK license (the "Free As An AIR License").  
 * You may use this file without any exception and/or restriction.
 *
 */
```	
	
	  
