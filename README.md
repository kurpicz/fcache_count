# fcache_count

## What is it?
This is a small program monitoring the file cache.
It can also disable the file cache.
We have only tested this on Linux but other systems implementing POSIX should also work.
Please notice that this is just a side project we needed for some tests, hence the the program may not contain every feature you want.

## Installation and Usage
First clone the repository.
```
git clone https://github.com/kurpicz/fcache_count.git
cd fcache_count
```
If you want to *disable* the file cache you need to compile `fcache_count.cpp` with `FCACHE_DISABLED` defined (either via your compiler or directly in the code).
Then compile your code (adding `fcache_count.o`) and linking the *dl* library (`-ldl`).

For more details (especially regarding the logging) please look at the example we provide in the folder `test-fcache-profile`.

## Credits
We took some code from [nocache](https://github.com/feh/nocache) (please notice the copyright in the code and the license file).
This project was also heavily influenced by [malloc_count](https://github.com/bingmann/malloc_count).
