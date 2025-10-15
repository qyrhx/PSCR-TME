# TME4 Answers

Tracer vos expériences et conclusions dans ce fichier.

Le contenu est indicatif, c'est simplement la copie rabotée d'une IA, utilisée pour tester une version de l'énoncé.
On a coupé ses réponses complètes (et souvent imprécises voire carrément fausses, deadlocks etc... en Oct 2025 les LLM ont encore beaucoup de mal sur ces questions, qui demandent du factuel et des mesures, et ont de fortes tendances à inventer).
Cependant on a laissé des indications en particulier des invocations de l'outil possibles, à adapter à votre code.

## Question 1: Baseline sequential

### Measurements (Release mode)

**Resize + pipe mode:**
```
SSD

> ./build/TME4 -m resize -i input_images -o output_images
Image resizer starting with input folder '../input_images', output folder '../output_images', mode 'resize', nthreads 4
Thread 139918895626240 (main): 11018 ms CPU
Total runtime (wall clock): 12648 ms
Memory usage: Resident: 69.9 MB, Peak: 169 MB
Total CPU time across all threads: 11018 ms


> ./build/TME4 -m pipe -i input_images -o output_images
Image resizer starting with input folder '../input_images', output folder '../output_images', mode 'pipe', nthreads 4
Thread 140353728411328 (treatImage): 10824 ms CPU
Thread 140353742329856 (main): 62 ms CPU
Total runtime (wall clock): 12585 ms
Memory usage: Resident: 70.1 MB, Peak: 169 MB
Total CPU time across all threads: 10886 ms
```

Total runtime vs CPU time -> par exemple quand on attend le HHD pour charger la memoire, CPU ne fait rien
Pic de RAM -> a cause du chargement de l'image en totalite?

## Question 2: Steps identification

Recherche des images:
```
> mode resize
pr::findImageFiles(opts.inputFolder, [&](const std::filesystem::path &file) { ---> recherche
      QImage original = pr::loadImage(file); ---> chargement
      if (!original.isNull()) {
        QImage resized = pr::resizeImage(original); ---> redimensionnement
        std::filesystem::path outputFile = opts.outputFolder / file.filename();
        pr::saveImage(resized, outputFile); ---> sauvegarde
      }
    });
> mode pipe
pr::findImageFiles(opts.inputFolder,
                       [&](const std::filesystem::path &file) { fileQueue.push(file); }); ---> recherche

-- Tasks.cpp
void treatImage(FileQueue &fileQueue, const std::filesystem::path &outputFolder) {
  // measure CPU time in this thread
  pr::thread_timer timer;

  while (true) {
    std::filesystem::path file = fileQueue.pop();
    if (file == pr::FILE_POISON)
      break; // poison pill
    QImage original = pr::loadImage(file); ---> chargement
    if (!original.isNull()) {
      QImage resized = pr::resizeImage(original); ---> redimensionnement
      std::filesystem::path outputFile = outputFolder / file.filename();
      pr::saveImage(resized, outputFile); ---> sauvegarde
    }
  }

    // ...
}
```

I/O-bound: chargement, sauvegarde, recherche
CPU-bound: redimensionnement

parallelisable a priori?
le redimensionnement peut etre?

## Question 3: BoundedBlockingQueue analysis

- Oui, il supporte plusieurs producteurs et plusieurs consommateurs
- un appel est bloque si on fait push et la queue est pleine,
  ou bien si on fait pop alors qu'il est vide
- la lambda de cv_.wait test la condition dedant a chaque fois qu'il est reveille (avec notify)
  si la condition est F, il bloque, sinon il continue l'execution
  la lambda capture le pointeur `this` par *copie*
- l'alternatif est une boucle while, mais cette forme est plus idiomatique


## Question 4: Pipe mode study

FILE_POISON: est definie dans `Tasks.h`, il est utilise pour marquer la fin du traitement
apres qu'on push tout les fichier a traiter

Order/invert:
- l'etape 1 (construire la file) doit rester ou elle est, puisque tous ce qui vient apres en depend
- l'etape 2 (init worker) et 3 (chercher les fichiers) ne peut pas etre inverser, sinon, la queue
  chargera un certain nb de fichiers selon sa capacite, et puisque il n y a pas des workers pour les
  consommer, il restera bloque
- l'etape 4 (inserer FILE\_POISON) doit rester ou elle est, sinon on va inserer FILE_POISON au milieux,
  et les fichiers qui viennent apres ne seront pas traite
- l'etape 5 (worker.join) doit rester ou elle est


## Question 5: Multi-thread pipe_mt

Implement pipe_mt mode with multiple worker threads.

For termination, ... poison pills...

Measurements:
- N=1:
```
Thread 140436301674176 (treatImage): 10845 ms CPU
Thread 140436305102848 (main): 64 ms CPU
Total runtime (wall clock): 12432 ms
Memory usage: Resident: 70.4 MB, Peak: 170 MB
Total CPU time across all threads: 10909 ms
```
- N=2:
```
Thread 139869502301888 (treatImage): 5397 ms CPU
Thread 139869510694592 (treatImage): 5548 ms CPU
Thread 139869517162496 (main): 64 ms CPU
Total runtime (wall clock): 7974 ms
Memory usage: Resident: 98.1 MB, Peak: 256 MB
Total CPU time across all threads: 11009 ms
```
- N=4:
```
Thread 139649301333696 (treatImage): 2660 ms CPU
Thread 139649318119104 (treatImage): 2656 ms CPU
Thread 139649309726400 (treatImage): 2769 ms CPU
Thread 139649326511808 (treatImage): 2959 ms CPU
Thread 139649332897792 (main): 64 ms CPU
Total runtime (wall clock): 8591 ms
Memory usage: Resident: 141 MB, Peak: 373 MB
Total CPU time across all threads: 11108 ms
```
- N=8:
```
Thread 139684661405376 (treatImage): 1250 ms CPU
Thread 139684703368896 (treatImage): 1436 ms CPU
Thread 139684669798080 (treatImage): 1245 ms CPU
Thread 139684686583488 (treatImage): 1394 ms CPU
Thread 139684653012672 (treatImage): 1408 ms CPU
Thread 139684644619968 (treatImage): 1459 ms CPU
Thread 139684678190784 (treatImage): 1505 ms CPU
Thread 139684694976192 (treatImage): 1423 ms CPU
Thread 139684717287424 (main): 64 ms CPU
Total runtime (wall clock): 8587 ms
Memory usage: Resident: 223 MB, Peak: 627 MB
Total CPU time across all threads: 11184 ms
```

Best: `N=2` (nb de coeurs de mon cpu)

## Question 6: TaskData struct

```cpp
struct TaskData {
  QImage img{};
  std::filesystem::path filepath{};
};
```

Fields:
`QImage`: pour l'image a traiter
`std::filesystem::path`: parce que `QImage` ne garde pas le chemin de l'image

J'ai utilise un stockage directe.

`TASK_POISON`: nullptr pour un pointeur, sinon on construit un QImage
avec le constructeur par defaut (cad: sans passer un nom de fichier) qui aura
`QImage().isNull() = true`

## Question 7: ImageTaskQueue typing

pointers vs values

Pour les pointeurs: il faut manuellement faire delete (solution: smart pointers)
Stockage direct:

Choose BoundedBlockingQueue<TaskData> as consequence

## Question 8: Pipeline functions

Implement reader, resizer, saver in Tasks.cpp.

mt_pipeline mode: Creates threads for each stage, with configurable numbers.

Termination: Main pushes the appropriate number of poisons after joining the previous stage.

Measurements:
```
> 2 threads: nb de coeurs de mon cpu
Image resizer starting with input folder '../input_images', output folder 'mt_pipeline_output', mode 'mt_pipeline', nthreads 2
Thread 140136085964480 (reader): 4767 ms CPU
Thread 140136094357184 (saver): 634 ms CPU
Thread 140136102749888 (resizer): 149 ms CPU
Thread 140136111142592 (reader): 4886 ms CPU
Thread 140136077571776 (resizer): 115 ms CPU
Thread 140136069179072 (saver): 576 ms CPU
Thread 140136114661376 (main): 64 ms CPU
Total runtime (wall clock): 9097 ms
Memory usage: Resident: 177 MB, Peak: 262 MB
Total CPU time across all threads: 11191 ms


Image resizer starting with input folder '../input_images', output folder 'mt_pipeline_output', mode 'mt_pipeline', nthreads 4
Thread 140318747891392 (reader): 2247 ms CPU
Thread 140318739498688 (resizer): 47 ms CPU
Thread 140318756284096 (saver): 322 ms CPU
Thread 140318764676800 (resizer): 43 ms CPU
Thread 140318705927872 (saver): 281 ms CPU
Thread 140318722713280 (reader): 2385 ms CPU
Thread 140318773069504 (reader): 2539 ms CPU
Thread 140318789854912 (resizer): 61 ms CPU
Thread 140318781462208 (saver): 262 ms CPU
Thread 140318798247616 (reader): 2527 ms CPU
Thread 140318714320576 (resizer): 90 ms CPU
Thread 140318731105984 (saver): 351 ms CPU
Thread 140318804617216 (main): 64 ms CPU
Total runtime (wall clock): 8527 ms
Memory usage: Resident: 240 MB, Peak: 373 MB
Total CPU time across all threads: 11219 ms
```


## Question 9: Configurable parallelism

Added nbread, nbresize, nbwrite options.


Timings:
- 1/1/1 (default):
```
Thread 140651609978560 (reader): 9809 ms CPU
Thread 140651601585856 (resizer): 233 ms CPU
Thread 140651593193152 (saver): 1189 ms CPU
Thread 140651621812224 (main): 66 ms CPU
Total runtime (wall clock): 11653 ms
Memory usage: Resident: 125 MB, Peak: 204 MB
Total CPU time across all threads: 11297 ms
```
- 1/2/1:
```
Thread 140110569928384 (reader): 9852 ms CPU
Thread 140110553142976 (resizer): 118 ms CPU
Thread 140110561535680 (resizer): 111 ms CPU
Thread 140110544750272 (saver): 1191 ms CPU
Thread 140110594099200 (main): 68 ms CPU
Total runtime (wall clock): 12323 ms
Memory usage: Resident: 127 MB, Peak: 216 MB
Total CPU time across all threads: 11340 ms
```

- 2/1/1:
```
Thread 140367175345856 (reader): 4827 ms CPU
Thread 140367183738560 (reader): 5040 ms CPU
Thread 140367166953152 (resizer): 224 ms CPU
Thread 140367158560448 (saver): 1210 ms CPU
Thread 140367187445760 (main): 66 ms CPU
Total runtime (wall clock): 8987 ms
Memory usage: Resident: 160 MB, Peak: 249 MB
Total CPU time across all threads: 11367 ms

```

- 1/1/2
```
Thread 140496768857792 (reader): 9824 ms CPU
Thread 140496760465088 (resizer): 225 ms CPU
Thread 140496752072384 (saver): 547 ms CPU
Thread 140496743679680 (saver): 654 ms CPU
Thread 140496775620608 (main): 64 ms CPU
Total runtime (wall clock): 12237 ms
Memory usage: Resident: 117 MB, Peak: 208 MB
Total CPU time across all threads: 11314 ms
```

Best config: 2/1/1
Interprétation: le bottleneck c'est le read apparement

- Ma reponse a la Q2 a ete fausse; j'ai oublie que dans le reading on fait aussi
  une conversion image -> bitmap, et cela est une operation non triviale en termes de CPU

## Question 10: Queue sizes impact


With size 1:
```
Thread 140398280304320 (treatImage): 5645 ms CPU
Thread 140398288697024 (treatImage): 5889 ms CPU
Thread 140398295287808 (main): 64 ms CPU
Total runtime (wall clock): 9455 ms
Memory usage: Resident: 98.1 MB, Peak: 231 MB
Total CPU time across all threads: 11598 ms
```

With size 100:
```
Thread 140256881927872 (treatImage): 5314 ms CPU
Thread 140256890320576 (treatImage): 5669 ms CPU
Thread 140256896903168 (main): 63 ms CPU
Total runtime (wall clock): 8617 ms
Memory usage: Resident: 98.1 MB, Peak: 234 MB
Total CPU time across all threads: 11046 ms
```

impact

Complexity:


## Question 11: BoundedBlockingQueueBytes

Implemented with byte limit.

mesures

## Question 12: Why important

Always allow push if current_bytes == 0, ...

Fairness: ...

## Bonus
