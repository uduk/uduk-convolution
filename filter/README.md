## UDUK Filter

Collection of filters gathered from the world wide web

## Note

The normalized transfer function of a first order all-pass filter is:

               s - 1 
        H(s) = -----
               s + 1
Using a bilinear z-transform (BZT) method, we arrive at a difference equation of:

        y(n) = A * x(n) + A * y(n-1) - x(n-1)
        
Where the coefficient A is described by:

            1 - wp    
        A = ------
            1 + wp
and:
        wp = (PI * freq) / Fs
        Fs = sampling rate    
