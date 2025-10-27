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
PixelJob: derives from Job, captures
```
const Scene& scene;
Image& img;
int x, y;
```

renderPoolPixel (capacite de Pool: 500):
```
λ> ./TME5 -m PoolPixel -n 6
Ray tracer starting with output 'spheres.bmp', resolution 2000x2000, spheres 250, mode PoolPixel, threads 6
Total time 9606ms.

λ> ./TME5 -m PoolPixel -n 8
Ray tracer starting with output 'spheres.bmp', resolution 2000x2000, spheres 250, mode PoolPixel, threads 8
Total time 8283ms.

λ> ./TME5 -m PoolPixel -n 10
Ray tracer starting with output 'spheres.bmp', resolution 2000x2000, spheres 250, mode PoolPixel, threads 10
Total time 10435ms.

λ> ./TME5 -m PoolPixel -n 12
Ray tracer starting with output 'spheres.bmp', resolution 2000x2000, spheres 250, mode PoolPixel, threads 12
Total time 9736ms.

λ> ./TME5 -m PoolPixel -n 16
Ray tracer starting with output 'spheres.bmp', resolution 2000x2000, spheres 250, mode PoolPixel, threads 16
Total time 21904ms.
```

### Question 9
LineJob: derives from Job, captures:
```
const Scene &scene;
Image &img;
int x;
```

renderPoolRow (capacite de Pool: 100)
Mesures:
```
λ> ./TME5 -m PoolRow -n 4
Ray tracer starting with output 'spheres.bmp', resolution 2000x2000, spheres 250, mode PoolRow, threads 4
Total time 5844ms.

λ> ./TME5 -m PoolRow -n 6
Ray tracer starting with output 'spheres.bmp', resolution 2000x2000, spheres 250, mode PoolRow, threads 6
Total time 5988ms.

λ> ./TME5 -m PoolRow -n 8 -o i.bmp
Ray tracer starting with output 'i.bmp', resolution 2000x2000, spheres 250, mode PoolRow, threads 8
Total time 5454ms.

λ> ./TME5 -m PoolRow -n 16 -o i.bmp
Ray tracer starting with output 'i.bmp', resolution 2000x2000, spheres 250, mode PoolRow, threads 16
Total time 5859ms.

```

### Question 10
Best:

*Note: Je n'ai que 2 coeurs sur ma machine*

Pour *PoolPixel* 8 threads, pour *PoolRow* aussi 8 threads, en general *PoolRow* est mieux.

## Bonus

### Question 11

pool supportant soumission de lambda
