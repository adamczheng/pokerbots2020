import numpy as np
import eval7
import random
import scipy
import scipy.stats

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
#print(prob_at_j)

'''
#prob_table = np.zeros((13,13))
#i_at_pos_j = np.zeros((13,13,13))
for pi in range(13):
    #pi corresponds to the position in the permutation.
    for i in range(13):
        if pi == 0:
            i_at_pos_j[pi,i,i] = 1
        else:
            for j in range(i-pi,i+1):
                left = i - j
                right = pi - left
                

        prob_pi = sum(prob_at_j[pi, j] * i_at_pos_j[pi, i, j] for j in range(i+1))
        prob_table[pi, i] = prob_pi
#        if pi == 0:
#            i_at_pos_j[i,i,0] = 1
#            prob_pi = (0.75 ** i) * 0.25
#            prob_table[pi,i] = prob_pi
#        else:
#            prob_pi = 0
            #compute if displaced
#            for j in range(i):
#                prob_jtaken = 1 - 
    if pi == 0:
        prob_table[pi] /= prob_table[pi].sum(0)

for row in prob_table:
    print(row)
'''

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

#print(perm_prob(actual_perm))
#print(perm_prob(list(range(13))))

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

def initialize_perms(n):
    '''
    initializes the list of potential permutations.

    INPUT:
    n: number of permtuations
    OUTPUT:
    perm_list: list of generated permutations
    '''
    perm_list = []
    for i in range(n):
        perm_list.append(permute_values())
    return perm_list

def compatible(p, board_state):
    '''
    tests compatibility of permutation with the showdown result on the board.
    '''
    #p is permutation, s is showdown
    return showdown(p, board_state) == showdown(actual_perm, board_state)


def permutation_deducer():
    '''
    tries to find the correct permutation.
    '''

    #define constants/external variables
    NUM_INITIALS = 100000
    state = 1

    #generate initial list of candidates
    perms = initialize_perms(NUM_INITIALS)

    #print out the list of initial candidates
    for i, p in enumerate(perms):
        print(i, p)

    #create a dictionary for node weights
    node_weights = {tuple(p) : perm_prob(p) for p in perms}
    
    #regenerate once below threshold
    def regenerate(perms_list,show_board):
        new_perms = perms_list.copy()
        for p in perms_list:
            for i in range(13):
                for j in range(13):
                    if i != j:
                        if i < j:
                            reinsert = p[:i]+p[i+1:j]+p[i:i+1]+p[j:]
                        else:
                            reinsert = p[:j]+p[i:i+1]+p[j:i]+p[i+1:]
                        #metropolis-hastings
                        #print("reinsert",reinsert)
                        node_weights[tuple(reinsert)] = node_weights.get(tuple(reinsert), perm_prob(reinsert)) * compatible(reinsert, show_board)
                        relative_weight = node_weights[tuple(reinsert)] / node_weights[tuple(p)]
                        swap_prob = min(1, relative_weight)
                        if np.random.uniform() < swap_prob:
                            new_perms.append(reinsert)
        perms_list.extend(new_perms)
        return perms_list

    #simulate a showdown update
    num_showdowns = 0 #counts number of showdowns used
    for i in range(1,5):
        state = i
        #while above threshold
        while len(perms) > threshold(state):
            s_board = create_board()
            num_showdowns += 1
            for p in perms:
                if not compatible(p, s_board):
                    node_weights[tuple(p)] = 0
            perms = [p for p in perms if compatible(p, s_board)]
        for i, p in enumerate(perms):
            print(i, p)
        #now it's below threshold
        if i < 4:
            perms = regenerate(perms,s_board)
        print("shows",num_showdowns)
    return perms



def threshold(s):
    '''
    number of permutations before we start regenerating.
    '''
    if s == 1:
        return 1000
    if s == 2:
        return 250
    if s == 3:
        return 100
    if s == 4:
        return 25
    return 5


def lcs_dist(text1, text2):
    n,m=len(text1),len(text2)
    dp=[[0]*(m+1) for _ in range(n+1)]
    for i in range(n):
        for j in range(m):
            if text1[i]==text2[j]:
                dp[i+1][j+1]=dp[i][j]+1
            else:
                dp[i+1][j+1]=max(dp[i][j+1],dp[i+1][j])
    return len(text1) - dp[-1][-1]

def swap_dist(p, q):
    print("p:", p)
    print("actual:", q)
    inverse_p = [p.index(i) for i in range(len(p))]
    invert_q = [q[inverse_p[i]] for i in range(len(q))]
    q = invert_q
    not_visited = [True] * len(q)
    cycles = 0
    for i in range(len(q)):
        curr_pos = i
        if not_visited[curr_pos]:
            cycles += 1
        while not_visited[curr_pos]:
            not_visited[curr_pos] = False
            curr_pos = q[curr_pos]
    return len(q) - cycles

#print(swap_dist([0,1,2,3,4,5,6],[1,2,0,3,4,5,6]))
#print(lcs_dist([0,1,2,3,4,5,6],[1,2,0,3,4,5,6]))

'''
for p in perms:
    correct = []
    for i in range(len(p)):
        if p[i] == actual_perm[i]:
            correct.append(1)
        else:
            correct.append(0)
    print(correct,sum(correct))
'''
perms = permutation_deducer()
print(actual_perm)
#print([ranks[x] for x in actual_perm])

swap_total = 0
reinsert_total = 0
swap_min = 13
reinsert_min = 13
for p in perms:
    swap = swap_dist(p, actual_perm)
    swap_total += swap
    if swap < swap_min:
        swap_min = swap
    rein = lcs_dist(p, actual_perm)
    reinsert_total += rein
    if rein < reinsert_min:
        reinsert_min = rein
    print("Swap distance:", swap) 
    print("Reinsertion distance:", rein)
print("swap_average", swap_total/len(perms))
print("reinsert_average", reinsert_total/len(perms))
print("swap_min", swap_min)
print("reinsert_min", reinsert_min)
print(actual_perm)

#random_board = create_board()
#print(compatible(permute_values(),random_board))
