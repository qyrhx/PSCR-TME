# TME5 Correction: Parallelization of a Ray Tracer

## Baseline Sequential

### Question 1
Extracted TME5.zip from Moodle, added to repo, committed and pushed.

Configured project with CMake as previous TMEs. No dependencies, all handmade.

### Question 2

Ran `build/TME5` and generated `spheres.bmp`.

Platform:
```
Void Linux
CPU: dual core Intel Celeron N4000, speed/min/max: 1710/800/2600 MHz
Kernel: 6.12.49_1 x86_64
Mem: 8GB
Storage: SSD
```

Temps/baseline choisi :
```
Ray tracer starting with output 'spheres.bmp', resolution 2000x2000, spheres 250, mode sequential
Total time 9335ms.
```

## With Manual Threads

### Question 3
Implemented `void renderThreadPerPixel(const Scene& scene, Image& img)` in Renderer.

Mesures:
```
Ca plante
```

### Question 4
Implemented `void renderThreadPerRow(const Scene& scene, Image& img)` in Renderer.

Mesures:
```
Ray tracer starting with output 'spheres.bmp', resolution 2000x2000, spheres 250, mode ThreadPerRow
Total time 5410ms.
```


### Question 5
Implemented `void renderThreadManual(const Scene& scene, Image& img, int nbthread)` in Renderer.

Mesures:
```
Ray tracer starting with output 'spheres.bmp', resolution 2000x2000, spheres 250, mode ThreadManual, threads 4
Total time 5159ms.
```


## With Thread Pool

### Question 6
Queue class: blocking by default, can switch to non-blocking.

### Question 7
Pool class: constructor with queue size, start, stop.
Job abstract class with virtual run().

### Question 8
PixelJob: derives from Job, captures ?TODO?

renderPoolPixel:

Mode "-m PoolPixel" with -n.

mesures

### Question 9
LineJob: derives from Job, captures TODO

renderPoolRow:

Mode "-m PoolRow -n nbthread".

mesures

### Question 10
Best:

## Bonus

### Question 11

pool supportant soumission de lambda.
