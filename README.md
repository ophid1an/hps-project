# Project for High Performance Systems course
## Parallel implementation of the HyperLogLog algorithm
### Results - AMD FX-8350 / Debian 4.9.0-6-amd64 / gcc 6.3.0
#### OpenMP
|32-bit integers|Registers|Threads|Time|Speedup|Percent Error|
|:---:|:---:|:---:|:---:|:---:|:---:|
|2^25|16384|1|1.211|1|0.226|
|2^25|16384|2|0.619|0|0.226|
|2^25|16384|3|0.431|0|0.226|
|2^25|16384|4|0.324|0|0.226|
|2^25|16384|5|0.328|0|0.226|
|2^25|16384|6|0.299|0|0.226|
|2^25|16384|7|0.268|0|0.226|
|2^25|16384|8|0.244|0|0.226|
|2^26|16384|1|2.634|1|0.279|
|2^26|16384|2|1.317|0|0.279|
|2^26|16384|3|0.892|0|0.279|
|2^26|16384|4|0.662|0|0.279|
|2^26|16384|5|0.629|0|0.279|
|2^26|16384|6|0.615|0|0.279|
|2^26|16384|7|0.555|0|0.279|
|2^26|16384|8|0.484|0|0.279|
|2^27|16384|1|4.843|1|0.454|
|2^27|16384|2|2.454|0|0.454|
|2^27|16384|3|1.632|0|0.454|
|2^27|16384|4|1.222|0|0.454|
|2^27|16384|5|1.198|0|0.454|
|2^27|16384|6|1.166|0|0.454|
|2^27|16384|7|1.086|0|0.454|
|2^27|16384|8|0.953|0|0.454|
|2^28|16384|1|9.719|1|1.591|
|2^28|16384|2|4.868|0|1.591|
|2^28|16384|3|3.288|0|1.591|
|2^28|16384|4|3.335|0|1.591|
|2^28|16384|5|2.528|0|1.591|
|2^28|16384|6|2.345|0|1.591|
|2^28|16384|7|2.130|0|1.591|
|2^28|16384|8|1.910|0|1.591|
|2^29|16384|1|19.448|1|0.534|
|2^29|16384|2|9.811|0|0.534|
|2^29|16384|3|6.580|0|0.534|
|2^29|16384|4|6.639|0|0.534|
|2^29|16384|5|5.138|0|0.534|
|2^29|16384|6|4.575|0|0.534|
|2^29|16384|7|4.312|0|0.534|
|2^29|16384|8|3.795|0|0.534|
|2^30|16384|1|42.590|1|0.647|
|2^30|16384|2|21.359|0|0.647|
|2^30|16384|3|14.149|0|0.647|
|2^30|16384|4|10.799|0|0.647|
|2^30|16384|5|9.513|0|0.647|
|2^30|16384|6|9.278|0|0.647|
|2^30|16384|7|8.492|0|0.647|
|2^30|16384|8|7.626|0|0.647|
### References
* [“HyperLogLog.” Wikipedia, April 3, 2018.](https://en.wikipedia.org/w/index.php?title=HyperLogLog&oldid=833994784)
* [Bozkus, Cem, and Basilio B. Fraguela. “Accelerating the HyperLogLog Cardinality Estimation Algorithm.” Scientific Programming 2017 (2017): 1–8. https://doi.org/10.1155/2017/2040865.
](biblio/2040865.pdf)
* [K. Kumar and S. Subash, “Approximate large multiset cardinality using map reduce,” Tech. Rep., Rochester Institute Of Technology, 2015.](biblio/report.pdf)
* [Heule, Stefan, Marc Nunkesser, and Alexander Hall. “HyperLogLog in Practice: Algorithmic Engineering of a State of the Art Cardinality Estimation Algorithm.” In Proceedings of the 16th International Conference on Extending Database Technology, 683–692. ACM, 2013.
](biblio/p683-heule.pdf)
* [Flajolet, Philippe, Éric Fusy, Olivier Gandouet, and Frédéric Meunier. “Hyperloglog: The Analysis of a near-Optimal Cardinality Estimation Algorithm.” In AofA: Analysis of Algorithms, 137–156. Discrete Mathematics and Theoretical Computer Science, 2007.
](biblio/FlFuGaMe07.pdf)
