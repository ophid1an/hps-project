# Project for the High Performance Systems course
## Parallel implementation of the HyperLogLog++ algorithm
### Description
Fills an array of 2^**p** 32-bit unsigned integers with random values and counts the distinct numbers using qsort(). It then approximates this count by using an implementation (without the bias corrections) of the HyperLogLog++ (Heule et al. 2013) algorithm using 64-bit hashes produced by [xxHash](https://github.com/Cyan4973/xxHash).
### Usage
Build with `make`, then `./bin/hll [p [b [s]]]`, where 2^**p** is the length of the array (default: 27 = 134217728 32-bit integers occupying 512MiB), 2^**b** [4..16] the number of 8-bit "registers" the algorithm will use (default: 14 = 16384 registers) and **s** the unsigned integer to seed the RNG used by rand() (default: 1). The count of distinct numbers has been precalculated for **s** = 1 and **p** = [0..30].
### Results - AMD FX-8350 / Debian 4.9.0-6-amd64 / gcc 6.3.0
#### OpenMP / b = 14 (16384 "registers"), s = 1
|32-bit integers|Threads|Time|Speedup|Percent Error|
|:---:|:---:|:---:|:---:|:---:|
|2^25|1|1.211|1|0.226|
|2^25|2|0.619|N/A|0.226|
|2^25|3|0.431|N/A|0.226|
|2^25|4|0.324|N/A|0.226|
|2^25|5|0.328|N/A|0.226|
|2^25|6|0.299|N/A|0.226|
|2^25|7|0.268|N/A|0.226|
|2^25|8|0.244|N/A|0.226|
|2^26|1|2.634|1|0.279|
|2^26|2|1.317|N/A|0.279|
|2^26|3|0.892|N/A|0.279|
|2^26|4|0.662|N/A|0.279|
|2^26|5|0.629|N/A|0.279|
|2^26|6|0.615|N/A|0.279|
|2^26|7|0.555|N/A|0.279|
|2^26|8|0.484|N/A|0.279|
|2^27|1|4.843|1|0.454|
|2^27|2|2.454|N/A|0.454|
|2^27|3|1.632|N/A|0.454|
|2^27|4|1.222|N/A|0.454|
|2^27|5|1.198|N/A|0.454|
|2^27|6|1.166|N/A|0.454|
|2^27|7|1.086|N/A|0.454|
|2^27|8|0.953|N/A|0.454|
|2^28|1|9.719|1|1.591|
|2^28|2|4.868|N/A|1.591|
|2^28|3|3.288|N/A|1.591|
|2^28|4|3.335|N/A|1.591|
|2^28|5|2.528|N/A|1.591|
|2^28|6|2.345|N/A|1.591|
|2^28|7|2.130|N/A|1.591|
|2^28|8|1.910|N/A|1.591|
|2^29|1|19.448|1|0.534|
|2^29|2|9.811|N/A|0.534|
|2^29|3|6.580|N/A|0.534|
|2^29|4|6.639|N/A|0.534|
|2^29|5|5.138|N/A|0.534|
|2^29|6|4.575|N/A|0.534|
|2^29|7|4.312|N/A|0.534|
|2^29|8|3.795|N/A|0.534|
|2^30|1|42.590|1|0.647|
|2^30|2|21.359|N/A|0.647|
|2^30|3|14.149|N/A|0.647|
|2^30|4|10.799|N/A|0.647|
|2^30|5|9.513|N/A|0.647|
|2^30|6|9.278|N/A|0.647|
|2^30|7|8.492|N/A|0.647|
|2^30|8|7.626|N/A|0.647|
### References
* [“HyperLogLog.” Wikipedia, April 3, 2018.](https://en.wikipedia.org/w/index.php?title=HyperLogLog&oldid=833994784)
* [Bozkus, Cem, and Basilio B. Fraguela. “Accelerating the HyperLogLog Cardinality Estimation Algorithm.” Scientific Programming 2017 (2017): 1–8. https://doi.org/10.1155/2017/2040865.
](biblio/2040865.pdf)
* [K. Kumar and S. Subash, “Approximate large multiset cardinality using map reduce,” Tech. Rep., Rochester Institute Of Technology, 2015.](biblio/report.pdf)
* [Heule, Stefan, Marc Nunkesser, and Alexander Hall. “HyperLogLog in Practice: Algorithmic Engineering of a State of the Art Cardinality Estimation Algorithm.” In Proceedings of the 16th International Conference on Extending Database Technology, 683–692. ACM, 2013.
](biblio/p683-heule.pdf)
* [Flajolet, Philippe, Éric Fusy, Olivier Gandouet, and Frédéric Meunier. “Hyperloglog: The Analysis of a near-Optimal Cardinality Estimation Algorithm.” In AofA: Analysis of Algorithms, 137–156. Discrete Mathematics and Theoretical Computer Science, 2007.
](biblio/FlFuGaMe07.pdf)
* [Collet, Yann. xxHash: Extremely Fast Non-Cryptographic Hash Algorithm. C, 2018.](https://github.com/Cyan4973/xxHash)
