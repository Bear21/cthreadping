# cthreadping
## Summery
Tests the time taken for logical threads to communicate on AMD64 Windows systems. This is to find interesting characteristics about how a particular cpu functions. Which in turn helps software developers better understand how to multithread applications to yield beneficial results.

## Usage
For best results run as administrator which will allow the application to run in 'realtime' priority.
1. Launch from commandline ./cthreadping.exe > result.csv
1. Just run it, the text output is csv format.

## ping test
Due to the latency being so low to get an accurate result the test must be re-ran a lot. this is due to the high-performance counter on cpus typically being less than 1ghz, so each 'tick' in the cpus clock is normally separated more than 10 ns*, so the latency needs to be calculated and approximated. it's also done twice, once in each direction

Intel Core i7 6700k clock runs at 3914069hz (255.5ns per tick) and we need to use it to measure something that happens in less than 10ns.
## Cache testing
The verbose cache testing is done n^2 where n is logical processor count in an attempt to weed out any quirks that maybe hidden on a system. E.g. ryzen ccx>ccx latency 

## Coming soon
*  Better context switch duration measurement.

## System requirements
* Windows x64
* 256MB free memory
* At least two threads

# FAQ

### Q: My cache latencies are sometimes consistently one value between two cores but sometimes they change when I run this tool 10 minutes later. Is this tool broken?
A: No. From my experiments with my 6700k as shown in the results folder the real latencies between two physical cores seem to change. I speculate that if the linking of the cores for atomic operations become unsynchronised it can add/remove ~7ns (value derived from my 6700k)
