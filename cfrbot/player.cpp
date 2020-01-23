/**
 * Simple example pokerbot, written in C++.
 */
#include "player.hpp"
#include <bits/stdc++.h>
using namespace std;

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

class Buckets {
	string pf_cluster[8] = { "23s,24s,25s,26s,27s,34s,35s,36s,37s,45s,46s,32o,43o,42o,54o,53o,52o,65o,64o,63o,62o,74o,73o,72o,83o,82o",
"28s,29s,2Ts,38s,39s,47s,48s,49s,75o,85o,84o,95o,94o,93o,92o,T5o,T4o,T3o,T2o,J3o,J2o",
"3Ts,4Ts,56s,57s,58s,59s,5Ts,67s,68s,69s,6Ts,78s,79s,89s,67o,68o,69o,6To,78o,79o,7To,89o,8To",
"22,J2s,J3s,J4s,J5s,J6s,Q2s,Q3s,Q4s,Q5s,K2s,J4o,J5o,J6o,J7o,Q2o,Q3o,Q4o,Q5o,Q6o,Q7o,K2o,K3o,K4o",
"6Qs,7Ts,7Js,7Qs,8Ts,8Js,8Qs,9Ts,9Js,9Qs,TJs,T9o,J8o,J9o,JTo,Q8o,Q9o,QTo,QJo",
"33,44,55,K3s,K4s,K5s,K6s,K7s,K8s,A2s,A3s,A4s,A5s,A6s,K5o,K6o,K7o,K8o,K9o,A2o,A3o,A4o,A5o,A6o,A7o,A8o",
"66,77,QTs,QJs,K9s,KTs,KJs,KQs,A7s,A8s,A9s,ATs,AJs,AQs,AKs,KTo,KJo,KQo,A9o,ATo,AJo,AQo,AKo",
"88,99,TT,JJ,QQ,KK,AA" };
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
	Buckets() {
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
		array<int, 7> cards;
		cards[0] = hand[0];
		cards[1] = hand[1];
		cards[2] = board[0];
		cards[3] = board[1];
		cards[4] = board[2];
        auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
	    mt19937 rng(seed);
        uniform_int_distribution<int> distribution(0, 51);
		vector<double> bucket(51, 0.0);
		if (street == 3) {
            
            for (int iter = 0; iter < 200; iter++) {
                do {
                    cards[5] = distribution(rng);
                } while (used[cards[5]]);
                used[cards[5]] = 1;
                do {
                    cards[6] = distribution(rng);
                } while (used[cards[6]]);
                used[cards[6]] = 1;
                int my_tot = 0, tot = 0;
                int my_strength = GetHandValue(cards);
                for (int i = 0; i < 52; i++) {
                    if (used[i]) continue;
                    for (int j = i+1; j < 52; j++) {
                        if (used[j]) continue;
                        cards[0] = i;
                        cards[1] = j;
                        int opp_strength = GetHandValue(cards);
                        cards[0] = hand[0];
                        cards[1] = hand[1];
                        tot += 2;
                        if (my_strength > opp_strength) my_tot += 2;
                        else if (my_strength == opp_strength) my_tot++;
                    }
                }
                bucket[(int)(50.0 * my_tot / tot)] += 1;
                used[cards[5]] = 0;
                used[cards[6]] = 0;
            }
		}
		else {
			// street == 4
			cards[5] = board[3];
			for (int i = 0; i < 52; i++) {
				if (used[i]) continue;
				cards[6] = i;
                used[i] = 1;
                int my_strength = GetHandValue(cards);
                int my_tot = 0, tot = 0;
				for (int a = 0; a < 52; a++) {
                    if (used[a]) continue;
                    for (int b = a + 1; b < 52; b++) {
                        if (used[b]) continue;
                        cards[0] = a;
                        cards[1] = b;
                        int opp_strength = GetHandValue(cards);
                        cards[0] = hand[0];
                        cards[1] = hand[1];
                        tot += 2;
                        if (my_strength > opp_strength) my_tot += 2;
                        else if (my_strength == opp_strength) my_tot++;
                    }
                }
                bucket[(int)(50.0 * my_tot / tot)] += 1;
                used[i] = 0;
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
			for (int i = 1; i < river_centers.size(); i++) {
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
const int FILTER_END_SIZE = 200;
// permutation generator
map<long long, bool> seen_permutation;
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
vector<int> permute_values() {
    vector<int> orig_perm = {12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    vector<int> prop_perm;
    mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    geometric_distribution<int> distribution(0.25);
    for (int i = 0; i < 13; i++) {
        int pop_i = orig_perm.size() - 1 - (distribution(rng) % orig_perm.size());
        prop_perm.push_back(orig_perm[pop_i]);
        orig_perm.erase(orig_perm.begin() + pop_i);
    }
    //reverse(prop_perm.begin(),prop_perm.end());
    return prop_perm;
}
int preflop_delta;
int filter_done, filter_done_sd;
map<char, int> rank_map, suit_map;
map<int,char> rankrmap;
vector<vector<int> > proposal_perms;
vector<int> final_perm;
bool perm_finalized;
vector<pair<vector<string>,vector<string> > > showdown_rules;
bool quit_chasing;
int all_in_pre_cnt;

Buckets* bucketer;
map<pair<int, array<int, 2> >, vector<double> > strategy_map[2][6][200];
mt19937 rng;
/**
 * Called when a new game starts. Called exactly once.
 */
Player::Player()
{
    auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
	rng.seed(seed);
    memset(HR, 0, sizeof(HR));
    FILE* hrfin = fopen("handranks.dat", "rb");
    size_t bytesread = fread(HR, sizeof(HR), 1, hrfin);
    fclose(hrfin);
    ifstream fin("324k.txt");
    string line;
    getline(fin, line);
    for (int i = 1; i < 554276; i++) {
        getline(fin, line);
        istringstream ss(line);
        int street, infoset, player, pot;
        array<int, 2> pips;
        ss >> street >> infoset >> player >> pot >> pips[0] >> pips[1];
        char colon_dummy;
        ss >> colon_dummy;
        vector<double> strategy;
        double p;
        while (ss >> p) {
            strategy.push_back(p);
        }
        strategy.pop_back();
        strategy_map[player][street][infoset][make_pair(pot, pips)] = strategy;
    }
    fin.close();
    bucketer = new Buckets();
    quit_chasing = false;
	all_in_pre_cnt = 0;
    //string ranks="23456789TJQKA";
    //string suits="shdc";
    vector<int> asdfasdf = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    seen_permutation[perm_to_mask(asdfasdf)] = true;
    proposal_perms.push_back(asdfasdf);

    for (int i = 0; i < 50000; i++) {
        vector<int> proposal_perm = permute_values();
        proposal_perms.push_back(proposal_perm);
        seen_permutation[perm_to_mask(proposal_perm)] = true;
    }
    rank_map['2']=2;
    rank_map['3']=3;
    rank_map['4']=4;
    rank_map['5']=5;
    rank_map['6']=6;
    rank_map['7']=7;
    rank_map['8']=8;
    rank_map['9']=9;
    rank_map['T']=10;
    rank_map['J']=11;
    rank_map['Q']=12;
    rank_map['K']=13;
    rank_map['A']=14;
    for(auto it : rank_map){
        rankrmap[it.second]=it.first;
    }
    suit_map['s']=0;
    suit_map['h']=1;
    suit_map['c']=2;
    suit_map['d']=3;
}
bool all_in_pre;
bool on_check_fold;
bool big_blind;
int opp_3b_size;
int cnt_4b_value;
int opp_4b_size;
int small_pots_won, small_pots_lost, big_pots_won, big_pots_lost, chopped_pots; // small is <= 50 chips
int spw,spl,bpw,bpl,cp; // same as variables above but only for first 500 rounds
vector<int> pf_deltas;
int all_in_pre_delta;

int flop_delta, turn_delta, river_delta, showdown_delta;
int flop_tot, turn_tot, river_tot, showdown_tot;

int my_btn_vpip, opp_btn_vpip;
int my_bb_vpip, opp_bb_vpip;

pair<int, array<int, 2> > prev;

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
    big_blind = (bool) active;  // true if you are the big blind
    all_in_pre = false;
    opp_3b_size = 2;
    opp_4b_size = 1;
    on_check_fold = false;
    int street = round_state->street;
    assert(street == 0);
	if (showdown_tot+all_in_pre_cnt >= 5){
    string ranks = "23456789TJQKA";
    mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	uniform_int_distribution<int> R(0,proposal_perms.size()-1);
    vector<int> guess = proposal_perms[R(rng)];
    if (perm_finalized) {
        guess=final_perm;
    }
    for (int i = 0; i < 13; i++) {
        rank_map[ranks[guess[i]]]=i+2;
    }
	}
}

void handle_showdowns(GameState* game_state, TerminalState* terminal_state, int active){
    if (perm_finalized || quit_chasing || game_state->game_clock < 10.0) return;
    RoundState* previous_state = (RoundState*) terminal_state->previous_state;  // RoundState before payoffs
    int street = previous_state->street;  // 0, 3, 4, or 5 representing when this round ended
    std::array<std::string, 2> my_cards = previous_state->hands[active];  // your cards
    std::array<std::string, 2> opp_cards = previous_state->hands[1-active];  // opponent's cards or "" if not revealed
    int my_delta = terminal_state->deltas[active];  // your bankroll change from this round
    vector<string> my_hand = {my_cards[0], my_cards[1]}, opp_hand = {opp_cards[0], opp_cards[1]};
    std::array<std::string, 5> board_cards = previous_state->deck;
    for (int i = 0; i < street; i++) {
        my_hand.push_back(board_cards[i]);
        opp_hand.push_back(board_cards[i]);
    }

    if (my_delta > 0) {
        showdown_rules.push_back(make_pair(my_hand, opp_hand));
    } else if (my_delta < 0) {
        showdown_rules.push_back(make_pair(opp_hand, my_hand));
    }
    vector<vector<int> > new_perms;
    
    for (const vector<int> &proposal_perm : proposal_perms) {
        vector<int> my_sd_cards;
        vector<int> opp_sd_cards;
        assert(my_hand.size() == 7);
        string ranks="23456789TJQKA";
        map<char,char>mpp;
        // guess =  0  1  2 3 4 5 6 7 8 9 11 10 12
        // ranks =  2  3  4 5 6 7 8 9 T J Q   K  A
        // rg    =  2  3  4 5 6 7 8 9 T J K   Q  A
        for (int i = 0; i < 13; i++) {
            mpp[ranks[i]]=ranks[proposal_perm[i]];
            //cout << ranks[i] << " maps to " << ranks[proposal_perm[i]] << endl;
        }
        for (string &c : my_hand) {
            my_sd_cards.push_back(hand_to_int(""+mpp[c[0]]+c[1]));
        }
        for (string &c : opp_hand) {
            opp_sd_cards.push_back(hand_to_int(""+mpp[c[0]]+c[1]));
        }

        array<int, 7> h;
        for (int i = 0; i < 7; i++) {
            h[i] = my_sd_cards[i];
        }
        int my_strength = GetHandValue(h);
        
        array<int, 7> h2;
        for (int i = 0; i < 7; i++) {
            h2[i] = opp_sd_cards[i];
        }
        int opp_strength = GetHandValue(h2);
        if (my_strength > opp_strength && my_delta > 0)
            new_perms.push_back(proposal_perm);
        else if (my_strength < opp_strength && my_delta < 0)
            new_perms.push_back(proposal_perm);
        else if (my_strength == opp_strength && my_delta == 0)
            new_perms.push_back(proposal_perm);
        
        //else
        //    cout << "contradiction\n";
    }
	if (new_perms.size() == 0) {
		quit_chasing = true;
		return;
	}
	auto TIME= clock();
    while (new_perms.size() >= 2 && new_perms.size() < FILTER_END_SIZE && 1.0*(clock()-TIME)/CLOCKS_PER_SEC<5.0) {
        
        //if (new_perms.size() == FILTER_END_SIZE) {
            //cout << "here\n";
            int s = new_perms.size();
            for (int i = 0; i < s; i++) {
                vector<int> prop = new_perms[i];
                //int cnt = 0;
                for (int j = 0; j < 13; j++){
                    for (int k=j;k<13;k++){
                        if (j==k&&j!=0) continue;
                        //cnt++;
                        //if (cnt==10)break;
                        swap(prop[j],prop[k]);
                        bool works = true;
                        string ranks="23456789TJQKA";
                        map<char,char>mpp;
                        if (seen_permutation[perm_to_mask(prop)]) {
                        //    works = false;
                        }
                        seen_permutation[perm_to_mask(prop)] = true;
                        if (works){
                        for (int ii = 0; ii < 13; ii++) {
                            mpp[ranks[ii]]=ranks[prop[ii]];
                            //cout << ranks[i] << " maps to " << ranks[proposal_perm[i]] << endl;
                        }
                        for (auto it : showdown_rules) {
                            
                            vector<int> my_sd_cards;
                            vector<int> opp_sd_cards;
                            assert(my_hand.size() == 7);
                            for (string &c : it.first) {
                                my_sd_cards.push_back(hand_to_int(""+mpp[c[0]]+c[1]));
                            }
                            for (string &c : it.second) {
                                opp_sd_cards.push_back(hand_to_int(""+mpp[c[0]]+c[1]));
                            }

                            array<int, 7> h;
                            for (int i = 0; i < 7; i++) {
                                h[i] = my_sd_cards[i];
                            }
                            int my_strength = GetHandValue(h);
                            
                            array<int, 7> h2;
                            for (int i = 0; i < 7; i++) {
                                h2[i] = opp_sd_cards[i];
                            }
                            int opp_strength = GetHandValue(h2);
                            if (my_strength <= opp_strength) {
                                works = false;
                                break;
                            }
                        }                
                     }
                        if (works) {
                            new_perms.push_back(prop);
                        }
                        swap(prop[j],prop[k]);
                    }
                }
            }
            bool all_equal = true;
            for (int i = 1; i < (int)new_perms.size(); i++) {
                if (new_perms[i] != new_perms[i-1]){
                    all_equal=false;
                    break;
                }
            }
            if (all_equal) {
                filter_done = game_state->round_num;
                filter_done_sd = showdown_tot;
                perm_finalized=true;
                final_perm=new_perms[0];
                return;
            }
        //}
       
    }
    //cout << new_perms.size() << endl;
    proposal_perms.clear();
    for (int i = 0; i < (int)new_perms.size(); i++)
        proposal_perms.push_back(new_perms[i]);
    
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
    //int my_delta = terminal_state->deltas[active];  // your bankroll change from this round
    //RoundState* previous_state = (RoundState*) terminal_state->previous_state;  // RoundState before payoffs
    //int street = previous_state->street;  // 0, 3, 4, or 5 representing when this round ended
    //std::array<std::string, 2> my_cards = previous_state->hands[active];  // your cards
    //std::array<std::string, 2> opp_cards = previous_state->hands[1-active];  // opponent's cards or "" if not revealed
    RoundState* previous_state = (RoundState*) terminal_state->previous_state;  // RoundState before payoffs
    int street = previous_state->street;  // 0, 3, 4, or 5 representing when this round ended
    std::array<std::string, 2> my_cards = previous_state->hands[active];  // your cards
    std::array<std::string, 2> opp_cards = previous_state->hands[1-active];  // opponent's cards or "" if not revealed
    int my_delta = terminal_state->deltas[active];  // your bankroll change from this round
    vector<string> my_hand = {my_cards[0], my_cards[1]}, opp_hand = {opp_cards[0], opp_cards[1]};
    std::array<std::string, 5> board_cards = previous_state->deck;
    for (int i = 0; i < street; i++) {
        my_hand.push_back(board_cards[i]);
        opp_hand.push_back(board_cards[i]);
    }
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
        if (!perm_finalized){
            cout << "possible permutations (by particle filter):" << endl;
            set<long long> ssss;
            for (const vector<int> &v : proposal_perms) {
                /*for (int i=0;i<13;i++){
                    cout<<ranks[v[i]] << ' ';
                }
                cout<<'\n';*/
                ssss.insert(perm_to_mask(v));
            }
            for (long long fdsa : ssss) {
                for (int x : mask_to_perm(fdsa)) {
                    cout << ranks[x] << ' ';
                }
                cout << '\n';
            }
        }
        cout << "filter done on round: " << filter_done << endl;
        cout << "filter done on SD#:   " << filter_done_sd << endl;
        cout << "final perm:\n";
        for (int x:final_perm)cout<<ranks[x]<<' ';
        cout<<'\n';
        cout << "time left: " << (game_state->game_clock) << endl;
    }
    
    if (street == 5 && my_contribution == opp_contribution) {
        handle_showdowns(game_state,terminal_state,active);
    }
}

int SampleStrategy(const vector<double>& s) {
    std::uniform_real_distribution<double> distribution(0, 1);
    double r = distribution(rng);
    double acc = 0;
    for (int i = 0; i < (int)s.size(); i++) {
        acc += s[i];
        if (r < acc) return i;
    }
    assert(0);
}


const int DECK_SIZE = 52;
const int NUM_BET_SIZES = 4;
const int NUM_RAISE_SIZES = 2;
const array<double, NUM_BET_SIZES> BET_SIZES = { 0.5, 1.0, 2.0, 999 };
const array<double, NUM_RAISE_SIZES> RAISE_SIZES = { 1.0, 999 };
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

    my_cards[0][0]=rankrmap[rank_map[my_cards[0][0]]];
    my_cards[1][0]=rankrmap[rank_map[my_cards[1][0]]];

    std::array<std::string, 5> board_cards = round_state->deck;  // the board cards

    for (int i = 0; i < street; i++) {
        board_cards[i][0] = rankrmap[rank_map[board_cards[i][0]]];
    }

    array<int, 2> my_hand = {hand_to_int(my_cards[0]), hand_to_int(my_cards[1])};
    array<int, 5> board = {0, 0, 0, 0, 0};
    for (int i = 0; i < street; i++) {
        board[i] = hand_to_int(board_cards[i]);
    }
    int my_pip = round_state->pips[active];  // the number of chips you have contributed to the pot this round of betting
    int opp_pip = round_state->pips[1-active];  // the number of chips your opponent has contributed to the pot this round of betting
    int my_stack = round_state->stacks[active];  // the number of chips you have remaining
    int opp_stack = round_state->stacks[1-active];  // the number of chips your opponent has remaining
    int continue_cost = opp_pip - my_pip;  // the number of chips needed to stay in the pot
    int my_contribution = STARTING_STACK - my_stack;  // the number of chips you have contributed to the pot
    int opp_contribution = STARTING_STACK - opp_stack;  // the number of chips your opponent has contributed to the pot
    int pot = 2 * opp_contribution;
    //if (RAISE_ACTION_TYPE & legal_actions)
    //{
    //    std::array<int, 2> raise_bounds = round_state->raise_bounds();  // the smallest and largest numbers of chips for a legal bet/raise
    //    int min_cost = raise_bounds[0] - my_pip;  // the cost of a minimum bet/raise
    //    int max_cost = raise_bounds[1] - my_pip;  // the cost of a maximum bet/raise
    //}
    if (street == 3 && pot == 400) {
        all_in_pre = true;
    }
    if (((NUM_ROUNDS - game_state->round_num + 1)*3 + 1) / 2 < game_state->bankroll) {
        on_check_fold = true;
    }
    if (on_check_fold) {
        //return CheckAction();
    }
    if (pot == 400) {
        return CheckAction();
    }
    //auto TIME = clock();
    // (B-x)(1+A)/(B-A)/(1+x)
    // keep track of imaginary pips, imaginary round state, whether our new actions are legal
    //double opp_sizing = 1.0*(opp_pip-my_pip)/(pot - (opp_pip-my_pip));
    vector<double> s = strategy_map[active][street][bucketer->GetBucket(street, my_hand, board)][make_pair(pot, round_state->pips)];
    //cout << "Fetch time: " << 1000.0*(clock()-TIME) / CLOCKS_PER_SEC << "ms" << endl;
    if (s.size() == 0) {
        cout << active << ' ' << street << ' ' << bucketer->GetBucket(street, my_hand, board) << ' ' << pot << ' ' << round_state->pips[0] << ' ' << round_state->pips[1] << endl;
    }
    vector<Action> a;
    if ((legal_action_mask & RAISE_ACTION_TYPE) && round_state->button < 4) {
        int min_raise = round_state->raise_bounds()[0];
        int max_raise = round_state->raise_bounds()[1];
        bool is_bet = (continue_cost == 0);
        if (is_bet) {
            for (int i = 0; i < NUM_BET_SIZES; i++) {
                int delta = continue_cost + (int)(BET_SIZES[i] * pot);
                int sizing = my_pip + delta;
                if (min_raise <= sizing && sizing < max_raise) {
                    //history.push_back({ 'R', sizing });
                    a.push_back(RaiseAction(sizing));
                    //dump_strategy(state->children[child_id++], history);
                    //history.pop_back();
                }
                if (BET_SIZES[i] > 990) {
                    //history.push_back({ 'R', max_raise });
                    a.push_back(RaiseAction(max_raise));
                    //dump_strategy(state->children[child_id++], history);
                    //history.pop_back();
                }
            }
        }
        else {
            for (int i = 0; i < NUM_RAISE_SIZES; i++) {
                int delta = continue_cost + (int)(RAISE_SIZES[i] * pot);
                int sizing = my_pip + delta;
                if (min_raise <= sizing && sizing < max_raise) {
                    //history.push_back({ 'R', sizing });
                    a.push_back(RaiseAction(sizing));
                    //dump_strategy(state->children[child_id++], history);
                    //history.pop_back();
                }
                if (RAISE_SIZES[i] > 990) {
                    a.push_back(RaiseAction(max_raise));
                    //history.push_back({ 'R', max_raise });
                    //dump_strategy(state->children[child_id++], history);
                    //history.pop_back();
                }
            }
        }
    }
    if (legal_action_mask & CHECK_ACTION_TYPE) {
        //history.push_back({ 'X', 0 });
        a.push_back(CheckAction());
        //dump_strategy(state->children[child_id++], history);
        //history.pop_back();
    }
    else {
        assert(legal_action_mask & CALL_ACTION_TYPE);
        assert(legal_action_mask & FOLD_ACTION_TYPE);
        a.push_back(CallAction());
        a.push_back(FoldAction());
        //history.push_back({ 'C', 0 });
        //dump_strategy(state->children[child_id++], history);
        //history.pop_back();
        //history.push_back({ 'F', 0 });
        //dump_strategy(state->children[child_id++], history);
        //history.pop_back();
    }
    int i = SampleStrategy(s);
    return a[i];
    //assert(0);
}
