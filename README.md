# pokerbots2020

CFR code at
https://github.com/adamczheng/CFR

# Included in repository
### Pokerbots 2020 engine and skeleton code
from https://github.com/mitpokerbots/engine-2020

### Foldbot: 
check-folds every hand

### Aimbot v4.12:
an if statement bot from middle of week 1 that doesn't consider permutation

### Aimbot:
an if statement bot that attempts to determine permutation through particle filter + random swaps when # candidates drops under threshold of 200, uses OMPEval from https://github.com/zekyll/OMPEval, Note: if you want to run on other machines you may need to recompile OMPEval

### Process gamelog:
script to format a gamelog.txt file

# Permutation Learning
Our week 1 strategy (which is still basically the case) was to generate a lot of permutations (50,000) from the prior (0.25 geometric distribution) and then particle filter over showdowns we see. It drops really fast and could drop to 0 from pretty high numbers (>200) although unlikely. If we ever drop below the threshold (200), we go through each permutation and try each permutation with swap distance of <= 1 to see if they follow showdown rules. If they do we add them to the list. If our list becomes entirely one permutation, then we're done. 

Before the end of week 2 we decided that moving an index i->j performs slightly better than swap(i,j) on average, although there would be double the permutations with distance 1 and its implementation is slower, so we didn't do it.

For playing, before 5 showdowns, we assume the permutation is 23456789TJQKA, but after 5 showdowns we randomly select from remaining candidate.

Against check-call bot, we determine the permutation about 30% of the time with our week 1 strategy, although we get a good idea of the permutation much more often. It sometimes takes too long (if we never generate the right permutation) so we don't allow this part of the program to exceed 20-25 seconds.

Currently, we're trying to implement Metropolis Hasting to see if it yields better results.

# Abstractions
## Card abstractions:

### abstractions folder:
code for generating flop, turn, and river abstractions

### Preflop bucketing:
169 buckets representing all strategically different hands

### Flop bucketing:
equity histogram (rounded to nearest 2%) + earth mover’s distance (100 buckets using kmeans++)

### Turn bucketing:
equity histogram (rounded to nearest 2%) + earth mover’s distance (100 buckets using kmeans++)

### River bucketing:
opponent cluster hand strength (vs 8 ranges) + l2 distance (100 buckets using kmeans++) 

### Imperfect recall scheme:
forget everything from the previous street

### Kevin Waugh isomorphisms for data encoding
from https://github.com/kdub0/hand-isomorphism

## Planned action abstractions:

Bet sizes: 1/2 pot, pot, 2x pot, all in

Raise sizes: 1/2 pot, pot, 2x pot, all in

Other actions: check, fold

##  Information set: 
card abstraction, k-th action on street, street, pot, pot odds bucket (see report for more details)

## CFR implementation:
external sampling, linear weighting

# Where we learned from
https://github.com/mitpokerbots/lecture-notes-2020

http://modelai.gettysburg.edu/2013/cfr/

https://github.com/Fro116/Pokerbots

https://www.pokernews.com/strategy/artificial-intelligence-hold-em-1-23152.htm

http://poker-ai.org/phpbb/

http://poker.cs.ualberta.ca/
