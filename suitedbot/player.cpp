#include "player.hpp"
#include <iostream>
#include <fstream>
#include <random>
#include <cassert>
#include <map>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <set>
#include "OMPEval/omp/EquityCalculator.h"
using namespace std;
using namespace omp;
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

// for OMPEval
int chars_to_cards(char rank, char suit) {
    map<char, int> suit_map = {{'s',0},{'h',1},{'c',2},{'d',3}};
    map<char, int> rank_map;
    string ranks = "23456789TJQKA";
    for (int i = 0; i < 13; i++) {
        rank_map[ranks[i]] = i;
    }
    return rank_map[rank]*4 + suit_map[suit];
}

/**
 * Called when a new game starts. Called exactly once.
 */
 struct Card {
	int rank, suit;
};
int preflop_delta;
int filter_done, filter_done_sd;
vector<pair<string,string> > hand_ranking;
map<char, int> rank_map, suit_map;
map<int,char> rankrmap;
vector<Card>deck;
vector<vector<int> > proposal_perms;
vector<int> final_perm;
bool perm_finalized;
vector<pair<vector<string>,vector<string> > > showdown_rules;
HandEvaluator eval;
bool quit_chasing;
int all_in_pre_cnt;

Player::Player() {
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
    for (int i = 2; i <= 14; i++) {
		for (int j = 0; j < 4; j++) {
			deck.push_back({i, j});
		}
	}
    ifstream fin("ranking.txt");
    hand_ranking.resize(1326);
    for (int i = 0; i < 1326; i++) {
        fin >> hand_ranking[i].first >> hand_ranking[i].second;
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
bool on_check_fold = false;
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


const string rank_convert[15]={"error0","error1","2","3","4","5","6","7","8","9","T","J","Q","K","A"};
const string hand_type_convert[8]={"high card","pair","two pair","3 of a kind","flush","full house","quads","unknown"};
const double outs_table[23]={0,0.042,0.084,0.124,0.164,0.203,0.241,0.278,0.314,0.349,0.383,0.417,0.449,0.481,0.511,0.541,0.569,0.624,0.650,0.675,0.699,0.72};
// 0: high card, 1:pair, 2:2 pair, 3:3 of a kind, 4:flush, 5:full house, 6:quads, 7:not revealed
int hand_type(const vector<string> &cards) {
    int rank_cnt[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int suit_cnt[4] = {0, 0, 0, 0};
	for (const string &c : cards) {
        if (c.size() < 2) return 7;
		rank_cnt[rank_map[c[0]]]++;
		suit_cnt[suit_map[c[1]]]++;
	}
	int cnt_cnt[5] = {0, 0, 0, 0, 0};
	for (int i = 2; i <= 14; i++) {
		cnt_cnt[rank_cnt[i]]++;
	}
	int oa;
	if (cnt_cnt[4]) oa = 6;
	else if (cnt_cnt[3] && cnt_cnt[3]+cnt_cnt[2]>=2) oa = 5;
	else if (*max_element(suit_cnt,suit_cnt+4) >= 5) oa = 4;
	else if (cnt_cnt[3]) oa = 3;
	else if (cnt_cnt[2] >= 2) oa = 2;
	else if (cnt_cnt[2]) oa = 1;
	else oa = 0;
    return oa;
}

/**
 * Called when a new round starts. Called NUM_ROUNDS times.
 *
 * @param game_state Pointer to the GameState object.
 * @param round_state Pointer to the RoundState object.
 * @param active Your player's index.
 */
void Player::handle_new_round(GameState* game_state, RoundState* round_state, int active) {
    //int my_bankroll = game_state->bankroll;  // the total number of chips you've gained or lost from the beginning of the game to the start of this round
    //float game_clock = game_state->game_clock;  // the total number of seconds your bot has left to play this game
    //int round_num = game_state->round_num;  // the round number from 1 to NUM_ROUNDS
    big_blind = (bool) active;  // true if you are the big blind
    all_in_pre = false;
    opp_3b_size = 2;
    opp_4b_size = 1;
    // on_check_fold = false;
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

int extras;

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
            my_sd_cards.push_back(chars_to_cards(mpp[c[0]], c[1]));
        }
        for (string &c : opp_hand) {
            opp_sd_cards.push_back(chars_to_cards(mpp[c[0]], c[1]));
        }

        Hand h = Hand::empty();
        for (int c : my_sd_cards) {
            h += Hand(c);
        }
        int my_strength = eval.evaluate(h);
        Hand h2 = Hand::empty();
        for (int c : opp_sd_cards) {
            h2 += Hand(c);
        }
        int opp_strength = eval.evaluate(h2);
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
                            assert(it.first.size() == 7);
                            
                            for (string &c : it.first) {
                                my_sd_cards.push_back(chars_to_cards(mpp[c[0]], c[1]));
                            }
                            for (string &c : it.second) {
                                opp_sd_cards.push_back(chars_to_cards(mpp[c[0]], c[1]));
                            }
                            Hand h = Hand::empty();
                            for (int c : my_sd_cards) {
                                h += Hand(c);
                            }
                            int my_strength = eval.evaluate(h);
                            Hand h2 = Hand::empty();
                            for (int c : opp_sd_cards) {
                                h2 += Hand(c);
                            }
                            int opp_strength = eval.evaluate(h2);
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
void Player::handle_round_over(GameState* game_state, TerminalState* terminal_state, int active) {

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
    
    if(big_blind && street ==0){
        if(my_delta == 1) //if someone folds as SMB
        {
            on_check_fold = true; 
        }
    }
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
        cout << my_delta << ", round #" << (game_state->round_num) << 
        ", street: " << street << ", " << hand_type_convert[hand_type(my_hand)] << " vs " << hand_type_convert[hand_type(opp_hand)];
        if (all_in_pre) cout << " (all-in pre)";
        if (hand_type(my_hand) > hand_type(opp_hand)) cout << " (STRAIGHT)";
        cout << "\n";
        big_pots_lost++;
        if (game_state->round_num <= 500) bpl++;
    } else if (my_delta > 50) {
        cout << my_delta << ", hand #" << (game_state->round_num) << 
        ", street: " << street << ", " << hand_type_convert[hand_type(my_hand)] << " vs " << hand_type_convert[hand_type(opp_hand)];
        if (all_in_pre) cout << " (all-in pre)";
        if (hand_type(my_hand) < hand_type(opp_hand) && hand_type(opp_hand) != 7) cout << " (we somehow had a STRAIGHT)";
        cout << "\n";
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
        cout << "extras: " << extras << endl;
        cout << "final perm:\n";
        for (int x:final_perm)cout<<ranks[x]<<' ';
        cout<<'\n';
        cout << "time left: " << (game_state->game_clock) << endl;
    }
    
    if (street == 5 && my_contribution == opp_contribution) {
        handle_showdowns(game_state,terminal_state,active);
    }
        
}



// check-fold, but calls a bet if size <= 1/4 pot
Action check_fold(GameState* game_state, RoundState* round_state, int active) {
    int legal_actions = round_state->legal_actions();
    if (CHECK_ACTION_TYPE & legal_actions)  // check-call
    {
        return CheckAction();
    }
    int my_pip = round_state->pips[active];  // the number of chips you have contributed to the pot this round of betting
    int opp_pip = round_state->pips[1-active];  // the number of chips your opponent has contributed to the pot this round of betting
    int my_stack = round_state->stacks[active];  // the number of chips you have remaining
    int opp_stack = round_state->stacks[1-active];  // the number of chips your opponent has remaining
    int continue_cost = opp_pip - my_pip;  // the number of chips needed to stay in the pot
    int my_contribution = STARTING_STACK - my_stack;  // the number of chips you have contributed to the pot
    int opp_contribution = STARTING_STACK - opp_stack;  // the number of chips your opponent has contributed to the pot
    int pot = my_contribution + opp_contribution;
    if (4*continue_cost <= pot-continue_cost) {
        return CallAction();
    }
    return FoldAction();
}

// check-call any bet
Action check_call(GameState* game_state, RoundState* round_state, int active) {
    int legal_actions = round_state->legal_actions();
    if (CHECK_ACTION_TYPE & legal_actions)  // check-call
    {
        return CheckAction();
    }
    return CallAction();
}

// check-call a bet <= pot
Action check_call_under_pot(GameState* game_state, RoundState* round_state, int active) {
    int legal_actions = round_state->legal_actions();
    if (CHECK_ACTION_TYPE & legal_actions)  // check-call
    {
        return CheckAction();
    }
    int my_pip = round_state->pips[active];  // the number of chips you have contributed to the pot this round of betting
    int opp_pip = round_state->pips[1-active];  // the number of chips your opponent has contributed to the pot this round of betting
    int my_stack = round_state->stacks[active];  // the number of chips you have remaining
    int opp_stack = round_state->stacks[1-active];  // the number of chips your opponent has remaining
    int continue_cost = opp_pip - my_pip;  // the number of chips needed to stay in the pot
    int my_contribution = STARTING_STACK - my_stack;  // the number of chips you have contributed to the pot
    int opp_contribution = STARTING_STACK - opp_stack;  // the number of chips your opponent has contributed to the pot
    int pot = my_contribution + opp_contribution;
    if (2*continue_cost <= pot) {
        return CallAction();
    }
    return FoldAction();
}


// uses a geometric sizing, jams if necessary
Action geometric(GameState* game_state, RoundState* round_state, int active) {
    int legal_actions = round_state->legal_actions();  // mask representing the actions you are allowed to take
    int street = round_state->street;  // 0, 3, 4, or 5 representing pre-flop, flop, turn, or river respectively
    int my_pip = round_state->pips[active];  // the number of chips you have contributed to the pot this round of betting
    int opp_pip = round_state->pips[1-active];  // the number of chips your opponent has contributed to the pot this round of betting
    int my_stack = round_state->stacks[active];  // the number of chips you have remaining
    int opp_stack = round_state->stacks[1-active];  // the number of chips your opponent has remaining
    int continue_cost = opp_pip - my_pip;  // the number of chips needed to stay in the pot
    int my_contribution = STARTING_STACK - my_stack;  // the number of chips you have contributed to the pot
    int opp_contribution = STARTING_STACK - opp_stack;  // the number of chips your opponent has contributed to the pot
    int initial_pot = 2 * max(my_contribution, opp_contribution);
    int all_in_pot_size = 400;
    int bet_size = 0;
    if (street == 0) {
        bet_size = continue_cost + (int)pow((1.0*all_in_pot_size/initial_pot-1)/2*initial_pot, 0.25);
    } else if (street == 3) {
        bet_size = continue_cost + (int)pow((1.0*all_in_pot_size/initial_pot-1)/2*initial_pot, 0.33);
    } else if (street == 4) {
        bet_size = continue_cost + (int)pow((1.0*all_in_pot_size/initial_pot-1)/2*initial_pot, 0.5);
    } else if (street == 5) {
        bet_size = continue_cost + (int)(1.0*all_in_pot_size/initial_pot-1)/2*initial_pot;
    }
    // prevent massive overbets
    bet_size = min(2*initial_pot, bet_size);
    if (RAISE_ACTION_TYPE & legal_actions)  // check-call
    {
        std::array<int, 2> raise_bounds = round_state->raise_bounds();  // the smallest and largest numbers of chips for a legal bet/raise
        //int min_cost = raise_bounds[0] - my_pip;  // the cost of a minimum bet/raise
        //int max_cost = raise_bounds[1] - my_pip;  // the cost of a maximum bet/raise
        //cout << street << ' ' << initial_pot << ' ' << bet_size << endl;
        // 9, 14, 5, 15, 196, pot before was thus 8 (4 each) and we bet 5 and opp bet 10 and our min rai
        //cout << (game_state->round_num) << my_contribution << ' ' << opp_contribution << ' ' << my_pip << ' ' << raise_bounds[0] << ' ' << raise_bounds[1] << endl;
        if (bet_size < raise_bounds[0] || bet_size > raise_bounds[1]) {
            return RaiseAction(raise_bounds[1]);
        }
        return RaiseAction(bet_size);
    }
    if (CALL_ACTION_TYPE & legal_actions) {
        return CallAction();
    }
    return CheckAction();
}

// for now redirects to geometric, but should just jam
Action jam(GameState* game_state, RoundState* round_state, int active) {
    return geometric(game_state, round_state, active);
    int legal_actions = round_state->legal_actions();
    if (RAISE_ACTION_TYPE & legal_actions)  // check-call
    {
        std::array<int, 2> raise_bounds = round_state->raise_bounds();  // the smallest and largest numbers of chips for a legal bet/raise
        //int min_cost = raise_bounds[0] - my_pip;  // the cost of a minimum bet/raise
        //int max_cost = raise_bounds[1] - my_pip;  // the cost of a maximum bet/raise
        return RaiseAction(raise_bounds[1]);
    }
    if (CALL_ACTION_TYPE & legal_actions) {
        return CallAction();
    }
    return CheckAction();
}

Action quarter_pot_raise(GameState* game_state, RoundState* round_state, int active) {
    int legal_actions = round_state->legal_actions();  // mask representing the actions you are allowed to take
    //int street = round_state->street;  // 0, 3, 4, or 5 representing pre-flop, flop, turn, or river respectively
    //int my_pip = round_state->pips[active];  // the number of chips you have contributed to the pot this round of betting
    //int opp_pip = round_state->pips[1-active];  // the number of chips your opponent has contributed to the pot this round of betting
    //int my_stack = round_state->stacks[active];  // the number of chips you have remaining
    int opp_stack = round_state->stacks[1-active];  // the number of chips your opponent has remaining
    //int continue_cost = opp_pip - my_pip;  // the number of chips needed to stay in the pot
    //int my_contribution = STARTING_STACK - my_stack;  // the number of chips you have contributed to the pot
    int opp_contribution = STARTING_STACK - opp_stack;  // the number of chips your opponent has contributed to the pot
    int pot = 2 * opp_contribution;
    int quarter_pot = pot / 4;
    if (RAISE_ACTION_TYPE & legal_actions)  {
        std::array<int, 2> raise_bounds = round_state->raise_bounds();  // the smallest and largest numbers of chips for a legal bet/raise
        //int min_cost = raise_bounds[0] - my_pip;  // the cost of a minimum bet/raise
        //int max_cost = raise_bounds[1] - my_pip;  // the cost of a maximum bet/raise
        if (opp_contribution + quarter_pot < raise_bounds[0]) {
            return RaiseAction(raise_bounds[0]);
        }
        if (opp_contribution + quarter_pot > raise_bounds[1]) {
            return RaiseAction(raise_bounds[1]);
        }
        return RaiseAction(opp_contribution + quarter_pot);
    }
    if (CALL_ACTION_TYPE & legal_actions) {
        return CallAction();
    }
    return CheckAction();
}

// min raise / min bet
Action min_raise(GameState* game_state, RoundState* round_state, int active) {
    return quarter_pot_raise(game_state, round_state, active);
    int legal_actions = round_state->legal_actions();
    if (RAISE_ACTION_TYPE & legal_actions)  {
        std::array<int, 2> raise_bounds = round_state->raise_bounds();  // the smallest and largest numbers of chips for a legal bet/raise
        //int min_cost = raise_bounds[0] - my_pip;  // the cost of a minimum bet/raise
        //int max_cost = raise_bounds[1] - my_pip;  // the cost of a maximum bet/raise
        return RaiseAction(raise_bounds[0]);
    }
    if (CALL_ACTION_TYPE & legal_actions) {
        return CallAction();
    }
    return CheckAction();
}

Action preflop_BTN(GameState* game_state, RoundState* round_state, int active) {
    assert(round_state->street == 0);
    assert(!big_blind);
    int legal_actions = round_state->legal_actions();
    std::array<std::string, 2> my_cards = round_state->hands[active];  // your cards
    my_cards[0][0]=rankrmap[rank_map[my_cards[0][0]]];
    my_cards[1][0]=rankrmap[rank_map[my_cards[1][0]]];
    //int my_pip = round_state->pips[active];  // the number of chips you have contributed to the pot this round of betting
    //int opp_pip = round_state->pips[1-active];  // the number of chips your opponent has contributed to the pot this round of betting
    int my_stack = round_state->stacks[active];  // the number of chips you have remaining
    int opp_stack = round_state->stacks[1-active];  // the number of chips your opponent has remaining
    //int continue_cost = opp_pip - my_pip;  // the number of chips needed to stay in the pot
    int my_contribution = STARTING_STACK - my_stack;  // the number of chips you have contributed to the pot
    int opp_contribution = STARTING_STACK - opp_stack;  // the number of chips your opponent has contributed to the pot
    
    if(my_contribution ==4)
    {
        if(opp_contribution != 14 && opp_contribution !=4)
        {
            on_check_fold = true;
            return FoldAction();
        }
    }

    if (game_state->round_num == 1) 
    {
        return FoldAction();
    }
    // check if RFI
    if (my_contribution == 1) {
        // check if in top 1074 hands
        for (int i = 0; i < 252; i++) {
            
            if ((my_cards[0] == hand_ranking[i].first && my_cards[1] == hand_ranking[i].second)
            || (my_cards[1] == hand_ranking[i].first && my_cards[0] == hand_ranking[i].second)) {
                return FoldAction();
            }
        }
        return RaiseAction(4);
    }
    // calling jams with top 69 hands
    if (opp_contribution == 200) {
        for (int i = 1326-69; i < 1326; i++) {
            if ((my_cards[0] == hand_ranking[i].first && my_cards[1] == hand_ranking[i].second) 
            || (my_cards[1] == hand_ranking[i].first && my_cards[0] == hand_ranking[i].second)) {
                return CallAction();
            }
        }
    }
    // check if we are facing a 3-bet+
    //if (my_contribution == 4) {
        double MDF = min(1.0,1*(1 - 1.0 * (opp_contribution - opp_3b_size) / (opp_contribution + my_contribution)));
        opp_3b_size = opp_contribution;
        int defend_cnt = MDF*1074;
        cnt_4b_value = defend_cnt/6;
        int bottom_of_continuing_range = 1326-defend_cnt;
        //cout << bottom_of_continuing_range << endl;
        int sizing_4b = opp_contribution * 5 / 2;
        for (int i = bottom_of_continuing_range; i < 1326; i++) {
            if ((my_cards[0] == hand_ranking[i].first && my_cards[1] == hand_ranking[i].second) || (my_cards[1] == hand_ranking[i].first && my_cards[0] == hand_ranking[i].second)) {
                // 4 bet or 4 bet bluff
                if (i < bottom_of_continuing_range + cnt_4b_value || i >= 1326 - cnt_4b_value) {
                    if (legal_actions & RAISE_ACTION_TYPE) {
                        std::array<int, 2> raise_bounds = round_state->raise_bounds();
                        sizing_4b = max(sizing_4b, raise_bounds[0]);
                        if (sizing_4b < raise_bounds[1]) {
                            return RaiseAction(sizing_4b);
                        }
                        return RaiseAction(raise_bounds[1]);
                    }
                    return CallAction();
                }
                return CallAction();
            }
        }
        return FoldAction();
    //}
    // we are facing a 5-bet
    /*double MDF = 1 - 1.0 * (opp_contribution - opp_3b_size) / (opp_contribution + my_contribution);
    int defend_cnt = MDF*cnt_4b;
    for (int i = 1326 - defend_cnt; i < 1326; i++) {
        if (my_cards[0] == hand_ranking[i][0] && my_cards[1] == hand_ranking[i][1]) {
            if (legal_actions & RAISE_ACTION_TYPE) {
                std::array<int, 2> raise_bounds = round_state->raise_bounds();
                return RaiseAction(raise_bounds[1]);
            }
            return CallAction();
        }
        return FoldAction();
    }*/
}
Action preflop_BB(GameState* game_state, RoundState* round_state, int active) {
    assert(round_state->street == 0);
    assert(big_blind);
    int legal_actions = round_state->legal_actions();
    std::array<std::string, 2> my_cards = round_state->hands[active];  // your cards
    my_cards[0][0]=rankrmap[rank_map[my_cards[0][0]]];
    my_cards[1][0]=rankrmap[rank_map[my_cards[1][0]]];
    //int my_pip = round_state->pips[active];  // the number of chips you have contributed to the pot this round of betting
    //int opp_pip = round_state->pips[1-active];  // the number of chips your opponent has contributed to the pot this round of betting
    int my_stack = round_state->stacks[active];  // the number of chips you have remaining
    int opp_stack = round_state->stacks[1-active];  // the number of chips your opponent has remaining
    //int continue_cost = opp_pip - my_pip;  // the number of chips needed to stay in the pot
    int my_contribution = STARTING_STACK - my_stack;  // the number of chips you have contributed to the pot
    int opp_contribution = STARTING_STACK - opp_stack;  // the number of chips your opponent has contributed to the pot

    if(my_contribution ==2)
    {
        if(opp_contribution !=6) // need to include round cap (round <250)
        {
            on_check_fold = true;
            return FoldAction();
        }
    } 
    if(my_contribution ==24)
    {
        if(opp_contribution != 24 && opp_contribution !=62)
        {
            on_check_fold = true;
            return FoldAction();
        }
    } 


    // calling jams with top 69 hands
    if (opp_contribution == 200) {
        for (int i = 1326-69; i < 1326; i++) {
            if ((my_cards[0] == hand_ranking[i].first && my_cards[1] == hand_ranking[i].second) || (my_cards[1] == hand_ranking[i].first && my_cards[0] == hand_ranking[i].second)) {
                return CallAction();
            }
        }
    }

    // if opponent just limped/opened
    if (my_contribution == 2) {
        if (opp_contribution == 2) {
            for (int i = 0; i < 300; i++) {
                if ((my_cards[0] == hand_ranking[i].first && my_cards[1] == hand_ranking[i].second) || (my_cards[1] == hand_ranking[i].first && my_cards[0] == hand_ranking[i].second)) {
                    return CheckAction();
                }
            }
            return RaiseAction(6);
        } else {
            double freq_def =min(1.5*(1 - 1.0 * (opp_contribution - opp_4b_size) / (opp_contribution + my_contribution)),1.0);;
            int defend_cnt = freq_def*1326;
            int cnt_3b = defend_cnt*3/10;
            int bottom_of_continuing_range = 1326-defend_cnt;
            int sizing_3b = opp_contribution * 4;
            for (int i = bottom_of_continuing_range; i < 1326; i++) {
                if ((my_cards[0] == hand_ranking[i].first && my_cards[1] == hand_ranking[i].second) || (my_cards[1] == hand_ranking[i].first && my_cards[0] == hand_ranking[i].second)) {
                    // 3 bet
                    if (i >= 1326 - cnt_3b) {
                        if (legal_actions & RAISE_ACTION_TYPE) {
                            std::array<int, 2> raise_bounds = round_state->raise_bounds();
                            sizing_3b = max(sizing_3b, raise_bounds[0]);
                            if (sizing_3b < raise_bounds[1]) {
                                return RaiseAction(sizing_3b);
                            }
                            return RaiseAction(raise_bounds[1]);
                        }
                        return CallAction();
                    }
                    return CallAction();
                }
            }
            return FoldAction();
        }

    }
    double freq_def = min(1*(1 - 1.0 * (opp_contribution - opp_4b_size) / (opp_contribution + my_contribution)),1.0);
    opp_4b_size = opp_contribution;
    int defend_cnt = freq_def*1326;
    int cnt_3b = defend_cnt*3/10;
    int bottom_of_continuing_range = 1326-defend_cnt;
    int sizing_3b = opp_contribution * 4;
    for (int i = bottom_of_continuing_range; i < 1326; i++) {
        if ((my_cards[0] == hand_ranking[i].first && my_cards[1] == hand_ranking[i].second) || (my_cards[1] == hand_ranking[i].first && my_cards[0] == hand_ranking[i].second)) {
            // 3 bet
            if (i >= 1326 - cnt_3b) {
                if (legal_actions & RAISE_ACTION_TYPE) {
                    std::array<int, 2> raise_bounds = round_state->raise_bounds();
                    sizing_3b = max(raise_bounds[0], sizing_3b);
                    if (sizing_3b < raise_bounds[1]) {
                        return RaiseAction(sizing_3b);
                    }
                    return RaiseAction(raise_bounds[1]);
                }
                return CallAction();
            }
            return CallAction();
        }
    }
    return FoldAction();
    
    /*if (opp_contribution == )
    double MDF = 1 - 1.0 * (opp_contribution - opp_4b_size) / (opp_contribution + my_contribution);*/

}

/**
 * Where the magic happens - your code should implement this function.
 * Called any time the engine needs an action from your bot.
 *
 * @param game_state Pointer to the GameState object.
 * @param round_state Pointer to the RoundState object.
 * @param active Your player's index.
 * @return Your action.
 */
Action Player::get_action(GameState* game_state, RoundState* round_state, int active) {
    int legal_actions = round_state->legal_actions();  // mask representing the actions you are allowed to take
    int street = round_state->street;  // 0, 3, 4, or 5 representing pre-flop, flop, turn, or river respectively
    std::array<std::string, 2> my_cards = round_state->hands[active];  // your cards
    my_cards[0][0]=rankrmap[rank_map[my_cards[0][0]]];
    my_cards[1][0]=rankrmap[rank_map[my_cards[1][0]]];
    std::array<std::string, 5> board_cards = round_state->deck;  // the board cards
    for (int i = 0; i < street; i++) {
        board_cards[i][0] = rankrmap[rank_map[board_cards[i][0]]];
    }
    int my_pip = round_state->pips[active];  // the number of chips you have contributed to the pot this round of betting
    int opp_pip = round_state->pips[1-active];  // the number of chips your opponent has contributed to the pot this round of betting
    int my_stack = round_state->stacks[active];  // the number of chips you have remaining
    int opp_stack = round_state->stacks[1-active];  // the number of chips your opponent has remaining
    int continue_cost = opp_pip - my_pip;  // the number of chips needed to stay in the pot
    int my_contribution = STARTING_STACK - my_stack;  // the number of chips you have contributed to the pot
    int opp_contribution = STARTING_STACK - opp_stack;  // the number of chips your opponent has contributed to the pot
    int pot = my_contribution + opp_contribution;
    if (street == 3 && pot == 400) {
        all_in_pre = true;
    }
    if ( legal_actions == CHECK_ACTION_TYPE ) {
        return CheckAction();
    }
    if (((NUM_ROUNDS - game_state->round_num + 1)*3 + 1) / 2 < game_state->bankroll) {
        on_check_fold = true;
    }
    if (on_check_fold) {
        return check_fold(game_state, round_state, active);
    }
    if (street == 0) {
        if (big_blind) return preflop_BB(game_state, round_state, active);
        return preflop_BTN(game_state, round_state, active);
    }
    
    mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	uniform_int_distribution<int> R(0,99);

    int pair_cnt = 0;
    int suited_cnt = 0;
    int board_suited_cnt = 0;

    // compute above vars
    map<char, int> suit_cnts;
    suit_cnts[my_cards[0][1]]++;
    suit_cnts[my_cards[1][1]]++;
    if (my_cards[0][0] == my_cards[1][0]) {
        pair_cnt++;
    }
    for (int i = 0; i < street; i++) {
        if (my_cards[0][0] == board_cards[i][0] || my_cards[1][0] == board_cards[i][0]) {
            pair_cnt++;
        }
        suit_cnts[board_cards[i][1]]++;
    }
    for (auto it : suit_cnts) {
        suited_cnt = max(suited_cnt, it.second);
    }
    suit_cnts[my_cards[0][1]]--;
    suit_cnts[my_cards[1][1]]--;
    for (auto it : suit_cnts) {
        board_suited_cnt = max(board_suited_cnt, it.second);
    }

    vector<string> my_hand = {my_cards[0], my_cards[1]};
    for (int i = 0; i < street; i++) {
        my_hand.push_back(board_cards[i]);
    }
    int my_hand_type = hand_type(my_hand);

    // flop play
    if (street == 3) {
        bool dont_raise = false;
        if (my_pip > 0 && opp_pip > 0) {
            dont_raise = true;
        }
        // exploitatively bet 30% whenever possible
        if (4*continue_cost <= pot-continue_cost) {
            int bet_size = max(2, 3*(pot+continue_cost)/10);
            if ((legal_actions & RAISE_ACTION_TYPE) && (!dont_raise)) {
                std::array<int, 2> raise_bounds = round_state->raise_bounds();
                bet_size = min(bet_size, raise_bounds[1]);
                bet_size = max(raise_bounds[0], bet_size);
                return RaiseAction(bet_size);
            }
            return CallAction();
        }

        // backdoor flush draw with no pair (<=4.2% of hitting)
        if (pair_cnt == 0 && suited_cnt == 3 && board_suited_cnt < suited_cnt && my_cards[0][1] == my_cards[1][1]) {
            if (R(rng) < 50) {
                return CallAction();
            }
            return FoldAction();
        }

        // flush draw with no pair
        if (pair_cnt == 0 && suited_cnt == 4 && board_suited_cnt < suited_cnt) {
            // TODO: take into account monotone flops
            if (R(rng) < 50 && (!dont_raise)) {
                return min_raise(game_state, round_state, active);
            }
            return CallAction();
        }

        // flush draw with a pair
        if (pair_cnt > 0 && suited_cnt == 4 && board_suited_cnt < suited_cnt) {
            if (R(rng) < 50 && !dont_raise) {
                return min_raise(game_state, round_state, active);
            }
            return CallAction();
        }

        // call half our flushes and raise the rest
        if (suited_cnt >= 5) {
            if (R(rng) < 50 && !dont_raise) {
                return min_raise(game_state, round_state, active);
            }
            return CallAction();
        }

        // fold our air to a raise
        if (pair_cnt == 0) {
            return check_fold(game_state, round_state, active);
        }

        if (pair_cnt == 1) {
            return CallAction();
        }

        // value hands
        if (R(rng) < 40 && !dont_raise) {
            return jam(game_state, round_state, active);
        } else if (R(rng) < 40 && !dont_raise) {
            return min_raise(game_state, round_state, active);
        } else {
            return check_call(game_state, round_state, active);
        }

        assert(0);
    }
    bool dont_raise = false;
    if (my_pip>0 && opp_pip>0) {
        dont_raise=true;
    }
    // unmade hands
    if (pair_cnt == 0 && suited_cnt < 5) {
        if (suited_cnt == 4 && street == 4 && board_suited_cnt < suited_cnt) {
            if (R(rng) < 40) {
                return min_raise(game_state, round_state, active);
            } else {
                // this is weak
                return check_call_under_pot(game_state, round_state, active);
            }
        }
        // high card
        if (4*continue_cost <= pot-continue_cost) {
            int bet_size = max(2, 3*(pot+continue_cost)/10);
            if ((legal_actions & RAISE_ACTION_TYPE) && (!dont_raise)) {
                std::array<int, 2> raise_bounds = round_state->raise_bounds();
                bet_size = min(bet_size, raise_bounds[1]);
                bet_size = max(raise_bounds[0], bet_size);
                return RaiseAction(bet_size);
            }
            return CheckAction();
        }
        return check_fold(game_state, round_state, active);
    }
    // value
    if ((my_hand_type == 2 && suited_cnt < 4 && pair_cnt >= 2) // two pair
        || (my_hand_type == 3 && suited_cnt < 4 && pair_cnt >= 2) // trips
        || (my_hand_type == 4) // flush (TODO: consider 4-flush boards)
        || (my_hand_type > 4)) { // nuts (TODO: double paired boards or triple boards or quad boards)
        if (R(rng) < 40) {
            return jam(game_state, round_state, active);
        } else if (R(rng) < 80) {
            return min_raise(game_state, round_state, active);
        } else {
            return check_call(game_state, round_state, active);
        }
    }
    // one pair
    if (pair_cnt == 1 || suited_cnt >= 4) {
        // fold non-flush on a 4-flush board unless small bet and 2p+
        if (2*continue_cost <= pot-continue_cost) { 
            return check_call(game_state, round_state, active);
        }
        if (board_suited_cnt == 4) {
            if (4*continue_cost <= pot-continue_cost) {
                int bet_size = max(2, 3*(pot+continue_cost)/10);
                if ((legal_actions & RAISE_ACTION_TYPE) && (!dont_raise)) {
                    std::array<int, 2> raise_bounds = round_state->raise_bounds();
                    bet_size = min(bet_size, raise_bounds[1]);
                    bet_size = max(raise_bounds[0], bet_size);
                    return RaiseAction(bet_size);
                }
                return CheckAction();
            }
            return check_fold(game_state, round_state, active);
        }
        // raise with 1 pair
        if (suited_cnt == 4 && board_suited_cnt < suited_cnt && R(rng) < 50 && (legal_actions & RAISE_ACTION_TYPE) && street == 4) {
            return min_raise(game_state, round_state, active);
        }
        if (continue_cost <= pot-continue_cost && board_suited_cnt == 4) { 
            return check_call(game_state, round_state, active);
        }
        if (4*continue_cost <= pot-continue_cost) {
            int bet_size = max(2, 3*(pot+continue_cost)/10);
            if ((legal_actions & RAISE_ACTION_TYPE) && (!dont_raise)) {
                std::array<int, 2> raise_bounds = round_state->raise_bounds();
                bet_size = min(bet_size, raise_bounds[1]);
                bet_size = max(raise_bounds[0], bet_size);
                return RaiseAction(bet_size);
            }
            return CheckAction();
        }
        return check_fold(game_state, round_state, active);
    }

    // shouldn't get here
    assert(0);
    return FoldAction();
}
