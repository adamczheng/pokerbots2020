/**
 * Simple example pokerbot, written in C++.
 */
#include "player.hpp"
#include <iostream>
#include <fstream>
#include <random>
#include <cassert>
#include <map>
#include <chrono>
#include <cmath>
#include <algorithm>
using namespace std;

/**
 * Called when a new game starts. Called exactly once.
 */
int preflop_delta;
vector<pair<string,string> > hand_ranking;
map<char, int> rank_map, suit_map;
Player::Player() {
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
    suit_map['s']=0;
    suit_map['h']=1;
    suit_map['d']=2;
    suit_map['c']=3;
}
bool on_check_fold;
bool big_blind;
int opp_3b_size;
int cnt_4b_value;
int opp_4b_size;
int small_pots_won, small_pots_lost, big_pots_won, big_pots_lost, chopped_pots; // small is <= 50 chips
int spw,spl,bpw,bpl,cp; // same as variables above but only for first 500 rounds

struct Card {
	int rank, suit;
};
const string rank_convert[15]={"error0","error1","2","3","4","5","6","7","8","9","T","J","Q","K","A"};
const string hand_type_convert[8]={"high card","pair","two pair","3 of a kind","flush","full house","quads","unknown"};
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
    if (oa == 6) {
        cout << "Quads with: ";
        for (const string &c : cards) {
            cout << c << ' ';
        }
        cout<<endl;
    }
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
    //std::array<std::string, 2> my_cards = round_state->hands[active];  // your cards
    big_blind = (bool) active;  // true if you are the big blind
    opp_3b_size = 2;
    opp_4b_size = 1;
    on_check_fold = false;
    int street = round_state->street;
    assert(street == 0);
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
    if (game_state->round_num <= 500) {
        if (my_delta < -50) {
            cout << my_delta << ", round #" << (game_state->round_num) << 
            ", street " << street << ", " << hand_type_convert[hand_type(my_hand)] << " vs " << hand_type_convert[hand_type(opp_hand)] << '\n';
            big_pots_lost++;
            bpl++;
        } else if (my_delta > 50) {
            cout << my_delta << ", hand #" << (game_state->round_num) << 
            ", street: " << street << ", " << hand_type_convert[hand_type(my_hand)] << " vs " << hand_type_convert[hand_type(opp_hand)] << '\n';
            big_pots_won++;
            bpw++;
        } else if (my_delta == 0) {
            chopped_pots++;
            cp++;
        } else if (my_delta > 0) {
            small_pots_won++;
            spw++;
        } else {
            small_pots_lost++;
            spl++;
        }
    } else {
        if (my_delta < -50) {
            cout << my_delta << ", round #" << (game_state->round_num) << 
             ", street " << street << ", " << hand_type_convert[hand_type(my_hand)] << " vs " << hand_type_convert[hand_type(opp_hand)] << '\n';
            big_pots_lost++;
        } else if (my_delta > 50) {
            cout << my_delta << ", hand #" << (game_state->round_num) << 
            ", street: " << street << ", " << hand_type_convert[hand_type(my_hand)] << " vs " << hand_type_convert[hand_type(opp_hand)] << '\n';
            big_pots_won++;
        } else if (my_delta == 0) {
            chopped_pots++;
        } else if (my_delta > 0) {
            small_pots_won++;
        } else {
            small_pots_lost++;
        }
    }
    if (street == 0 && !on_check_fold) {
       preflop_delta += my_delta;
    }
    if (game_state->round_num == 1000) {
        cout << "pf delta: " << preflop_delta << endl;
        cout << "spw/spl/bpw/bpl/cp: " << small_pots_won << ' ' << small_pots_lost << ' ' << big_pots_won << ' ' << big_pots_lost << ' ' << chopped_pots << endl;
        cout << "first half:         " << spw << ' ' << spl << ' ' << bpw << ' ' << bpl << ' ' << cp << endl;
        cout << "second half:        " << small_pots_won - spw << ' ' << small_pots_lost - spl << ' ' << big_pots_won - bpw << ' ' << big_pots_lost - bpl << ' ' << chopped_pots - cp << endl;
    }
}

// check-fold, but calls a bet if size <= 1/5 pot
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
    if (10*continue_cost <= pot) {
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
    std::array<std::string, 2> my_cards = round_state->hands[active];  // your cards
    std::array<std::string, 5> board_cards = round_state->deck;  // the board cards
    int my_pip = round_state->pips[active];  // the number of chips you have contributed to the pot this round of betting
    int opp_pip = round_state->pips[1-active];  // the number of chips your opponent has contributed to the pot this round of betting
    int my_stack = round_state->stacks[active];  // the number of chips you have remaining
    int opp_stack = round_state->stacks[1-active];  // the number of chips your opponent has remaining
    int continue_cost = opp_pip - my_pip;  // the number of chips needed to stay in the pot
    int my_contribution = STARTING_STACK - my_stack;  // the number of chips you have contributed to the pot
    int opp_contribution = STARTING_STACK - opp_stack;  // the number of chips your opponent has contributed to the pot
    int initial_pot = 2 * max(my_contribution, opp_contribution);
    int all_in_pot_size = 400;
    int bet_size;
    if (street == 0) {
        bet_size = continue_cost + (int)pow((1.0*all_in_pot_size/initial_pot-1)/2*initial_pot, 0.25);
    } else if (street == 3) {
        bet_size = continue_cost + (int)pow((1.0*all_in_pot_size/initial_pot-1)/2*initial_pot, 0.33);
    } else if (street == 4) {
        bet_size = continue_cost + (int)pow((1.0*all_in_pot_size/initial_pot-1)/2*initial_pot, 0.5);
    } else if (street == 5) {
        bet_size = continue_cost + (int)(1.0*all_in_pot_size/initial_pot-1)/2*initial_pot;
    }
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
    int street = round_state->street;  // 0, 3, 4, or 5 representing pre-flop, flop, turn, or river respectively
    std::array<std::string, 2> my_cards = round_state->hands[active];  // your cards
    std::array<std::string, 5> board_cards = round_state->deck;  // the board cards
    int my_pip = round_state->pips[active];  // the number of chips you have contributed to the pot this round of betting
    int opp_pip = round_state->pips[1-active];  // the number of chips your opponent has contributed to the pot this round of betting
    int my_stack = round_state->stacks[active];  // the number of chips you have remaining
    int opp_stack = round_state->stacks[1-active];  // the number of chips your opponent has remaining
    int continue_cost = opp_pip - my_pip;  // the number of chips needed to stay in the pot
    int my_contribution = STARTING_STACK - my_stack;  // the number of chips you have contributed to the pot
    int opp_contribution = STARTING_STACK - opp_stack;  // the number of chips your opponent has contributed to the pot
    int pot = 2 * opp_contribution;
    int quarter_pot = pot / 4;
    if (RAISE_ACTION_TYPE & legal_actions)  {
        std::array<int, 2> raise_bounds = round_state->raise_bounds();  // the smallest and largest numbers of chips for a legal bet/raise
        //int min_cost = raise_bounds[0] - my_pip;  // the cost of a minimum bet/raise
        //int max_cost = raise_bounds[1] - my_pip;  // the cost of a maximum bet/raise
        if (continue_cost + quarter_pot < raise_bounds[0]) {
            return RaiseAction(raise_bounds[0]);
        }
        if (continue_cost + quarter_pot > raise_bounds[1]) {
            return RaiseAction(raise_bounds[1]);
        }
        return RaiseAction(continue_cost + quarter_pot);
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
    //int my_pip = round_state->pips[active];  // the number of chips you have contributed to the pot this round of betting
    //int opp_pip = round_state->pips[1-active];  // the number of chips your opponent has contributed to the pot this round of betting
    int my_stack = round_state->stacks[active];  // the number of chips you have remaining
    int opp_stack = round_state->stacks[1-active];  // the number of chips your opponent has remaining
    //int continue_cost = opp_pip - my_pip;  // the number of chips needed to stay in the pot
    int my_contribution = STARTING_STACK - my_stack;  // the number of chips you have contributed to the pot
    int opp_contribution = STARTING_STACK - opp_stack;  // the number of chips your opponent has contributed to the pot
    
    // check if RFI
    if (my_contribution == 1) {
        // check if in top 1074 hands
        for (int i = 0; i < 252; i++) {
            if ((my_cards[0] == hand_ranking[i].first && my_cards[1] == hand_ranking[i].second) || (my_cards[1] == hand_ranking[i].first && my_cards[0] == hand_ranking[i].second)) {
                return FoldAction();
            }
        }
        return RaiseAction(4);
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
                        if (sizing_4b >= raise_bounds[0] && sizing_4b <= raise_bounds[1]) {
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
    //int my_pip = round_state->pips[active];  // the number of chips you have contributed to the pot this round of betting
    //int opp_pip = round_state->pips[1-active];  // the number of chips your opponent has contributed to the pot this round of betting
    int my_stack = round_state->stacks[active];  // the number of chips you have remaining
    int opp_stack = round_state->stacks[1-active];  // the number of chips your opponent has remaining
    //int continue_cost = opp_pip - my_pip;  // the number of chips needed to stay in the pot
    int my_contribution = STARTING_STACK - my_stack;  // the number of chips you have contributed to the pot
    int opp_contribution = STARTING_STACK - opp_stack;  // the number of chips your opponent has contributed to the pot
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
            double freq_def = .6;
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
                            if (sizing_3b >= raise_bounds[0] && sizing_3b <= raise_bounds[1]) {
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
                    if (sizing_3b >= raise_bounds[0] && sizing_3b <= raise_bounds[1]) {
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
    std::array<std::string, 5> board_cards = round_state->deck;  // the board cards
    int my_pip = round_state->pips[active];  // the number of chips you have contributed to the pot this round of betting
    int opp_pip = round_state->pips[1-active];  // the number of chips your opponent has contributed to the pot this round of betting
    int my_stack = round_state->stacks[active];  // the number of chips you have remaining
    int opp_stack = round_state->stacks[1-active];  // the number of chips your opponent has remaining
    int continue_cost = opp_pip - my_pip;  // the number of chips needed to stay in the pot
    int my_contribution = STARTING_STACK - my_stack;  // the number of chips you have contributed to the pot
    int opp_contribution = STARTING_STACK - opp_stack;  // the number of chips your opponent has contributed to the pot
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

    // unmade hands
    if (street > 0 && pair_cnt == 0) {
        // flush
        if (suited_cnt >= 5) {
            return jam(game_state, round_state, active);
        }
        // raise with air
        if (suited_cnt == 3 && street == 3 && board_suited_cnt < suited_cnt) {
            if (R(rng) < 67) {
                return min_raise(game_state, round_state, active);
            } else {
                // this is weak
                return check_call_under_pot(game_state, round_state, active);
            }
        }
        if (suited_cnt == 4 && street == 4 && board_suited_cnt < suited_cnt) {
            if (R(rng) < 40) {
                return min_raise(game_state, round_state, active);
            } else {
                // this is weak
                return check_call_under_pot(game_state, round_state, active);
            }
        }
        // high card
        return check_fold(game_state, round_state, active);
    }
    // value
    if (pair_cnt > 1 || suited_cnt >= 5) {
        if (R(rng) < 70) {
            return jam(game_state, round_state, active);
        } else if (R(rng) < 90) {
            return min_raise(game_state, round_state, active);
        } else {
            return check_call(game_state, round_state, active);
        }
    }
    // one pair
    if (pair_cnt == 1) {
        int pot = my_contribution + opp_contribution;
        if (2*continue_cost <= pot) {
            // raise with 1 pair
            if (suited_cnt == 3 && board_suited_cnt < suited_cnt && R(rng) < 60 && (legal_actions & RAISE_ACTION_TYPE) && street == 3) {
                return min_raise(game_state, round_state, active);
            }
            if (suited_cnt == 4 && board_suited_cnt < suited_cnt && R(rng) < 50 && (legal_actions & RAISE_ACTION_TYPE) && street == 4) {
                return min_raise(game_state, round_state, active);
            }
            return check_call(game_state, round_state, active);
        }
        // raise with 1 pair
        if (suited_cnt == 3 && board_suited_cnt < suited_cnt && R(rng) < 85 && (legal_actions & RAISE_ACTION_TYPE) && street == 3) {
            return min_raise(game_state, round_state, active);
        }
        if (suited_cnt == 4 && board_suited_cnt < suited_cnt && R(rng) < 60 && (legal_actions & RAISE_ACTION_TYPE) && street == 4) {
            return min_raise(game_state, round_state, active);
        }
        return check_fold(game_state, round_state, active);
    }
    // shouldn't get here
    assert(0);
    return FoldAction();
}
