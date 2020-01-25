import numpy as np
import eval7
import random
import scipy
import scipy.stats
import collections

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

def pdict(perm):
    '''
    Generates a permutation dictionary given a permutation.
    '''
    perm_dict = {}
    for i, v in enumerate(ranks):
        for s in suits:
            card = v + s
            permuted_i = perm[i]
            permuted_v = ranks[permuted_i]
            permuted_card = eval7.Card(permuted_v + s)
            perm_dict[card] = permuted_card
    return perm_dict


'''
create the probability table of j getting sent to pi.
'''
prob_at_j = np.zeros((13,13))

for pi in range(13):
    for j in range(13-pi):
        prob = (0.75 ** j) * 0.25
        prob_at_j[pi,j] = prob
    prob_at_j[pi] /= prob_at_j[pi].sum(0)

def perm_prob(perm):
    '''
    finds the probability of generating perm from the prior.

    OUTPUT:
    curr_prob: the probability of generating perm.
    '''
    editing = list(range(13))
    curr_prob = 1
    for pi, i in enumerate(perm):
        j = editing.index(i)
        curr_prob *= prob_at_j[pi,j]
        editing.remove(i)
    return curr_prob

actual_perm = permute_values()
actual_dict = pdict(actual_perm)

def create_board():
    '''
    Generates a random board tuple (a, b, board_cards).

    OUTPUTS:
    a: player A's hand.
    b: player B's hand.
    board_cards: the river.
    '''
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
    board_cards = [gen_card(), gen_card(), gen_card(), gen_card(), gen_card()]
    return a, b, board_cards

def showdown(p,board_state):
    '''
    Computes a showdown between two players, assessed using p as the desired permutation.

    OUTPUTS:
     1: a > b
     0: a = b
    -1: a > b
    '''
    proposed_dict = pdict(p)
    #pa = [perm_dict[x] for x in a]
    #pb = [perm_dict[x] for x in b]
    #pboard = [perm_dict[x] for x in board]
    a, b, board = board_state
    pa = [proposed_dict[x] for x in a]
    pb = [proposed_dict[x] for x in b]
    pboard = [proposed_dict[x] for x in board]

    a_val = eval7.evaluate(pa+pboard)
    b_val = eval7.evaluate(pb+pboard)
    
    if a_val > b_val:
        #print(a)
        #print("beats")
        #print(b)
        return 1
    elif a_val < b_val:
        #print(b)
        #print("beats")
        #print(a)
        return -1
    else:
        #print(a)
        #print("ties")
        #print(b)
        return 0

def compatible(p, board_state):
    '''
    tests compatibility of permutation with the showdown result on the board.
    '''
    #p is permutation, s is showdown
    return showdown(p, board_state) == showdown(actual_perm, board_state)

def new_candidate(p, fails):
    '''
    #generate random indices i, j
    r = np.random.uniform()
    #option 1: random dude
    r_time = 0.01*(2 ** fails - 1)
    if r < r_time:
        return permute_values()
    '''
    i = np.random.randint(0,13)
    j = np.random.randint(0,13)
    '''
    #option 2: reinsert
    r = np.random.uniform()
    if r < 0.75:
        #reinsert i in position j
        if i == j:
            return p
        if i < j:
            return p[:i]+p[i+1:j]+p[i:i+1]+p[j:]
        return p[:j]+p[i:i+1]+p[j:i]+p[i+1:]
    #option 3: swap
    '''
    p[i], p[j] = p[j], p[i]
    return p
    

'''
def new_candidate(p):
    #generate random indices i, j
    i = np.random.randint(0,13)
    j = np.random.randint(0,13)
    p[i], p[j] = p[j], p[i]
    return p
'''

def fail_penalty(fails):
    #return 1
    return 0.05 ** fails

def mh_showdown(showdowns,curr,increment=1):
    '''
    #initialize node
    curr = list(range(13))
    curr_prior = perm_prob(curr)
    #dictionary of probabilities
    visits = collections.defaultdict(int)
    '''
    visits = collections.defaultdict(int)
    curr_prior = perm_prob(curr)
    #for iterations
    NUM_ITERATIONS = 10000
    #FAILED_OKAY = 10

    curr_fails = 0
    #update fails
    for s in showdowns:
        curr_fails += not compatible(curr, s)
    #alphas = {}
    for i in range(NUM_ITERATIONS):
        #propose new candidate
        q = new_candidate(curr, curr_fails)
        #compute showdown ratio
        q_failed = 0
        for s in showdowns:
            #if q_failed > FAILED_OKAY:
            #    break
            q_failed += not compatible(q, s)
        showdown_ratio = fail_penalty(q_failed) / fail_penalty(curr_fails)
        #compute prior ratio
        q_prior = perm_prob(q)
        prior_ratio = q_prior / curr_prior
        #compute alpha
        alpha = prior_ratio * showdown_ratio
        #alpha = (q_prior / curr_prior) * (fail_penalty(q_failed) / fail_penalty(curr_fails))
        bob = np.random.uniform()
        if bob < alpha:
            curr = q
            curr_prior = q_prior
            curr_fails = q_failed
        if i >= 3000:
            visits[tuple(curr)] += increment
    return list(max(visits.items(), key = lambda k: k[1])[0]), curr_fails

def metropolis_hastings():
    #initialize node
    curr = list(range(13))
    curr_prior = perm_prob(curr)
    curr_fails = 0
    #do some showdowns
    SHOWDOWN_NUM = 80
    showdowns = []
    for i in range(SHOWDOWN_NUM//2):
        showdowns.append(create_board())
    for i in range(SHOWDOWN_NUM//2,SHOWDOWN_NUM):
        showdowns.append(create_board())
        curr, curr_fails = mh_showdown(showdowns,curr)
        print("showdown:", i)
        print("best guess:", curr, curr_fails)
        if curr_fails > 4:
            break

#bob = create_board()
#print(bob)
#print(compatible(list(range(13)),bob))

print(actual_perm)
metropolis_hastings()

#bruh what
'''
[2, 1, 8, 0, 5, 3, 7, 11, 9, 10, 6, 4, 12]
best guess: [1, 5, 0, 2, 7, 4, 11, 6, 3, 8, 12, 9, 10] 1
best guess: [7, 0, 2, 5, 4, 12, 3, 1, 8, 6, 9, 10, 11] 1
best guess: [5, 11, 1, 0, 7, 8, 2, 10, 12, 3, 6, 4, 9] 1
best guess: [2, 1, 9, 0, 3, 4, 5, 12, 7, 8, 6, 11, 10] 1
best guess: [1, 4, 0, 2, 5, 10, 3, 8, 6, 9, 12, 7, 11] 0
best guess: [0, 1, 7, 4, 3, 2, 6, 10, 8, 5, 12, 11, 9] 0
best guess: [0, 2, 8, 1, 5, 3, 6, 4, 9, 10, 7, 12, 11] 1
best guess: [2, 4, 7, 1, 0, 3, 10, 5, 9, 11, 8, 12, 6] 1
best guess: [2, 0, 1, 4, 6, 7, 3, 11, 5, 10, 9, 12, 8] 1
best guess: [3, 4, 7, 6, 2, 1, 9, 0, 5, 8, 12, 11, 10] 1
best guess: [0, 1, 8, 3, 5, 2, 9, 4, 7, 11, 10, 12, 6] 1
best guess: [0, 4, 6, 1, 3, 10, 9, 2, 7, 8, 5, 11, 12] 0
best guess: [0, 4, 1, 3, 8, 10, 2, 11, 5, 9, 12, 6, 7] 1
best guess: [1, 8, 4, 3, 5, 7, 2, 12, 0, 10, 9, 11, 6] 1
best guess: [0, 8, 1, 2, 6, 10, 3, 4, 5, 7, 12, 9, 11] 0
best guess: [0, 8, 2, 3, 6, 4, 1, 7, 10, 12, 11, 5, 9] 1
best guess: [0, 8, 2, 3, 6, 4, 1, 7, 10, 12, 11, 5, 9] 2
best guess: [0, 8, 2, 1, 3, 6, 4, 7, 10, 12, 11, 5, 9] 1
best guess: [0, 6, 8, 5, 2, 1, 7, 3, 11, 10, 9, 12, 4] 0
best guess: [2, 5, 0, 1, 4, 7, 3, 6, 8, 11, 12, 10, 9] 1
best guess: [2, 5, 1, 0, 4, 7, 3, 10, 8, 11, 12, 6, 9] 1
best guess: [2, 5, 0, 1, 4, 7, 3, 8, 10, 6, 12, 11, 9] 1
best guess: [2, 5, 0, 1, 4, 7, 3, 8, 10, 6, 12, 11, 9] 0
best guess: [2, 5, 1, 0, 4, 7, 3, 6, 10, 9, 12, 11, 8] 0
best guess: [0, 6, 3, 1, 4, 7, 2, 9, 10, 5, 12, 11, 8] 0
best guess: [0, 6, 3, 1, 4, 7, 2, 9, 10, 5, 12, 11, 8] 1
best guess: [0, 6, 1, 3, 4, 7, 2, 5, 9, 12, 10, 11, 8] 2
best guess: [0, 6, 1, 3, 4, 7, 2, 10, 9, 8, 12, 5, 11] 2
best guess: [1, 10, 6, 0, 2, 3, 7, 12, 8, 5, 9, 4, 11] 0
best guess: [1, 10, 6, 2, 3, 0, 7, 12, 8, 5, 9, 4, 11] 2
best guess: [1, 10, 6, 2, 3, 0, 7, 12, 8, 5, 9, 4, 11] 2
best guess: [1, 10, 6, 3, 2, 0, 7, 12, 8, 5, 9, 4, 11] 1
best guess: [1, 10, 6, 0, 3, 2, 7, 12, 9, 5, 8, 4, 11] 0
best guess: [1, 10, 6, 0, 3, 2, 7, 12, 9, 5, 8, 4, 11] 0
best guess: [1, 10, 6, 3, 2, 0, 7, 12, 9, 5, 8, 4, 11] 0
best guess: [0, 10, 6, 3, 2, 1, 7, 12, 9, 5, 8, 4, 11] 2
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 8, 5, 9, 4, 11] 1
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 8, 5, 9, 4, 11] 1
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 1
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 2
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 1
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 3
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 2
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 2
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 1
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 2
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 4
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 2
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 3
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 2
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 1
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 1
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 3
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 2
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 2
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 6
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 4
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 2
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 2
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 3
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 3
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 3
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 3
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 3
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 3
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 3
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 3
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 3
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 4
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 6
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 5
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 5
best guess: [2, 10, 6, 0, 3, 1, 7, 12, 9, 5, 8, 4, 11] 5
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 2
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 3
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 6
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 6
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 6
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 7
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 7
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 6
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 7
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 6
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 6
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 6
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 7
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 7
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 8
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 6
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 6
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 7
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 8
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 10
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 8
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 8
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 5
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 5
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 5
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 5
best guess: [0, 2, 10, 6, 3, 1, 7, 12, 9, 5, 8, 4, 11] 6
'''
