# Questions - TME 3 : Threads

Instructions : copiez vos réponses dans ce fichier (sous la question correspondante). A la fin de la séance, commitez vos réponses.

## Question 1.

```
cd build-release && ./TME3 ../WarAndPeace.txt freqstd && ./TME3 ../WarAndPeace.txt freqstdf && ./TME3 ../WarAndPeace.txt freq && check.sh *.freq

TRACE:
Preparing to parse ../WarAndPeace.txt (mode=freqstd N=4), containing 3235342 bytes
Total runtime (wall clock) : 474 ms
Preparing to parse ../WarAndPeace.txt (mode=freqstdf N=4), containing 3235342 bytes
Total runtime (wall clock) : 445 ms
Preparing to parse ../WarAndPeace.txt (mode=freq N=4), containing 3235342 bytes
Total runtime (wall clock) : 486 ms

All files are identical
```

## Question 2.

* freqstdf

start: 0

end: `file_size`

Code des lambdas :
```
[&](const std::string &word) {
  total_words++;
  um[word]++;
}
```

Accès identifiés :
```
total_words -> ref
um -> ref
```

* freq

start: 0

end: `file_size`

Code des lambdas :
```
[&](const std::string &word) {
  total_words++;
  hm.incrementFrequency(word);
}
```

Accès identifiés:
```
total_words -> ref
hm -> ref
```

## Question 3.
Trace:
```
Preparing to parse ../WarAndPeace.txt (mode=partition N=4), containing 3235342 bytes
Total runtime (wall clock) : 485 ms
```

Resultat identique aux resultats des autres modes, teste avec
```
../check.sh freq.freq freqstdf.freq freqstd.freq partition.freq
All files are identical
```

## Question 4.
