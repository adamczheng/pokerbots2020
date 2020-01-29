#include "player.hpp"
#include "LUT.h"
#include <bits/stdc++.h>
using namespace std;

const int NUM_LINES_STRATEGY = 128329;
// LUT
int HR[32487834];
int GetHandValue(array<int, 7> cards) {
    int p = HR[53 + cards[0] + 1];
    p = HR[p + cards[1] + 1];
    p = HR[p + cards[2] + 1];
    p = HR[p + cards[3] + 1];
    p = HR[p + cards[4] + 1];
    p = HR[p + cards[5] + 1];
    return HR[p + cards[6] + 1];
}

int seen_pot_sizes[500];

class Buckets {
	string pf_cluster[8] = { "23s,24s,25s,26s,27s,34s,35s,36s,37s,45s,46s,32o,43o,42o,54o,53o,52o,65o,64o,63o,62o,74o,73o,72o,83o,82o","28s,29s,2Ts,38s,39s,47s,48s,49s,75o,85o,84o,95o,94o,93o,92o,T5o,T4o,T3o,T2o,J3o,J2o","3Ts,4Ts,56s,57s,58s,59s,5Ts,67s,68s,69s,6Ts,78s,79s,89s,67o,68o,69o,6To,78o,79o,7To,89o,8To","22,J2s,J3s,J4s,J5s,J6s,Q2s,Q3s,Q4s,Q5s,K2s,J4o,J5o,J6o,J7o,Q2o,Q3o,Q4o,Q5o,Q6o,Q7o,K2o,K3o,K4o","6Qs,7Ts,7Js,7Qs,8Ts,8Js,8Qs,9Ts,9Js,9Qs,TJs,T9o,J8o,J9o,JTo,Q8o,Q9o,QTo,QJo","33,44,55,K3s,K4s,K5s,K6s,K7s,K8s,A2s,A3s,A4s,A5s,A6s,K5o,K6o,K7o,K8o,K9o,A2o,A3o,A4o,A5o,A6o,A7o,A8o","66,77,QTs,QJs,K9s,KTs,KJs,KQs,A7s,A8s,A9s,ATs,AJs,AQs,AKs,KTo,KJo,KQo,A9o,ATo,AJo,AQo,AKo","88,99,TT,JJ,QQ,KK,AA" };
	vector<array<int, 2> > preflop_cluster[8];
	// AKo or AKs or AA to a vector of card indices (0-indexed)
	vector<array<int, 2> > convert(string s) {
		string ranks = "23456789TJQKA";
		string suits = "shcd";
		map<char, int> rank_rev, suit_rev;
		int i = 0;
		for (char c : ranks) rank_rev[c] = i++;
		i = 0;
		for (char c : suits) suit_rev[c] = i++;
		vector<array<int, 2> > res;
		if (s.size() == 2) {
			assert(s[0] == s[1]);
			// pocket pairs
			int f = 4 * rank_rev[s[0]];
			for (int i = 0; i < 4; i++)
				for (int j = i + 1; j < 4; j++)
					res.push_back(array<int, 2>{f + i, f + j});
			return res;
		}
		assert(s.size() == 3);
		int r1 = 4 * rank_rev[s[0]];
		int r2 = 4 * rank_rev[s[1]];
		if (s.back() == 's') {
			// suited
			for (int i = 0; i < 4; i++)
				res.push_back(array<int, 2>{r1 + i, r2 + i});
		}
		else {
			assert(s.back() == 'o');
			// unsuited
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					if (i == j) continue;
					res.push_back(array<int, 2>{r1 + i, r2 + j});
				}
			}
		}
		return res;
	}
	void split_clusters() {
		for (int i = 0; i < 8; i++) {
			vector<string> split_cluster;
			istringstream ss(pf_cluster[i]);
			string token;
			while (getline(ss, token, ',')) {
				split_cluster.push_back(token);
				//cout << token << endl;
			}
			for (string s : split_cluster) {
				vector<array<int, 2> > tmp = convert(s);
				for (array<int, 2> x : tmp) preflop_cluster[i].push_back(x);
			}
		}
	}

	ifstream f_in, t_in, r_in;
	vector<vector<double> > flop_centers, turn_centers, river_centers;
public:
    CLutRiver* river;
	Buckets() {
        river = new CLutRiver();
        
		river->read_suitfile("river_suit.dat");
		river->load("river_lut.dat");
		f_in.open("flopcenters.txt");
		for (int i = 0; i < 100; i++) {
			vector<double> p(51);
			for (int j = 0; j < 51; j++) {
				f_in >> p[j];
			}
			flop_centers.push_back(p);
		}
		f_in.close();
		t_in.open("turncenters.txt");
		for (int i = 0; i < 100; i++) {
			vector<double> p(51);
			for (int j = 0; j < 51; j++) {
				t_in >> p[j];
			}
			turn_centers.push_back(p);
		}
		t_in.close();
		r_in.open("rivercenters.txt");
		for (int i = 0; i < 100; i++) {
			vector<double> p(8);
			for (int j = 0; j < 8; j++) {
				r_in >> p[j];
			}
			river_centers.push_back(p);
		}
		r_in.close();
        split_clusters();
	}
	static int GetBucketCount(int street) {
		if (street == 0) {
			return 169;
		}
		else if (street == 3) {
			return 100;
		}
		else if (street == 4) {
			return 100;
		}
		else {
			// street == 5
			return 100;
		}
	}
	vector<double> GetHistogram(int street, array<int, 2> hand, array<int, 5> board) {
		assert(street == 3 || street == 4);
		bitset<52> used;
		used[hand[0]] = 1;
		used[hand[1]] = 1;
		for (int i = 0; i < street; i++)
			used[board[i]] = 1;
		vector<double> bucket(51, 0.0);
		if (street == 3) {
			for (int i = 0; i < 52; i++) {
				if (used[i]) continue;
				for (int j = i + 1; j < 52; j++) {
					if (used[j]) continue;
					int arr[] = { board[0], board[1], board[2], i, j };
					bucket[(int)(river->data[river->g_index(hand[0], hand[1], arr)] * 50)] += 1;
				}
			}
		}
		else {
			// street == 4
			for (int i = 0; i < 52; i++) {
				if (used[i]) continue;
				int arr[] = { board[0], board[1], board[2], board[3], i };
				bucket[(int)(river->data[river->g_index(hand[0], hand[1], arr)] * 50)] += 1;
			}
		}
		double sum = 0;
		for (int i = 0; i < 51; i++)
			sum += bucket[i];
		for (int i = 0; i < 51; i++)
			bucket[i] /= sum;
		return bucket;
	}
	double EMD(const vector<double>& a, const vector<double>& b) {
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
	vector<double> GetOCHS(array<int, 2> hand, array<int, 5> board) {
		// only on river
		bitset<52> used;
		used[hand[0]] = 1;
		used[hand[1]] = 1;
		for (int i = 0; i < 5; i++)
			used[board[i]] = 1;
		array<int, 7> schand = { board[0], board[1], board[2], board[3], board[4], hand[0], hand[1] };
		int my_strength = GetHandValue(schand);
		vector<double> res;
		for (int cluster = 0; cluster < 8; cluster++) {
			int tot = 0;
			int my_tot = 0;
			for (array<int, 2> opp_hand : preflop_cluster[cluster]) {
				if (used[opp_hand[0]] || used[opp_hand[1]])
					continue;
				schand[5] = opp_hand[0];
				schand[6] = opp_hand[1];
				int opp_strength = GetHandValue(schand);
				if (my_strength > opp_strength) {
					my_tot += 2;
				}
				else if (my_strength == opp_strength) {
					my_tot++;
				}
				tot += 2;
			}
			res.push_back(1.0 * my_tot / tot);
		}
		return res;
	}
	double GetSquaredEuclideanDist(const vector<double>& a, const vector<double>& b) {
		assert(a.size() == 8 && b.size() == 8);
		double res = 0;
		for (int i = 0; i < 8; i++)
			res += (a[i] - b[i]) * (a[i] - b[i]);
		return res;
	}
	int GetBucket(int street, array<int, 2> hand, array<int, 5> board) {
		if (street == 0) {
			int r1 = hand[0] / 4;
			int r2 = hand[1] / 4;
			bool suited = ((hand[0] % 4) == (hand[1] % 4));
			if (r1 > r2) std::swap(r1, r2);
			if (r1 == r2) {
				return r1;
			}
			int os[12] = { 13, 25, 36, 46, 55, 63, 70, 76, 81, 85, 88, 90 };
			int ss[12] = { 91, 103, 114, 124, 133, 141, 148, 154, 159, 163, 166, 168 };
			if (suited) {
				return ss[r1] + r2 - r1 - 1;
			}
			return os[r1] + r2 - r1 - 1;
		}
		else if (street == 3) {
			// create histogram
			vector<double> hist = GetHistogram(street, hand, board);
			// return closest (emd) center from generated flop centers
			assert(flop_centers.size());
			int best_center = 0;
			double best_dist = EMD(hist, flop_centers[0]);
			for (int i = 1; i < 100; i++) {
				double cur_dist = EMD(hist, flop_centers[i]);
				if (cur_dist < best_dist) {
					best_dist = cur_dist;
					best_center = i;
				}
			}
			return best_center;
		}
		else if (street == 4) {
			// create histogram
			vector<double> hist = GetHistogram(street, hand, board);
			// return closest (emd) center from generated turn centers
			assert(turn_centers.size());
			int best_center = 0;
			double best_dist = EMD(hist, turn_centers[0]);
			for (int i = 1; i < 100; i++) {
				double cur_dist = EMD(hist, turn_centers[i]);
				if (cur_dist < best_dist) {
					best_dist = cur_dist;
					best_center = i;
				}
			}
			return best_center;
		}
		else {
			// street == 5
			// compute equities against 8 opponent hand clusters (1326 lookups)
			vector<double> OCHS = GetOCHS(hand, board);
			// return closest (l2) center from generated river centers
			assert(river_centers.size());
			int best_center = 0;
			double best_dist = GetSquaredEuclideanDist(OCHS, river_centers[0]);
			for (int i = 1; i < (int)river_centers.size(); i++) {
				double cur_dist = GetSquaredEuclideanDist(OCHS, river_centers[i]);
				if (cur_dist < best_dist) {
					best_dist = cur_dist;
					best_center = i;
				}
			}
			return best_center;
		}
	}
};
int hand_to_int(string s) {
    string ranks = "23456789TJQKA";
    string suits = "cdhs";
    map<char, int> rmap, smap;
    int i = 0;
    for (char c : ranks) rmap[c] = i++;
    i = 0;
    for (char c : suits) smap[c] = i++;
    return 4 * rmap[s[0]] + smap[s[1]];
}
// permutation generator
long long perm_to_mask(const vector<int> &perm) {
    long long res = 0;
    for (int i = 0; i < 13; i++) {
        res *= 16;
        res += perm[i];
    }
    return res;
}
vector<int> mask_to_perm(long long mask) {
    vector<int> res(13);
    for (int i = 0; i < 13; i++) {
        res[12-i] = mask % 16;
        mask /= 16;
    }
    return res;
}
/*vector<int> permute_values() {
    vector<int> orig_perm = {12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    vector<int> prop_perm;
    mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    geometric_distribution<int> distribution(0.25);
    for (int i = 0; i < 13; i++) {
        int pop_i = orig_perm.size() - 1 - (distribution(rng) % orig_perm.size());
        prop_perm.push_back(orig_perm[pop_i]);
        orig_perm.erase(orig_perm.begin() + pop_i);
    }
    return prop_perm;
}*/

double prior_prob[13][13];
void compute_prior_probs() {
    for (int i = 0; i < 13; i++) {
        double sum = 0;
        double prob = 0.25;
        for (int j = 0; j < 13 - i; j++) {
            if (j > 0) prob *= 0.75;
            prior_prob[i][j] = prob;
            sum += prior_prob[i][j];
        }
        for (int j = 0; j < 13 - i; j++) {
            prior_prob[i][j] /= sum;
        }
    }
}
double perm_prob(const vector<int> &guess) {
    double res = 1;
    vector<int> editing(13);
    for (int i = 0; i < 13; i++) {
        editing[i] = i;
    }
    for (int i = 0; i < 13; i++) {
        int id = -1;
        for (int j = 0; j < (int)editing.size(); j++) {
            if (editing[j] == guess[i]) {
                id = j;
                break;
            }
        }
        assert(id != -1);
        res *= prior_prob[i][id];
        editing.erase(editing.begin() + id);
    }
    return res;
}

int convert(const vector<int> &perm, int card) {
    for (int i = 0; i < 13; i++) {
        if (perm[i] == card / 4) {
            return 4 * i + (card % 4);
        }
    }
    assert(0);
}
mt19937 rng;
vector<int> gen_candidate(const vector<int> &guess) {
    // randomly reinsert
    uniform_int_distribution<int> distribution(0, 12);
    int x = distribution(rng), y = distribution(rng);
    vector<int> res;
    if (x < y) {
        for (int i = 0; i < 13; i++) {
            if (i < x) 
                res.push_back(guess[i]);
            else if (i >= x && i < y)
                res.push_back(guess[i + 1]);
            else if (i == y)
                res.push_back(guess[x]);
            else
                res.push_back(guess[i]);
        }
    } else if (x > y) {
        for (int i = 0; i < 13; i++) {
            if (i < y)
                res.push_back(guess[i]);
            else if (i == y)
                res.push_back(guess[x]);
            else if (i > y && i <= x)
                res.push_back(guess[i - 1]);
            else
                res.push_back(guess[i]);
        }
    } else {
        for (int i = 0; i < 13; i++) {
            res.push_back(guess[i]);
        }
    }
    return res;
}
int preflop_delta;
vector<pair<pair<array<int, 7>, array<int, 7> >, int> > showdown_rules;

int count_fails(const vector<int> &guess) {
    int fails = 0;
    for (pair<pair<array<int, 7>, array<int, 7> >, int> pp : showdown_rules) {
        array<int, 7> h1 = pp.first.first;
        array<int, 7> h2 = pp.first.second;
        int result = pp.second;
        for (int i = 0; i < 7; i++) {
            h1[i] = convert(guess, h1[i]);
            h2[i] = convert(guess, h2[i]);
        }
        int s1 = GetHandValue(h1);
        int s2 = GetHandValue(h2);
        int new_result = 0;
        if (s1 > s2) new_result = 1;
        if (s1 < s2) new_result = 2;
        if (result != new_result) {
            fails++;
        }
    }
    return fails;
}
vector<int> getinverse(const vector<int> &perm) {
    vector<int> inv(13);
    for (int i = 0; i < 13; i++) {
        inv[perm[i]] = i;
    }
    return inv;
}

vector<int> guess, guessinv;
bool use_guess;
int last_guess_mask;
int guess_fails = 0;
void metropolis_hastings() {
    guess_fails = count_fails(guess);
    int TOT_ITERS = 5000;
    double REJECTION_PROB = 0.04;
    map<long long, int> visits;
    for (int iter = 0; iter < TOT_ITERS; iter++) {
        //cout << iter << endl;
        vector<int> candidate = gen_candidate(guess);
       // cout << iter << endl;
        int candidate_fails = count_fails(candidate);
        double prior_ratio = perm_prob(getinverse(candidate)) / perm_prob(getinverse(guess));
        double showdown_ratio = pow(REJECTION_PROB, candidate_fails) / pow(REJECTION_PROB, guess_fails);
        double alpha = min(1.0, prior_ratio * showdown_ratio);
        uniform_real_distribution<double> distrib(0, 1);
        if (distrib(rng) < alpha) {
            // accept
            guess = candidate;
            guess_fails = candidate_fails;
        }
        if (iter >= TOT_ITERS / 4) {
            visits[perm_to_mask(guess)]++;
        }
    }
    long long best = -1;
    for (auto it : visits) {
        if (best == -1) best = it.first;
        else if (it.second > visits[best]) best = it.first;
    }
    assert(best != -1);
    guess = mask_to_perm(best);
    guessinv = getinverse(guess);
    string ranks = "23456789TJQKA";
    for (int x:guessinv)cout<<ranks[x]<<' ';
    cout<<'\n';
    if (showdown_rules.size() == 69 || best == last_guess_mask) {
        cout << "using inferred permutation now" << endl;
        //use_guess = true; /*UNCOMMENT TO USE INFERENCE AT ALL*/
    }
    last_guess_mask = perm_to_mask(guess);
}

int all_in_pre_cnt;
Buckets* bucketer;
map<pair<int, int >, vector<pair<pair<char,int>, double> > > strategy_map[7][200][7];

/**
 * Called when a new game starts. Called exactly once.
 */
Player::Player()
{
    auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
	rng.seed(seed);
    memset(HR, 0, sizeof(HR));
    FILE* hrfin = fopen("handranks.dat", "rb");
    fread(HR, sizeof(HR), 1, hrfin);
    fclose(hrfin);
    ifstream fin("strategy.txt");
    string line;
    getline(fin, line);
    for (int i = 1; i < NUM_LINES_STRATEGY; i++) {
        getline(fin, line);
        istringstream ss(line);
        int street, infoset, player, pot;
        int button;
        int pot_odd_round;
        //array<int, 2> pips;
        ss >> street >> infoset /*>> player*/ >> button>> pot >> pot_odd_round;//pips[0] >> pips[1];
        seen_pot_sizes[pot] = 1;
        char colon_dummy;
        ss >> colon_dummy;
        vector<pair<pair<char,int>,double> > strategy;
        char type;
        while (ss >> type) {
            int x;
            double p;
            ss >> x >> p;
            strategy.push_back({{type, x}, p});
        }
        strategy_map/*[player]*/[street][infoset][button][make_pair(pot, pot_odd_round)] = strategy;
    }
    fin.close();
    bucketer = new Buckets();
	all_in_pre_cnt = 0;
    compute_prior_probs();
    guess = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    guessinv = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    use_guess = false; /* DO WE USE INFERENCE? */
    last_guess_mask = perm_to_mask(guess);
}
bool all_in_pre;
bool on_check_fold;
bool big_blind;
int small_pots_won, small_pots_lost, big_pots_won, big_pots_lost, chopped_pots; // small is <= 50 chips
int spw,spl,bpw,bpl,cp; // same as variables above but only for first 500 rounds
vector<int> pf_deltas;
int all_in_pre_delta;

int flop_delta, turn_delta, river_delta, showdown_delta;
int flop_tot, turn_tot, river_tot, showdown_tot;

int my_btn_vpip, opp_btn_vpip;
int my_bb_vpip, opp_bb_vpip;

int imagined_pot;
/**
 * Called when a new round starts. Called NUM_ROUNDS times.
 *
 * @param game_state Pointer to the GameState object.
 * @param round_state Pointer to the RoundState object.
 * @param active Your player's index.
 */
void Player::handle_new_round(GameState* game_state, RoundState* round_state, int active)
{
    //int my_bankroll = game_state->bankroll;  // the total number of chips you've gained or lost from the beginning of the game to the start of this round
    //float game_clock = game_state->game_clock;  // the total number of seconds your bot has left to play this game
    //int round_num = game_state->round_num;  // the round number from 1 to NUM_ROUNDS
    //std::array<std::string, 2> my_cards = round_state->hands[active];  // your cards
    //bool big_blind = (bool) active;  // true if you are the big blind
    int street = round_state->street;  // 0, 3, 4, or 5 representing pre-flop, flop, river, or turn respectively
    big_blind = (bool) active;  // true if you are the big blind
    all_in_pre = false;
    on_check_fold = false;
    assert(street == 0);
    imagined_pot = 4;
}

void handle_showdowns(GameState* game_state, TerminalState* terminal_state, int active){
    if (game_state->game_clock < 10.0) return;
    RoundState* previous_state = (RoundState*) terminal_state->previous_state;  // RoundState before payoffs
    int street = previous_state->street;  // 0, 3, 4, or 5 representing when this round ended
    assert(street == 5);
    std::array<std::string, 2> my_cards = previous_state->hands[active];  // your cards
    std::array<std::string, 2> opp_cards = previous_state->hands[1-active];  // opponent's cards or "" if not revealed
    int my_delta = terminal_state->deltas[active];  // your bankroll change from this round

    array<int, 7> my_hand = {hand_to_int(my_cards[0]), hand_to_int(my_cards[1]), 0, 0, 0, 0, 0};
    array<int, 7> opp_hand = {hand_to_int(opp_cards[0]), hand_to_int(opp_cards[1]), 0, 0, 0, 0, 0};
    std::array<std::string, 5> board_cards = previous_state->deck;
    for (int i = 0; i < 5; i++) {
        my_hand[i+2]=hand_to_int(board_cards[i]);
        opp_hand[i+2]=hand_to_int(board_cards[i]);
    }
    //string ranks = "23456789TJQKA";
    //cout << game_state->round_num << ' ' << "we think we have: " << ranks[guessinv[my_hand[0]/4]] << ' ' << ranks[guessinv[my_hand[1]/4]] << endl;
    if (my_delta > 0) {
        showdown_rules.push_back(make_pair(make_pair(my_hand, opp_hand), 1));
    } else if (my_delta < 0) {
        showdown_rules.push_back(make_pair(make_pair(my_hand, opp_hand), 2));
    } else {
        showdown_rules.push_back(make_pair(make_pair(my_hand, opp_hand), 0));
    }
    metropolis_hastings();
}

/**
 * Called when a round ends. Called NUM_ROUNDS times.
 *
 * @param game_state Pointer to the GameState object.
 * @param terminal_state Pointer to the TerminalState object.
 * @param active Your player's index.
 */
void Player::handle_round_over(GameState* game_state, TerminalState* terminal_state, int active)
{
    //RoundState* previous_state = (RoundState*) terminal_state->previous_state;  // RoundState before payoffs
    RoundState* previous_state = (RoundState*) terminal_state->previous_state;  // RoundState before payoffs
    int street = previous_state->street;  // 0, 3, 4, or 5 representing when this round ended
    std::array<std::string, 2> my_cards = previous_state->hands[active];  // your cards
    std::array<std::string, 2> opp_cards = previous_state->hands[1-active];  // opponent's cards or "" if not revealed
    int my_delta = terminal_state->deltas[active];  // your bankroll change from this round
    //int my_pip = previous_state->pips[active];
    //int opp_pip = previous_state->pips[active];
    int my_stack = previous_state->stacks[active];
    int opp_stack = previous_state->stacks[1-active];
    int my_contribution = STARTING_STACK - my_stack;
    int opp_contribution = STARTING_STACK - opp_stack;
    
    if (big_blind) {
        if (my_contribution > 2) my_bb_vpip++;
        if (opp_contribution > 1) opp_btn_vpip++;
    } else {
        if (my_contribution > 1) my_btn_vpip++;
        if (opp_contribution > 2) opp_bb_vpip++;
    }
    
    if (!all_in_pre) {
        if (street == 3) {
            flop_delta += my_delta;
            flop_tot++;
        } else if (street == 4) {
            turn_delta += my_delta;
            turn_tot++;
        } else if (street == 5) {
            if (my_contribution!=opp_contribution){
                river_delta += my_delta;
                river_tot++;
            } else {
                showdown_delta += my_delta;
                showdown_tot++;
            }
        }
    }
    if (my_delta < -50) {
        big_pots_lost++;
        if (game_state->round_num <= 500) bpl++;
    } else if (my_delta > 50) {
        big_pots_won++;
        if (game_state->round_num <= 500) bpw++;
    } else if (my_delta == 0) {
        chopped_pots++;
        if (game_state->round_num <= 500) cp++;
    } else if (my_delta > 0) {
        small_pots_won++;
        if (game_state->round_num <= 500) spw++;
    } else {
        small_pots_lost++;
        if (game_state->round_num <= 500) spl++;
    }
    if (all_in_pre) {
        all_in_pre_delta += my_delta;
		all_in_pre_cnt++;
    }
    if (street == 0 && !on_check_fold) {
       preflop_delta += my_delta;
    }
    if (game_state->round_num % 100 == 0) {
        pf_deltas.push_back(preflop_delta);
    }
    if (game_state->round_num == 1000) {
        //cout << "pf delta: " << preflop_delta << endl;
        for (int i=0; i<10; i++) {
            cout << "pf delta on round " << (i+1)*100 << ": " << pf_deltas[i] << endl;
        }
        cout << "all-in pre delta:   " << all_in_pre_delta << endl;
        cout << "spw/spl/bpw/bpl/cp: " << small_pots_won << ' ' << small_pots_lost << ' ' << big_pots_won << ' ' << big_pots_lost << ' ' << chopped_pots << endl;
        cout << "first half:         " << spw << ' ' << spl << ' ' << bpw << ' ' << bpl << ' ' << cp << endl;
        cout << "second half:        " << small_pots_won - spw << ' ' << small_pots_lost - spl << ' ' << big_pots_won - bpw << ' ' << big_pots_lost - bpl << ' ' << chopped_pots - cp << endl;
        cout << "flop avg delta:     " << 1.0*flop_delta/flop_tot << "(sample: "<< flop_tot << ")\n";
        cout << "turn avg delta:     " << 1.0*turn_delta/turn_tot << "(sample: "<< turn_tot << ")\n";
        cout << "river avg delta:    " << 1.0*river_delta/river_tot << "(sample: "<< river_tot << ")\n";
        cout << "showdown avg delta: " << 1.0*showdown_delta/showdown_tot << "(sample: "<< showdown_tot << ")\n";
        cout << "opp BN VPIP/BB VPIP:" << (int)(100.0*opp_btn_vpip/500) << ' ' << (int)(100.0*opp_bb_vpip/500) << endl;
        cout << "my BN VPIP/BB VPIP: " << (int)(100.0*my_btn_vpip/500) << ' ' << (int)(100.0*my_bb_vpip/500) << endl;
        string ranks = "23456789TJQKA";
        cout << "final perm guess:\n";
        for (int x:guessinv)cout<<ranks[x]<<' ';
        cout<<'\n';
        cout << "time left: " << (game_state->game_clock) << endl;
    }
    
    if (street == 5 && my_contribution == opp_contribution) {
        handle_showdowns(game_state,terminal_state,active);
    }
}

int SampleStrategy(const vector<pair<pair<char,int>,double> >& s) {
    // fold threshold + purification
    const double FOLD_THRESHOLD = 1; 
    const double IGNORE_THRESHOLD = 0.15;
    double sum = 0;
    double pfold = 0, pcall = 0, pcheck = 0, pbet = 0;
    int foldi = -1, calli = -1, checki = -1;
    for (int i = 0; i < (int)s.size(); i++) {
        
        if (s[i].first.first == 'F' && s[i].second > FOLD_THRESHOLD) {
            return i;
        }
        if (s[i].first.first == 'F') {
            if (s[i].second >= IGNORE_THRESHOLD) sum += s[i].second;
            pfold += s[i].second;
            foldi = i;
        }
        else if (s[i].first.first == 'X') {
            if (s[i].second >= IGNORE_THRESHOLD) sum += s[i].second;
            pcheck += s[i].second;
            checki = i;
        }
        else if (s[i].first.first == 'C') {
            if (s[i].second >= IGNORE_THRESHOLD) sum += s[i].second;
            pcall += s[i].second;
            calli = i;
        }
        if (s[i].first.first == 'B' || s[i].first.first == 'R') {
            if (s[i].second >= IGNORE_THRESHOLD) sum += s[i].second;
            pbet += s[i].second;
        }
    }
    /*if (pfold >= max({pcall, pcheck, pbet})) {
        return foldi;
    } else if (pcall >= max({pfold, pcheck, pbet})) {
        return calli;
    } else if (pcheck >= max({pfold, pcall, pbet})) {
        return checki;
    }*/
    std::uniform_real_distribution<double> distribution(0, sum);
    double r = distribution(rng);
    double acc = 0;
    //int besti = -1;
    for (int i = 0; i < (int)s.size(); i++) {
        //if (s[i].first.first == 'B' || s[i].first.first == 'R') {
            //if (besti == -1) besti = i;
            //else if (s[i].second > s[besti].second) besti = i;
            if (s[i].second >= IGNORE_THRESHOLD) acc += s[i].second;
        //}
        if (r < acc) return i;
    }
    //if (besti != -1) return besti;
    cout << "this should be impossible (in sampling strategy)" << endl;
    return (int)s.size() - 1;
}

int pot_odds_bucket(double odds) {
	if (odds * 6 < 1) return 0;
	if (odds < 0.25) return 1;
	if (odds * 3 < 1) return 2;
	if (odds < 0.4) return 3;
	if (odds * 9 < 4) return 4;
	return 5;
}

int convert_card(string s) {
    int card_we_have = hand_to_int(s);
    int rank_we_think_we_have = card_we_have/4;
    if (use_guess) rank_we_think_we_have = guessinv[card_we_have/4];
    int suit_we_have = card_we_have % 4;
    return 4 * rank_we_think_we_have + suit_we_have;
}

const int DECK_SIZE = 52;
const int NUM_BET_SIZES = 4;
const int NUM_RAISE_SIZES = 4;
const array<double, NUM_BET_SIZES> BET_SIZES = { 0.5, 1.0, 2.0, 999 };
const array<double, NUM_RAISE_SIZES> RAISE_SIZES = { 0.5, 1.0, 2.0, 999 };
/**
 * Where the magic happens - your code should implement this function.
 * Called any time the engine needs an action from your bot.
 *
 * @param game_state Pointer to the GameState object.
 * @param round_state Pointer to the RoundState object.
 * @param active Your player's index.
 * @return Your action.
 */
Action Player::get_action(GameState* game_state, RoundState* round_state, int active)
{
    int legal_action_mask = round_state->legal_actions();  // mask representing the actions you are allowed to take
    int street = round_state->street;  // 0, 3, 4, or 5 representing pre-flop, flop, river, or turn respectively
    std::array<std::string, 2> my_cards = round_state->hands[active];  // your cards
    std::array<std::string, 5> board_cards = round_state->deck;  // the board cards
    array<int, 2> my_hand = {convert_card(my_cards[0]), convert_card(my_cards[1])};
    array<int, 5> board = {0, 0, 0, 0, 0};
    for (int i = 0; i < street; i++) {
        board[i] = convert_card(board_cards[i]);
    }
    int my_pip = round_state->pips[active];  // the number of chips you have contributed to the pot this round of betting
    int opp_pip = round_state->pips[1-active];  // the number of chips your opponent has contributed to the pot this round of betting
    //int my_stack = round_state->stacks[active];  // the number of chips you have remaining
    int opp_stack = round_state->stacks[1-active];  // the number of chips your opponent has remaining
    int continue_cost = opp_pip - my_pip;  // the number of chips needed to stay in the pot
    //int my_contribution = STARTING_STACK - my_stack;  // the number of chips you have contributed to the pot
    int opp_contribution = STARTING_STACK - opp_stack;  // the number of chips your opponent has contributed to the pot
    int pot = 2 * opp_contribution;
    if (street == 3 && pot == 400) {
        all_in_pre = true;
    }
    if (((NUM_ROUNDS - game_state->round_num + 1)*3 + 1) / 2 < game_state->bankroll) {
        on_check_fold = true;
    }
    if (on_check_fold) {
        return CheckAction();
    }
    if (pot == 400 && my_pip == opp_pip) {
        return CheckAction();
    }
    // (B-x)(1+A)/(B-A)/(1+x)
    double opp_sizing = 1.0*(opp_pip-my_pip)/(pot - 2*(opp_pip-my_pip));
    double interpreted_sizing = 0;
    int lb = -1, ub = -1;
    if (my_pip == 0) {
        for (int i = 0; i < NUM_BET_SIZES; i++) {
            if (BET_SIZES[i] <= opp_sizing)
                lb = i;
            if (BET_SIZES[i] >= opp_sizing) {
                ub = i;
                break;
            }
        }
    } else {
        for (int i = 0; i < NUM_RAISE_SIZES; i++) {
            if (RAISE_SIZES[i] <= opp_sizing)
                lb = i;
            if (RAISE_SIZES[i] >= opp_sizing) {
                ub = i;
                break;
            }
        }
    }
    double A = 0;
    if (lb != -1) {
        if (my_pip == 0) A = BET_SIZES[lb];
        else A = RAISE_SIZES[lb];
    }
    // all-in interpret fix
    if (A > 990) {
        //int oldpot = pot - opp_pip*2;
        //int opp_new_pip = 200 - oldpot/2;
        /* TO CONSIDER: Should we make this use imagined pot or actual pot? */
        //A = 1.0* (opp_new_pip-my_pip)/(400 - 2*(opp_new_pip-my_pip)); 
        A = 1.0 * (200 - imagined_pot/2) / imagined_pot;
    }
    // #actions leaves our tree
    if ((street == 0 && round_state->button > 7) || (street > 0 && round_state->button > 8)) {
        A = 0;
    }
    double B = (my_pip == 0) ? BET_SIZES[ub] : RAISE_SIZES[ub];
    // all-in interpret fix
    if (B > 990 || (street == 0 && round_state->button > 7) || (street > 0 && round_state->button > 8)) {
        //int oldpot = pot - opp_pip*2;
        //int opp_new_pip = 200 - oldpot/2;
        //B = 1.0* (opp_new_pip-my_pip)/(400 - 2*(opp_new_pip-my_pip));
        B = 1.0 * (200 - imagined_pot/2) / imagined_pot;
    }
    if (B-A<0.001) interpreted_sizing = A;
    else {
        double PA = (B-opp_sizing)*(1+A)/(B-A)/(1+opp_sizing);
        uniform_real_distribution<double> rdist(0,1);
        if (rdist(rng) < PA) interpreted_sizing = A;
        else interpreted_sizing = B;
    }
    //interpreted_sizing = 200/imagined_pot - 0.5 when all in
    imagined_pot += round(2 * imagined_pot * interpreted_sizing);
    if (imagined_pot > 400 || pot == 400) imagined_pot = 400;
    int imagined_pot_odds = pot_odds_bucket(interpreted_sizing / (1 + 2 * interpreted_sizing));
    int card_abs = bucketer->GetBucket(street, my_hand, board);
    // something our CFR strategy doesn't consider
    if ((street == 0 && round_state->button > 7) || (street > 0 && round_state->button > 8)) {
        cout << "roundstatebutton exceeded limit" << endl;
        cout << imagined_pot << ' ' << interpreted_sizing << endl;
        assert(imagined_pot == 400 || interpreted_sizing < 0.001);
    }
    // in the case of opening the pot
    if (street == 0 && round_state->button == 0) {
        imagined_pot = 4;
        imagined_pot_odds = 2;
    }
    if (imagined_pot == 400 && imagined_pot_odds == 0) {
        //cout << "got here, interesting" << endl;
        if (legal_action_mask & RAISE_ACTION_TYPE) {
            return RaiseAction(round_state->raise_bounds()[1]);
        }
        return CallAction();
    }
    // if they raise/bet that we interpret as a check, check whether we interpret as a check-back/call vs an opening check
    if (imagined_pot_odds == 0 && ((street == 0 && round_state->button > 0) || (street > 0 && round_state->button > 1))) {
        bool open_check = ((street == 0 && round_state->button == 1) || (street > 0 && round_state->button == 2));
        
        if (!open_check) {
            return CallAction();
        }
    }
    //if (street > 0) cout << street << ' ' << card_abs << endl;
    vector<pair<pair<char,int>,double> > strat = strategy_map[street][card_abs][round_state->button][make_pair(imagined_pot, imagined_pot_odds)];
    // something our CFR strategy never considered
    if (strat.size() == 0) {
        cout << "this wasn't supposed to happen" << endl;
        cout << "street: " << street << endl;
        cout << "card abs: " << card_abs << endl;
        cout << "button: "<< round_state->button << endl;
        cout << "imagined pot: "<< imagined_pot << endl;
        cout << "imagined pot odds index: " << imagined_pot_odds << endl;
        cout << "opp sizing: " << opp_sizing << endl;
        cout << "interpreted sizing: " << interpreted_sizing << endl;
        cout << "pot, pot odds: " << pot << ' ' << 1.0*continue_cost/pot << endl;

        if (imagined_pot_odds < 5) imagined_pot_odds++;
        strat = strategy_map[street][card_abs][round_state->button][make_pair(imagined_pot, imagined_pot_odds)];
        if (strat.size() == 0) {
            cout << "DOUBLY BAD!" << endl;
        }
    }

    auto bet_to_pip = [legal_action_mask, my_pip, opp_pip](int p, int min_raise, int max_raise) {
        if (p == opp_pip) {
            if (opp_pip == my_pip) {
                return CheckAction();
            }
            return CallAction();
        }
        if (min_raise <= p && p <= max_raise) {
            return RaiseAction(p);
        }
        else if (p < min_raise) {
            return RaiseAction(min_raise);
        }
        else if (p > max_raise) {
            return RaiseAction(max_raise);
        }
        assert(0);
    };
    int i = SampleStrategy(strat);
    if (strat[i].first.first == 'X') {
        if (legal_action_mask & CHECK_ACTION_TYPE) return CheckAction();
        return CallAction();
    } else if (strat[i].first.first == 'C') {
        // if we imagine an all-in as a call, we should jam
        if (imagined_pot == 400 && (legal_action_mask & RAISE_ACTION_TYPE)) {
            return RaiseAction(round_state->raise_bounds()[1]);
        }
        return CallAction();
    } else if (strat[i].first.first == 'F') {
        return FoldAction();
    } else if (strat[i].first.first == 'B') {
        imagined_pot += round(2 * imagined_pot * BET_SIZES[strat[i].first.second]);
        if (imagined_pot > 400) imagined_pot = 400; /* TO CONSIDER: Should we do this? */
        if (legal_action_mask & RAISE_ACTION_TYPE) {
            int min_raise = round_state->raise_bounds()[0];
            int max_raise = round_state->raise_bounds()[1];
            int delta = continue_cost + (int)(BET_SIZES[strat[i].first.second] * pot);
            int sizing = my_pip + delta;
            return bet_to_pip(sizing, min_raise, max_raise);
        } else {
            if (legal_action_mask & CHECK_ACTION_TYPE) return CheckAction();
            return CallAction();
        }
    } else {
        assert(strat[i].first.first == 'R');
        imagined_pot += round(2 * imagined_pot * RAISE_SIZES[strat[i].first.second]);
        if (imagined_pot > 400) imagined_pot = 400;
        if (legal_action_mask & RAISE_ACTION_TYPE) {
            int min_raise = round_state->raise_bounds()[0];
            int max_raise = round_state->raise_bounds()[1];
            int delta = continue_cost + (int)(RAISE_SIZES[strat[i].first.second] * pot);
            int sizing = my_pip + delta;
            return bet_to_pip(sizing, min_raise, max_raise);
        } else {
            if (legal_action_mask & CHECK_ACTION_TYPE) return CheckAction();
            return CallAction();
        }
    }
    assert(0);
}
