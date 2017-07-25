# Mandlebrot

This is a mandlebrot renderer written in C++ using boost multiprecision with GMP as the backend. It is written with deep 
zoom video generation in mind, but does rendering of single frames just fine.

It does support multithreading but more work is required because GMP doesn't like that very much. Spawning more than 1 thread
tanks performance, I think I need to split it into multiple processes or something like that, but I'm not entirely sure.

Currently I've yet to implement any major optimizations to the rendering technique. Next up is probably k-tree(or whatever 
it was called) optimization, but that only really has an impact on frames that have a lot of empty areas. I also want to move
the multithreading to make threads colaborate on one frame instead of them doing their own thing. That should speed up single
image renders significantly as well.

The codebase also needs a lot of cleanup because I've written this a late nights where I didn't care a lot about structure. If
you want to see cleaner code take a look at any other project I've done in the past :)
