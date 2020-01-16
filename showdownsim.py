import numpy as np
import eval7
import random

def permute_values():
	'''
	Selects a value permutation for the whole game according the prior distribution.
	'''
	orig_perm = list(range(13))[::-1]
	prop_perm = []
	seed = np.random.geometric(p=0.25, size=13) - 1
	for s in seed:
		pop_i = len(orig_perm) - 1 - (s % len(orig_perm))
		prop_perm.append(orig_perm.pop(pop_i))
	return prop_perm

ranks = list("23456789TJQKA")
suits = list("cdhs")

perm = permute_values()
perm_dict = {}
for i, v in enumerate(ranks):
    for s in suits:
        card = v + s
        permuted_i = perm[i]
        permuted_v = ranks[permuted_i]
        permuted_card = eval7.Card(permuted_v + s)
        perm_dict[card] = permuted_card


print([ranks[x] for x in perm])

def showdown():
    used = []
    def gen_card():
        rank = random.choice(ranks)
        suit = random.choice(suits)
        while (rank + suit) in used:
            rank = random.choice(ranks)
            suit = random.choice(suits)
        used.append(rank + suit)
        return rank + suit


    a = [gen_card(), gen_card()]
    b = [gen_card(), gen_card()]
    board = [gen_card(), gen_card(), gen_card(), gen_card(), gen_card()]
    pa = [perm_dict[x] for x in a]
    pb = [perm_dict[x] for x in b]
    pboard = [perm_dict[x] for x in board]

    a_val = eval7.evaluate(pa+pboard)
    b_val = eval7.evaluate(pb+pboard)

    
    if a_val > b_val:
        print(a)
        print("beats")
        print(b)
    elif a_val < b_val:
        print(b)
        print("beats")
        print(a)
    else:
        print(a)
        print("ties")
        print(b)
    print("on board")
    print(board)
    print()


for i in range(10):
    showdown()

