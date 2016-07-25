# UDUK Effects

## Tremolo
```
$ gcc -Wall -O2 -fopenmp -std=c99 uduk-tremolo.c -o uduk-tremolo -lsndfile
```

## Usage
$ ./uduk-tremolo input.wav output.wav

Average processing time on 61 MB WAVE audio, Microsoft PCM, 16 bit, stereo 44100 Hz

| Invocation    | Time      | 
| ------------- |:---------:| 
| real          | 0m1.739s  |
| user          | 0m0.982s  |
| sys           | 0m0.126s  |

	
	
	  