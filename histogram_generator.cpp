#include <bits/stdc++.h>
using namespace std;
#define MAX_GROUP_INDEX        0x100000 
#define MAX_CARDS_PER_ROUND    15
#define ROUND_SHIFT            4
#define ROUND_MASK             0xf
#define equal equallll
#define PRIhand_index        PRIu64
#define MAX_ROUNDS           8
#define this thiss
#define RANKS 13
#define SUITS 4
#define CARDS 52
typedef uint_fast32_t card_t;

static inline card_t deck_get_suit(card_t card) {
  return card&3;
}

static inline card_t deck_get_rank(card_t card) {
  return card>>2;
}

static inline card_t deck_make_card(card_t suit, card_t rank) {
  return rank<<2 | suit;
}
typedef uint64_t hand_index_t;
struct hand_indexer_s {
  uint8_t cards_per_round[MAX_ROUNDS], round_start[MAX_ROUNDS];
  uint_fast32_t rounds, configurations[MAX_ROUNDS], permutations[MAX_ROUNDS];
  hand_index_t round_size[MAX_ROUNDS];

  uint_fast32_t * permutation_to_configuration[MAX_ROUNDS], * permutation_to_pi[MAX_ROUNDS], * configuration_to_equal[MAX_ROUNDS];  
  uint_fast32_t (* configuration[MAX_ROUNDS])[SUITS];
  uint_fast32_t (* configuration_to_suit_size[MAX_ROUNDS])[SUITS];
  hand_index_t * configuration_to_offset[MAX_ROUNDS];
};

struct hand_indexer_state_s {
  uint_fast32_t suit_index[SUITS];
  uint_fast32_t suit_multiplier[SUITS];
  uint_fast32_t round, permutation_index, permutation_multiplier;
  uint32_t used_ranks[SUITS];
};
typedef struct hand_indexer_s hand_indexer_t;
typedef struct hand_indexer_state_s hand_indexer_state_t;


static uint8_t nth_unset[1<<RANKS][RANKS];
static bool equal[1<<(SUITS-1)][SUITS];
static uint_fast32_t nCr_ranks[RANKS+1][RANKS+1], rank_set_to_index[1<<RANKS], index_to_rank_set[RANKS+1][1<<RANKS], (*suit_permutations)[SUITS];
static hand_index_t nCr_groups[MAX_GROUP_INDEX][SUITS+1];
static void __attribute__((constructor)) hand_index_ctor() {
  for(uint_fast32_t i=0; i<1<<(SUITS-1); ++i) {
    for(uint_fast32_t j=1; j<SUITS; ++j) {
      equal[i][j] = i&1<<(j-1);
    }
  }

  for(uint_fast32_t i=0; i<1<<RANKS; ++i) {
    for(uint_fast32_t j=0, set=~i&(1<<RANKS)-1; j<RANKS; ++j, set&=set-1) {
      nth_unset[i][j] = set?__builtin_ctz(set):0xff;
    }
  }

  nCr_ranks[0][0]     = 1;
  for(uint_fast32_t i=1; i<RANKS+1; ++i) {
    nCr_ranks[i][0]   = nCr_ranks[i][i] = 1;
    for(uint_fast32_t j=1; j<i; ++j) {
      nCr_ranks[i][j] = nCr_ranks[i-1][j-1] + nCr_ranks[i-1][j];
    }
  }

  nCr_groups[0][0] = 1;
  for(uint_fast32_t i=1; i<MAX_GROUP_INDEX; ++i) {
    nCr_groups[i][0] = 1;
    if (i < SUITS+1) {
      nCr_groups[i][i] = 1;
    }
    for(uint_fast32_t j=1; j<(i<(SUITS+1)?i:(SUITS+1)); ++j) {
      nCr_groups[i][j] = nCr_groups[i-1][j-1] + nCr_groups[i-1][j];
    } 
  }

  for(uint_fast32_t i=0; i<1<<RANKS; ++i) {
    for(uint_fast32_t set=i, j=1; set; ++j, set&=set-1) {
      rank_set_to_index[i]  += nCr_ranks[__builtin_ctz(set)][j];
    }
    index_to_rank_set[__builtin_popcount(i)][rank_set_to_index[i]] = i;
  }

  uint_fast32_t num_permutations = 1;
  for(uint_fast32_t i=2; i<=SUITS; ++i) {
    num_permutations *= i;
  }

  suit_permutations = (uint_fast32_t (*)[4])calloc(num_permutations, SUITS*sizeof(uint_fast32_t));
  
  for(uint_fast32_t i=0; i<num_permutations; ++i) {
    for(uint_fast32_t j=0, index=i, used=0; j<SUITS; ++j) {
      uint_fast32_t suit = index%(SUITS-j); index /= SUITS-j;
      uint_fast32_t shifted_suit = nth_unset[used][suit];
      suit_permutations[i][j] = shifted_suit;
      used                   |= 1<<shifted_suit;
    }
  }
} 
void hand_indexer_free(hand_indexer_t * indexer) {
  for(uint_fast32_t i=0; i<indexer->rounds; ++i) {
    free(indexer->permutation_to_configuration[i]);
    free(indexer->permutation_to_pi[i]);
    free(indexer->configuration_to_equal[i]);
    free(indexer->configuration_to_offset[i]);
    free(indexer->configuration[i]);
    free(indexer->configuration_to_suit_size[i]);
  }
}

void enumerate_configurations_r(uint_fast32_t rounds, const uint8_t cards_per_round[], 
    uint_fast32_t round, uint_fast32_t remaining, 
    uint_fast32_t suit, uint_fast32_t equal, uint_fast32_t used[], uint_fast32_t configuration[],
    void (*observe)(uint_fast32_t, uint_fast32_t[], void*), void * data) {
  if (suit == SUITS) {
    observe(round, configuration, data);

    if (round+1 < rounds) {
      enumerate_configurations_r(rounds, cards_per_round, round+1, cards_per_round[round+1], 0, equal, used, configuration, observe, data);
    }
  } else {
    uint_fast32_t min = 0;
    if (suit == SUITS-1) {
      min = remaining;
    }
    
    uint_fast32_t max = RANKS-used[suit];
    if (remaining < max) {
      max = remaining;
    }
   
    uint_fast32_t previous = RANKS+1;
    bool was_equal = equal&1<<suit;
    if (was_equal) {
      previous = configuration[suit-1]>>ROUND_SHIFT*(rounds-round-1)&ROUND_MASK;
      if (previous < max) {
        max = previous;
      }
    }
    
    uint_fast32_t old_configuration = configuration[suit], old_used = used[suit];
    for(uint_fast32_t i=min; i<=max; ++i) {
      uint_fast32_t new_configuration = old_configuration | i<<ROUND_SHIFT*(rounds-round-1);
      uint_fast32_t new_equal = (equal&~(1<<suit))|(was_equal&(i==previous))<<suit;

      used[suit] = old_used+i;
      configuration[suit] = new_configuration;
      enumerate_configurations_r(rounds, cards_per_round, round, remaining-i, suit+1, new_equal, used, configuration, observe, data);
      configuration[suit] = old_configuration;
      used[suit] = old_used;
    }
  }
}

void enumerate_configurations(uint_fast32_t rounds, const uint8_t cards_per_round[],
    void (*observe)(uint_fast32_t, uint_fast32_t[], void*), void * data) {
  uint_fast32_t used[SUITS] = {0}, configuration[SUITS] = {0};
  enumerate_configurations_r(rounds, cards_per_round, 0, cards_per_round[0], 0, (1<<SUITS) - 2, used, configuration, observe, data);
}

void count_configurations(uint_fast32_t round, uint_fast32_t configuration[], void * data) {
  uint_fast32_t * counts = (uint_fast32_t*)data; ++counts[round];
}

void tabulate_configurations(uint_fast32_t round, uint_fast32_t configuration[], void * data) {
  hand_indexer_t * indexer = (hand_indexer_t*)data;

  uint_fast32_t id = indexer->configurations[round]++;
  for(; id>0; --id) {
    for(uint_fast32_t i=0; i<SUITS; ++i) {
      if (configuration[i] < indexer->configuration[round][id-1][i]) {
        break;
      } else if (configuration[i] > indexer->configuration[round][id-1][i]) {
        goto out;
      }
    }
    for(uint_fast32_t i=0; i<SUITS; ++i) {
      indexer->configuration[round][id][i]              = indexer->configuration[round][id-1][i];
      indexer->configuration_to_suit_size[round][id][i] = indexer->configuration_to_suit_size[round][id-1][i];
    }
    indexer->configuration_to_offset[round][id] = indexer->configuration_to_offset[round][id-1];
    indexer->configuration_to_equal[round][id]  = indexer->configuration_to_equal[round][id-1];
  }
out:;

  indexer->configuration_to_offset[round][id] = 1; 
  for(uint_fast32_t i=0; i<SUITS; ++i) {
    indexer->configuration[round][id][i] = configuration[i];
  }

  uint_fast32_t equal = 0;
  for(uint_fast32_t i=0; i<SUITS;) {
    hand_index_t size = 1;
    for(uint_fast32_t j=0, remaining=RANKS; j<=round; ++j) {
      uint_fast32_t ranks = configuration[i]>>ROUND_SHIFT*(indexer->rounds-j-1)&ROUND_MASK;
      size *= nCr_ranks[remaining][ranks];
      remaining -= ranks;
    }
    assert(size+SUITS-1 < MAX_GROUP_INDEX);
    
    uint_fast32_t j=i+1; for(; j<SUITS && configuration[j] == configuration[i]; ++j) {} 
    for(uint_fast32_t k=i; k<j; ++k) {
      indexer->configuration_to_suit_size[round][id][k] = size;
    }

    indexer->configuration_to_offset[round][id] *= nCr_groups[size+j-i-1][j-i];
    
    for(uint_fast32_t k=i+1; k<j; ++k) {
      equal |= 1<<k;
    }

    i = j;
  }
  
  indexer->configuration_to_equal[round][id] = equal>>1;
}

void enumerate_permutations_r(uint_fast32_t rounds, const uint8_t cards_per_round[], 
    uint_fast32_t round, uint_fast32_t remaining, 
    uint_fast32_t suit, uint_fast32_t used[], uint_fast32_t count[],
    void (*observe)(uint_fast32_t, uint_fast32_t[], void*), void * data) {
  if (suit == SUITS) {
    observe(round, count, data);

    if (round+1 < rounds) {
      enumerate_permutations_r(rounds, cards_per_round, round+1, cards_per_round[round+1], 0, used, count, observe, data);
    }
  } else {
    uint_fast32_t min = 0;
    if (suit == SUITS-1) {
      min = remaining;
    }
    uint_fast32_t max = RANKS-used[suit];
    if (remaining < max) {
      max = remaining;
    }
    
    uint_fast32_t old_count = count[suit], old_used = used[suit];
    for(uint_fast32_t i=min; i<=max; ++i) {
      uint_fast32_t new_count = old_count | i<<ROUND_SHIFT*(rounds-round-1);

      used[suit] = old_used+i;
      count[suit] = new_count;
      enumerate_permutations_r(rounds, cards_per_round, round, remaining-i, suit+1, used, count, observe, data);
      count[suit] = old_count;
      used[suit] = old_used;
    }
  }
}

void enumerate_permutations(uint_fast32_t rounds, const uint8_t cards_per_round[],
    void (*observe)(uint_fast32_t, uint_fast32_t[], void*), void * data) {
  uint_fast32_t used[SUITS] = {0}, count[SUITS] = {0};
  enumerate_permutations_r(rounds, cards_per_round, 0, cards_per_round[0], 0, used, count, observe, data);
}

void count_permutations(uint_fast32_t round, uint_fast32_t count[], void * data) {
  hand_indexer_t * indexer = (hand_indexer_t*)data;

  uint_fast32_t idx = 0, mult = 1;
  for(uint_fast32_t i=0; i<=round; ++i) {
    for(uint_fast32_t j=0, remaining=indexer->cards_per_round[i]; j<SUITS-1; ++j) {
      uint_fast32_t size = count[j]>>(indexer->rounds-i-1)*ROUND_SHIFT&ROUND_MASK;
      idx  += mult*size;
      mult *= remaining+1;
      remaining -= size;
    }
  }
  
  if (indexer->permutations[round] < idx+1) {
    indexer->permutations[round] = idx+1;
  }
}

void tabulate_permutations(uint_fast32_t round, uint_fast32_t count[], void * data) {
  hand_indexer_t * indexer = (hand_indexer_t*)data;

  uint_fast32_t idx = 0, mult = 1;
  for(uint_fast32_t i=0; i<=round; ++i) {
    for(uint_fast32_t j=0, remaining=indexer->cards_per_round[i]; j<SUITS-1; ++j) {
      uint_fast32_t size = count[j]>>(indexer->rounds-i-1)*ROUND_SHIFT&ROUND_MASK;
      idx       += mult*size;
      mult      *= remaining+1;
      remaining -= size;
    }
  }
  
  uint_fast32_t pi[SUITS];
  for(uint_fast32_t i=0; i<SUITS; ++i) {
    pi[i] = i;
  }

  for(uint_fast32_t i=1; i<SUITS; ++i) {
    uint_fast32_t j=i, pi_i = pi[i]; for(; j>0; --j) {
      if (count[pi_i] > count[pi[j-1]]) {
        pi[j] = pi[j-1];
      } else {
        break;
      }
    }
    pi[j] = pi_i;
  }

  uint_fast32_t pi_idx = 0, pi_mult = 1, pi_used = 0;
  for(uint_fast32_t i=0; i<SUITS; ++i) {
    uint_fast32_t this_bit = 1<<pi[i];
    uint_fast32_t smaller  = __builtin_popcount((this_bit-1)&pi_used);
    pi_idx  += (pi[i]-smaller)*pi_mult;
    pi_mult *= SUITS-i;
    pi_used |= this_bit;
  }

  indexer->permutation_to_pi[round][idx] = pi_idx;

  uint_fast32_t low = 0, high = indexer->configurations[round];
  while(low < high) {
    uint_fast32_t mid = (low+high)/2;

    int_fast32_t compare = 0;
    for(uint_fast32_t i=0; i<SUITS; ++i) {
      uint_fast32_t this  = count[pi[i]];
      uint_fast32_t other = indexer->configuration[round][mid][i];
      if (other > this) {
        compare = -1; break;
      } else if (other < this) {
        compare = 1; break;
      }
    }
 
    if (compare == -1) {
      high = mid;
    } else if (compare == 0) {
      low = high = mid;
    } else {
      low = mid+1;
    }
  }

  indexer->permutation_to_configuration[round][idx] = low;
}

bool hand_indexer_init(uint_fast32_t rounds, const uint8_t cards_per_round[], hand_indexer_t * indexer) {
  if (rounds == 0) {
    return false;
  }
  if (rounds > MAX_ROUNDS) {
    return false;
  }
  for(uint_fast32_t i=0, count=0; i<rounds; ++i) {
    count += cards_per_round[i];
    if (count > CARDS) {
      return false;
    }
  }

  memset(indexer, 0, sizeof(hand_indexer_t));

  indexer->rounds = rounds;
  memcpy(indexer->cards_per_round, cards_per_round, rounds); 
  for(uint_fast32_t i=0, j=0; i<rounds; ++i) {
    indexer->round_start[i] = j; j += cards_per_round[i];
  }

  memset(indexer->configurations, 0, sizeof(indexer->configurations));
  enumerate_configurations(rounds, cards_per_round, count_configurations, indexer->configurations);

  for(uint_fast32_t i=0; i<rounds; ++i) {
    indexer->configuration_to_equal[i]     = (uint_fast32_t*)calloc(indexer->configurations[i], sizeof(uint_fast32_t));
    indexer->configuration_to_offset[i]    = (hand_index_t*)calloc(indexer->configurations[i], sizeof(hand_index_t));
    indexer->configuration[i]              = (uint_fast32_t(*)[4])calloc(indexer->configurations[i], SUITS*sizeof(uint_fast32_t));
    indexer->configuration_to_suit_size[i] = (uint_fast32_t(*)[4])calloc(indexer->configurations[i], SUITS*sizeof(uint_fast32_t));
    if (!indexer->configuration_to_equal[i] ||
        !indexer->configuration_to_offset[i] ||
        !indexer->configuration[i] ||
        !indexer->configuration_to_suit_size[i]) {
      hand_indexer_free(indexer);
      return false; 
    }
  }

  memset(indexer->configurations, 0, sizeof(indexer->configurations));
  enumerate_configurations(rounds, cards_per_round, tabulate_configurations, indexer);
  
  for(uint_fast32_t i=0; i<rounds; ++i) {
    hand_index_t accum = 0; for(uint_fast32_t j=0; j<indexer->configurations[i]; ++j) {
      hand_index_t next = accum + indexer->configuration_to_offset[i][j];
      indexer->configuration_to_offset[i][j] = accum;
      accum = next;
    }
    indexer->round_size[i] = accum;
  }

  memset(indexer->permutations, 0, sizeof(indexer->permutations));
  enumerate_permutations(rounds, cards_per_round, count_permutations, indexer);
  
  for(uint_fast32_t i=0; i<rounds; ++i) {
    indexer->permutation_to_configuration[i] = (uint_fast32_t*)calloc(indexer->permutations[i], sizeof(uint_fast32_t));
    indexer->permutation_to_pi[i] = (uint_fast32_t*)calloc(indexer->permutations[i], sizeof(uint_fast32_t));
    if (!indexer->permutation_to_configuration ||
        !indexer->permutation_to_pi) {
      hand_indexer_free(indexer);
      return false; 
    }
  }

  enumerate_permutations(rounds, cards_per_round, tabulate_permutations, indexer);

  return true;
}



hand_index_t hand_indexer_size(const hand_indexer_t * indexer, uint_fast32_t round) {
  assert(round < indexer->rounds);
  return indexer->round_size[round];
}

void hand_indexer_state_init(const hand_indexer_t * indexer, hand_indexer_state_t * state) {
  memset(state, 0, sizeof(hand_indexer_state_t));
 
  state->permutation_multiplier = 1;
  for(uint_fast32_t i=0; i<SUITS; ++i) {
    state->suit_multiplier[i] = 1;
  }
}
hand_index_t hand_index_next_round(const hand_indexer_t * indexer, const uint8_t cards[], hand_indexer_state_t * state) {
  uint_fast32_t round = state->round++;
  assert(round < indexer->rounds);

  uint_fast32_t ranks[SUITS] = {0}, shifted_ranks[SUITS] = {0};
  for(uint_fast32_t i=0; i<indexer->cards_per_round[round]; ++i) {
    assert(cards[i] < CARDS);                 /* valid card */

    uint_fast32_t rank         = deck_get_rank(cards[i]), suit = deck_get_suit(cards[i]), rank_bit = 1<<rank;
    assert(!(ranks[suit]&rank_bit));
    ranks[suit]               |= rank_bit;
    shifted_ranks[suit]       |= rank_bit>>__builtin_popcount((rank_bit-1)&state->used_ranks[suit]);
  }

  for(uint_fast32_t i=0; i<SUITS; ++i) {
    assert(!(state->used_ranks[i]&ranks[i])); /* no duplicate cards */

    uint_fast32_t used_size    = __builtin_popcount(state->used_ranks[i]), this_size = __builtin_popcount(ranks[i]);
    state->suit_index[i]      += state->suit_multiplier[i]*rank_set_to_index[shifted_ranks[i]];
    state->suit_multiplier[i] *= nCr_ranks[RANKS-used_size][this_size];
    state->used_ranks[i]      |= ranks[i];
  }

  for(uint_fast32_t i=0, remaining=indexer->cards_per_round[round]; i<SUITS-1; ++i) {
    uint_fast32_t this_size          = __builtin_popcount(ranks[i]);
    state->permutation_index        += state->permutation_multiplier*this_size;
    state->permutation_multiplier   *= remaining+1;
    remaining                       -= this_size;
  }

  uint_fast32_t configuration = indexer->permutation_to_configuration[round][state->permutation_index];
  uint_fast32_t pi_index      = indexer->permutation_to_pi[round][state->permutation_index];
  uint_fast32_t equal_index   = indexer->configuration_to_equal[round][configuration];
  hand_index_t offset         = indexer->configuration_to_offset[round][configuration];
  const uint_fast32_t * pi    = suit_permutations[pi_index];

  hand_index_t suit_index[SUITS], suit_multiplier[SUITS];
  for(uint_fast32_t i=0; i<SUITS; ++i) {
    suit_index[i]      = state->suit_index[pi[i]]; 
    suit_multiplier[i] = state->suit_multiplier[pi[i]];
  }
  
  /* sort using an optimal sorting network */
#define swap(u, v) \
  do {\
    if (suit_index[u] > suit_index[v]) {\
      suit_index[u] ^= suit_index[v];\
      suit_index[v] ^= suit_index[u];\
      suit_index[u] ^= suit_index[v];\
    }\
  } while(0)

  hand_index_t index = offset, multiplier = 1;
  for(uint_fast32_t i=0; i<SUITS;) {
    hand_index_t part, size;

    if (i+1 < SUITS && equal[equal_index][i+1]) {
      if (i+2 < SUITS && equal[equal_index][i+2]) {
        if (i+3 < SUITS && equal[equal_index][i+3]) {
          /* four equal suits */
          swap(i, i+1); swap(i+2, i+3); swap(i, i+2); swap(i+1, i+3); swap(i+1, i+2);
          part = suit_index[i] + nCr_groups[suit_index[i+1]+1][2] + nCr_groups[suit_index[i+2]+2][3] + nCr_groups[suit_index[i+3]+3][4];
          size = nCr_groups[suit_multiplier[i]+3][4];
          i += 4;
        } else {
          /* three equal suits */
          swap(i, i+1); swap(i, i+2); swap(i+1, i+2);
          part = suit_index[i] + nCr_groups[suit_index[i+1]+1][2] + nCr_groups[suit_index[i+2]+2][3];
          size = nCr_groups[suit_multiplier[i]+2][3];
          i += 3;
        }
      } else {
        /* two equal suits*/
        swap(i, i+1);
        part = suit_index[i] + nCr_groups[suit_index[i+1]+1][2];
        size = nCr_groups[suit_multiplier[i]+1][2];
        i += 2;
      }
    } else {
      /* no equal suits */
      part = suit_index[i];
      size = suit_multiplier[i];
      i += 1;
    }

    index      += multiplier*part;
    multiplier *= size;
  }

#undef swap

  return index;
}

hand_index_t hand_index_all(const hand_indexer_t * indexer, const uint8_t cards[], hand_index_t indices[]) {
  if (indexer->rounds) {
    hand_indexer_state_t state; hand_indexer_state_init(indexer, &state);

    for(uint_fast32_t i=0, j=0; i<indexer->rounds; j+=indexer->cards_per_round[i++]) {
      indices[i] = hand_index_next_round(indexer, cards+j, &state);
    }

    return indices[indexer->rounds-1];
  }

  return 0;
}

hand_index_t hand_index_last(const hand_indexer_t * indexer, const uint8_t cards[]) {
  hand_index_t indices[MAX_ROUNDS];
  return hand_index_all(indexer, cards, indices);
}



bool hand_unindex(const hand_indexer_t * indexer, uint_fast32_t round, hand_index_t index, uint8_t cards[]) {
  if (round >= indexer->rounds || index >= indexer->round_size[round]) {
    return false;
  }

  uint_fast32_t low = 0, high = indexer->configurations[round], configuration_idx = 0;
  while(low < high) {
    uint_fast32_t mid = (low+high)/2;
    if (indexer->configuration_to_offset[round][mid] <= index) {
      configuration_idx = mid;
      low = mid+1;
    } else {
      high = mid;
    }
  }
  index -= indexer->configuration_to_offset[round][configuration_idx];

  hand_index_t suit_index[SUITS];
  for(uint_fast32_t i=0; i<SUITS;) {
    uint_fast32_t j=i+1; for(; j<SUITS && indexer->configuration[round][configuration_idx][j] == indexer->configuration[round][configuration_idx][i]; ++j) {}
    
    uint_fast32_t suit_size  = indexer->configuration_to_suit_size[round][configuration_idx][i];
    hand_index_t group_size  = nCr_groups[suit_size+j-i-1][j-i];
    hand_index_t group_index = index%group_size; index /= group_size;

    for(; i<j-1; ++i) {
      suit_index[i] = low = floor(exp(log(group_index)/(j-i) - 1 + log(j-i))-j-i); high = ceil(exp(log(group_index)/(j-i) + log(j-i))-j+i+1);
      if (high > suit_size) {
        high = suit_size;
      }
      if (high <= low) {
        low = 0;
      }
      while(low < high) {
        uint_fast32_t mid = (low+high)/2;
        if (nCr_groups[mid+j-i-1][j-i] <= group_index) {
          suit_index[i] = mid;
          low = mid+1;
        } else {
          high = mid;
        }
      }

      //for(suit_index[i]=0; nCr_groups[suit_index[i]+1+j-i-1][j-i] <= group_index; ++suit_index[i]) {}
      group_index -= nCr_groups[suit_index[i]+j-i-1][j-i]; 
    }

    suit_index[i] = group_index; ++i;
  }
  
  uint8_t location[MAX_ROUNDS]; memcpy(location, indexer->round_start, MAX_ROUNDS);
  for(uint_fast32_t i=0; i<SUITS; ++i) {
    uint_fast32_t used = 0, m = 0;
    for(uint_fast32_t j=0; j<indexer->rounds; ++j) {
      uint_fast32_t n              = indexer->configuration[round][configuration_idx][i]>>ROUND_SHIFT*(indexer->rounds-j-1)&ROUND_MASK;
      uint_fast32_t round_size     = nCr_ranks[RANKS-m][n]; m += n;
      uint_fast32_t round_idx      = suit_index[i]%round_size; suit_index[i] /= round_size;
      uint_fast32_t shifted_cards  = index_to_rank_set[n][round_idx], rank_set = 0;
      for(uint_fast32_t k=0; k<n; ++k) {
        uint_fast32_t shifted_card = shifted_cards&-shifted_cards; shifted_cards ^= shifted_card;
        uint_fast32_t card         = nth_unset[used][__builtin_ctz(shifted_card)]; rank_set |= 1<<card;
        cards[location[j]++]       = deck_make_card(i, card);
      }
      used |= rank_set;
    }
  }

  return true;
}





const char HandRanks[][16] = {
  "BAD!!",//0
  "High Card",//1
  "Pair",//2
  "Two Pair",//3
  "Three of a Kind",//4
  "Straight",//5
  "Flush",//6
  "Full House",//7
  "Four of a Kind",//8
  "Straight Flush"//9
 };

class EHSLookup {
  hand_indexer_t indexer[4];
  std::vector<std::vector<float>> lookup;

public:
  explicit EHSLookup(const char *filename) : lookup(4) {
	  uint8_t iiii[] = {2};
	  uint8_t iiiii[] = {2,3};
	  uint8_t iiiiii[] = {2,4};
	  uint8_t iiiiiii[] = {2,5};
    assert(hand_indexer_init(1, iiii, &indexer[0]));
    assert(hand_indexer_init(2, iiiii, &indexer[1]));
    assert(hand_indexer_init(2, iiiiii, &indexer[2]));
    assert(hand_indexer_init(2, iiiiiii, &indexer[3]));

    if (!load_lookup(filename))
      throw std::runtime_error("EHSLookup file could not be loaded.");
  }

  float raw(int round, size_t idx) { return lookup[round][idx]; }

  size_t size(int round) { return lookup[round].size(); }

  bool load_lookup(const char *filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);

    size_t round_size;
    for (size_t i = 0; i < 4; ++i) {
      round_size = indexer[i].round_size[i == 0 ? 0 : 1];
      lookup[i] = std::vector<float>(round_size);
      file.read(reinterpret_cast<char *>(&lookup[i][0]),
                sizeof(float) * round_size);
    }
    file.close();
    return true;
  }

  ~EHSLookup() {}
};
 
 //For the river, rank = HR[u6 + c7] for the hand rank, for 5 card you have to use rank = HR[HR[u4 + c5]], 
 //and for 6 card you have to use rank = HR[HR[u5 + c6]]. This extra array reference in pre-river lookups threw me.
// Cards in RayW implementation are 2c = 1, 2d = 2, ..., As = 52
// The one and only lookup table.
int HR[32487834];

// Initialize the 2+2 evaluator by loading the HANDRANKS.DAT file and
// mapping it to our 32-million member HR array. Call this once and
// forget about it.
int InitTheEvaluator()
{
    memset(HR, 0, sizeof(HR));
    FILE * fin = fopen("handranks.dat", "rb");
    // Load the HANDRANKS.DAT file data into the HR array
    size_t bytesread = fread(HR, sizeof(HR), 1, fin);
    fclose(fin);
}

// Given a group of 7 cards, return the hand category & rank. Let
// pCards be (a pointer to) an array of seven integers, each with
// a value between 1 and 52.
int GetHandValue_7_cards(int* pCards)
{
    int p = HR[53 + *pCards++];
    p = HR[p + *pCards++];
    p = HR[p + *pCards++];
    p = HR[p + *pCards++];
    p = HR[p + *pCards++];
    p = HR[p + *pCards++];
    return HR[p + *pCards++];
}

int GetHandValue_5_cards(int* pCards)
{
   int p = HR[53 + *pCards++];
   p = HR[p + *pCards++];
   p = HR[p + *pCards++];
   p = HR[p + *pCards++];
   p = HR[p + *pCards++];
   return HR[p];
}

int GetHandValue_6_cards(int* pCards)
{
   int p = HR[53 + *pCards++];
   p = HR[p + *pCards++];
   p = HR[p + *pCards++];
   p = HR[p + *pCards++];
   p = HR[p + *pCards++];
   p = HR[p + *pCards++];
   return HR[p];
}

// Set up a 7-card poker hand, perform the lookup into the table, and
// extract the category (full house, two pair, flush, etc.) and rank
// (within that category) of the hand. These numbers can be used to
// compare the hand to other hands.
void DoSomeWork()
{
    int myCards[] = { 1, 5, 9, 13, 51, 49 };
    int handInfo = GetHandValue_6_cards(myCards);
    int handCategory = handInfo >> 12;
    int rankWithinCategory = handInfo & 0x00000FFF;
	//for (int i = 0; i < 7; i++) cout << myRanks[i] << ' ';
	//cout << endl;
	cout << handInfo << ' ' << handCategory << ' ' << rankWithinCategory << endl;
}

int convert(string s) {
	char rank = s[0];
	char suit = s[1];
	string ranks = "23456789TJQKA";
	string suits = "cdhs";
	int r, ss;
	for (int i = 0; i < 13; i++)
		if (rank == ranks[i])
			r = i;
	for (int i = 0; i < 4; i++)
		if (suit == suits[i])
			ss = i;
	return 4 * r + ss + 1;
}

// O(N^4)
double HandStrength(int* cards) {
	int my_cards[7] = {cards[0], cards[1], cards[2], cards[3], cards[4], 0, 0};
	int opp_cards[7] = {cards[2], cards[3], cards[4], 0, 0, 0, 0};
	int ahead = 0, behind = 0, tied = 0;
	vector<bool> used(52, false);
	for (int i = 0; i < 5; i++)
		used[cards[i]] = true;
	for (int i = 1; i <= 52; i++) {
		if (used[i]) continue;
		used[i] = true;
		for (int j = 1; j <= 52; j++) {
			if (used[j]) continue;
			opp_cards[3] = i;
			opp_cards[4] = j;
			used[j] = true;
			for (int turn = 1; turn <= 52; turn++) {
				if (used[turn]) continue;
				used[turn] = true;
				for (int river = 1; river <= 52; river++) {
					if (used[river]) continue;
					opp_cards[5] = turn;
					opp_cards[6] = river;
					my_cards[5] = turn;
					my_cards[6] = river;
					int our_rank = GetHandValue_7_cards(my_cards);
					int opp_rank = GetHandValue_7_cards(opp_cards);
					if (our_rank > opp_rank) ahead++;
					else if (our_rank == opp_rank) tied++;
					else behind++;
				}
				used[turn] = false;
			}
			used[j] = false;
		}
		used[i] = false;
	}
	return (0.5*tied+ahead) / (ahead+tied+behind);
}

pair<double,pair<double,double> > HandStrengthAndPotential(int* cards) {
	int ahead = 0, behind = 1, tied = 2;
	int HP[3][3];
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			HP[i][j] = 0;
	int HPTotal[3] = {0};
	int my_cards[7] = {cards[0], cards[1], cards[2], cards[3], cards[4], 0, 0};
	int opp_cards[7] = {cards[2], cards[3], cards[4], 0, 0, 0, 0};
	int ahead_cnt = 0, behind_cnt = 0, tied_cnt = 0;
	int ourrank = GetHandValue_5_cards(my_cards);
	vector<bool> used(53, false);
	for (int i = 0; i < 5; i++)
		used[cards[i]] = true;
	for (int i = 1; i <= 52; i++) {
		if (used[i]) continue;
		used[i] = true;
		for (int j = 1; j <= 52; j++) {
			if (used[j]) continue;
			opp_cards[3] = i;
			opp_cards[4] = j;
			used[j] = true;
			
			int opprank = GetHandValue_5_cards(opp_cards);
			int index;
			if (ourrank > opprank) index = ahead;
			else if (ourrank == opprank) index = tied;
			else index = behind;
			
			
			for (int turn = 1; turn <= 52; turn++) {
				if (used[turn]) continue;
				used[turn] = true;
				for (int river = 1; river <= 52; river++) {
					if (used[river]) continue;
					HPTotal[index]++;
					opp_cards[5] = turn;
					opp_cards[6] = river;
					my_cards[5] = turn;
					my_cards[6] = river;
					int our_best = GetHandValue_7_cards(my_cards);
					int opp_best = GetHandValue_7_cards(opp_cards);
					if (our_best > opp_best) HP[index][ahead]++,ahead_cnt++;
					else if (our_best == opp_best) HP[index][tied]++,tied_cnt++;
					else HP[index][behind]++,behind_cnt++;
				}
				used[turn] = false;
			}
			used[j] = false;
		}
		used[i] = false;
	}
	double hs = (0.5*tied_cnt+ahead_cnt) / (ahead_cnt+tied_cnt+behind_cnt);
	double ppot = 1.0*(HP[behind][ahead] + 0.5*HP[behind][tied] + 0.5*HP[tied][ahead]) / (HPTotal[behind] + HPTotal[tied]);
	double npot = 1.0*(HP[ahead][behind] + 0.5*HP[tied][behind] + 0.5*HP[ahead][tied]) / (HPTotal[ahead] + HPTotal[tied]);
	return make_pair(hs,make_pair(ppot, npot));
}

double EHS(int* cards) {
	auto tmp = HandStrengthAndPotential(cards);
	double HS = tmp.first;
	double PPOT = tmp.second.first;
	double NPOT = tmp.second.second;
	double EHS = HS*(1-NPOT)+(1-HS)*PPOT;
	cout << HS+(1-HS)*PPOT << endl;
	cout << "(HS, PPOT, NPOT, EHS) = (" << HS <<", "<<PPOT<<", "<<NPOT<<", "<<EHS<<")"<<endl;
	return EHS;
}

void MakeHistogram()
{
	vector<string> cards = {"4s", "4h", "Kd", "4c", "As"};
	
	int my_cards[7];
	int ptr = 0;
	for (int i = 0; i < 5; i++)
		my_cards[ptr++] = convert(cards[i]);
	for (int i = 0; i < 5; i++)
		cout << my_cards[i] << ' ';
	cout << endl;
	cout << HandStrength(my_cards) << endl;
	cout << EHS(my_cards) << endl;
	/*for (int turn = 1; turn <= 52; turn++) {
		bool cont = false;
		for (int i = 0; i < 5; i++)
			if (turn == my_cards[i])
				cont = true;
		if (cont) continue;
		for (int river = 1; river <= 52; river++) {
			if (turn == river) continue;
			bool cont = false;
			for (int i = 0; i < 5; i++)
				if (river == my_cards[i])
					cont = true;
			if (cont) continue;
			my_cards[ptr++] = turn;
			my_cards[river++] = river;
			
		}
	}*/
			
	
}

hand_indexer_t indexer[4];
uint8_t iiii[] = {2};
uint8_t iiiii[] = {2,3};
uint8_t iiiiii[] = {2,4};
uint8_t iiiiiii[] = {2,5};
EHSLookup* ehslp;
vector<double> getBuckets(string x, string y) {
	uint8_t cards[7];
	cards[0] = convert(x)-1;
	cards[1] = convert(y)-1;
	vector<bool> used(53, false);
	used[cards[0]]=true;
	used[cards[1]]=true;
	vector<double> bucket(51,0.0);
	for(int f1=0;f1<52;f1++){
		if (used[f1])continue;
		used[f1]=true;
		for(int f2=f1+1;f2<52;f2++){
			if(used[f2])continue;
			used[f2]=true;
			for(int f3=f2+1;f3<52;f3++){
				if(used[f3])continue;
				used[f3]=true;
				for (int t=f3+1;t<52;t++){
					if(used[t])continue;
					used[t]=true;
					for (int r=t+1;r<52;r++) {
						if(used[r])continue;
						used[r]=true;
						cards[2]=f1;
						cards[3]=f2;
						cards[4]=f3;
						cards[5]=t;
						cards[6]=r;
						hand_index_t hit = hand_index_last(&indexer[3], cards);
						bucket[(int)(ehslp->raw(3, hit)*50)]+=1;
						used[r]=false;
					}
					
					used[t]=false;
				}
				
				
				used[f3]=false;
			}
			used[f2]=false;
		}
		used[f1]=false;
	}
	double sum = 0;
	for(int i = 0; i < 51;i++)
		sum+=bucket[i];
	for (int i = 0; i < 51; i++)
		bucket[i]/=sum;
	return bucket;
}

vector<double> GetHistogram(int street, array<int, 2> hand, array<int, 5> board) {
		assert(street == 3 || street == 4);
		bitset<52> used;
		used[hand[0]] = 1;
		used[hand[1]] = 1;
		for (int i = 0; i < street; i++)
			used[board[i]] = 1;
		uint8_t cards[7];
		cards[0] = hand[0];
		cards[1] = hand[1];
		cards[2] = board[0];
		cards[3] = board[1];
		cards[4] = board[2];
		vector<double> bucket(51, 0.0);
		if (street == 3) {
			for (int i = 0; i < 52; i++) {
				if (used[i]) continue;
				for (int j = i + 1; j < 52; j++) {
					if (used[j]) continue;
					cards[5] = i;
					cards[6] = j;
					hand_index_t hit = hand_index_last(&indexer[3], cards);
					bucket[(int)(ehslp->raw(3, hit) * 50)] += 1;
				}
			}
		}
		else {
			// street == 4
			cards[5] = board[3];
			for (int i = 0; i < 52; i++) {
				if (used[i]) continue;
				cards[6] = i;
				hand_index_t hit = hand_index_last(&indexer[3], cards);
				bucket[(int)(ehslp->raw(3, hit) * 50)] += 1;
			}
		}
		double sum = 0;
		for (int i = 0; i < 51; i++)
			sum += bucket[i];
		for (int i = 0; i < 51; i++)
			bucket[i] /= sum;
		return bucket;
	}

double EMD(const vector<double> &a, const vector<double> &b) {
	assert(a.size() == b.size());
	int n = a.size();
	double res = 0;
	double cur = 0;
	for (int i = 0; i < n; i++) {
		cur = a[i] + cur - b[i];
		res += abs(cur);
	}
	return res;
}

int main() {
	//InitTheEvaluator();
	//DoSomeWork();
	//MakeHistogram();
	auto TIME = clock();
cout <<"here"<<endl;
	ehslp = new EHSLookup("ehs.dat");
	for(int i = 0; i < 4; i++)
		cout << ehslp->size(i) << endl;
	/*uint8_t newcards[7];
      newcards[0] = 20;
      newcards[1] = 1;
      newcards[2] = 2;
      newcards[3] =3;
      newcards[4] = 10;
	  newcards[5] = 0;
	  newcards[6] = 1;*/
	    
    assert(hand_indexer_init(1, iiii, &indexer[0]));
    assert(hand_indexer_init(2, iiiii, &indexer[1]));
    assert(hand_indexer_init(2, iiiiii, &indexer[2]));
    assert(hand_indexer_init(2, iiiiiii, &indexer[3]));
	cout << "time to load ehs.dat: " << 1000.0*(clock()-TIME)/CLOCKS_PER_SEC << "ms" << endl;
	map<int, bool> used;
	bitset<52> bs;
	auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
	mt19937 rng(seed);
	uniform_int_distribution<int> distribution(0, 51);
	ofstream fout("flophistograms.txt");
	for (int iter = 0; iter < 200000; iter++) {
		if (iter % 10000 == 0) cout << iter << endl;
		int a[7];
		for (int i = 0; i < 5; i++) {
			int x;
			do {
				x = distribution(rng);
			} while(bs[x]);
			a[i] = x;
			bs[x] = 1;
		}
		array<int, 2> my_hand = {a[0], a[1]};
		array<int, 5> board = {a[2], a[3], a[4], 0, 0};
		vector<double> hist = GetHistogram(3, my_hand, board);
		for (int i = 0; i < 51; i++)
			fout << hist[i] << ' ';
		fout << "\n";
		bs.reset();
		//for (int i = 0; i < 7; i++) bs[a[i]] = 0;
	}
	/*cout << EMD(getBuckets("4s","4h"),getBuckets("Ts","Js")) << endl;
	cout << EMD(getBuckets("4s","4h"),getBuckets("6s","6h")) << endl;
	cout << EMD(getBuckets("4s","4h"),getBuckets("Qs","Ks")) << endl;
	cout << EMD(getBuckets("6s","6h"),getBuckets("Qs","Ks")) << endl;
	cout << EMD(getBuckets("6s","6h"),getBuckets("Ts","Js")) << endl;
	cout << EMD(getBuckets("Ts","Js"),getBuckets("Qs","Ks")) << endl;*/
	
      //hand_index_t fdsa = hand_index_last(&indexer[1],newcards);
	  //cout << ehslp->raw(1,fdsa)<<endl;
	/*
	for (int i = 0; i < 51; i++)
		cout << i*20 <<": "<<1.0*bucket[i]/sum << endl;
	cout<<endl;
	for (int i = 0; i < 51; i++) {
		cout << i*20 <<": ";
		for(int j = 0; j <bucket[i]/(300*50); j++)
			cout << "x";
		cout<<endl;
	}
	cout<<endl;*/
	return 0;
}
