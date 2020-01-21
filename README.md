# pokerbots2020

# Included in repository
Pokerbots 2020 engine and skeleton code from https://github.com/mitpokerbots/engine-2020

Foldbot: check-folds every hand

Aimbot v4.12: an if statement bot from middle of week 1 that doesn't consider permutation

Aimbot: an if statement bot that attempts to determine permutation through particle filter + random swaps when # candidates drops under threshold of 200, uses OMPEval from https://github.com/zekyll/OMPEval

Process gamelog: script to format a gamelog.txt file

# Abstractions
## Card abstractions:

abstractions folder: code for generating flop, turn, and river abstractions

Preflop bucketing: 169 buckets representing all strategically different hands

Flop bucketing: equity histogram + earth mover’s distance (100 buckets using kmeans++)

Turn bucketing: equity histogram + earth mover’s distance (100 buckets using kmeans++)

River bucketing: opponent cluster hand strength + l2 distance (100 buckets using kmeans++) 

Imperfect recall scheme: forget everything from the previous street

Kevin Waugh isomorphisms for data encoding from https://github.com/kdub0/hand-isomorphism

## Planned action abstractions:

Bet sizes: 1/2 pot, pot, 2x pot, all in

Raise sizes: pot, all in

Other actions: check, fold

##  Information set: action of current street, pot size and round pips, which bucket

## CFR implementation: external sampling, reward by equity on showdown nodes

# Where we learned from
https://github.com/mitpokerbots/lecture-notes-2020
http://modelai.gettysburg.edu/2013/cfr/
https://www.pokernews.com/strategy/artificial-intelligence-hold-em-1-23152.htm
http://poker-ai.org/phpbb/
http://poker.cs.ualberta.ca/
https://github.com/pandaant/poker-cfrm
