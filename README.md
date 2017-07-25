# Mandlebrot

This is a mandlebrot renderer written in C++ using boost multiprecision with GMP as the backend.
It does support multithreading but more work is required because GMP doesn't like that very much. Spawning more than 1 thread
tanks performance, I think I need to split it into multiple processes or something like that, but I'm not entirely sure.

The codebase also needs a lot of cleanup because I've written this a late nights where I didn't care a lot about structure.
If you want to see cleaner code take a look at any other project I've done in the past :)
