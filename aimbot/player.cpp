/**
 * Simple example pokerbot, written in C++.
 */
#include "player.hpp"
#include <iostream>
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
map<char, int> rank_map, suit_map;
Player::Player()
{
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
bool play_passive;
int pair_cnt;
int suited_cnt;

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
void Player::handle_new_round(GameState* game_state, RoundState* round_state, int active)
{
    //int my_bankroll = game_state->bankroll;  // the total number of chips you've gained or lost from the beginning of the game to the start of this round
    //float game_clock = game_state->game_clock;  // the total number of seconds your bot has left to play this game
    //int round_num = game_state->round_num;  // the round number from 1 to NUM_ROUNDS
    //std::array<std::string, 2> my_cards = round_state->hands[active];  // your cards
    //bool big_blind = (bool) active;  // true if you are the big blind
    on_check_fold = false;
    play_passive = false;
    pair_cnt = 0;
    suited_cnt = 0;
    int street = round_state->street;
    assert(street == 0);
    std::array<std::string, 2> my_cards = round_state->hands[active];
    if (street == 0) {
        string a = my_cards[0];
        string b = my_cards[1];
        int cnt = 0;
        if (a[0] == 'T' || a[0] == 'J' || a[0] == 'Q' || a[0] == 'K' || a[0] == 'A') {
            cnt++;
        }
        if (b[0] == 'T' || b[0] == 'J' || b[0] == 'Q' || b[0] == 'K' || b[0] == 'A') {
            cnt++;
        }
        if (a[0] == b[0] && a[0] != '2' && a[0] != '3') {
            cnt += 2;
        }
        if (cnt < 2) {
            play_passive = true;
        }
    }
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
    if (my_delta < -50) {
        cout << "Lost " << my_delta << " on hand #" << (game_state->round_num) << 
         " on street " << street << " with " << hand_type_convert[hand_type(my_hand)] << " vs " << hand_type_convert[hand_type(opp_hand)] << '\n';
    }
    //game_state = new GameState(game_state->bankroll + my_delta, game_state->game_clock, game_state->round_num + 1, )->bankroll += my_delta;
    
}

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

Action check_call(GameState* game_state, RoundState* round_state, int active) {
    int legal_actions = round_state->legal_actions();
    if (CHECK_ACTION_TYPE & legal_actions)  // check-call
    {
        return CheckAction();
    }
    return CallAction();
}



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

Action min_raise(GameState* game_state, RoundState* round_state, int active) {
    int legal_actions = round_state->legal_actions();
    if (RAISE_ACTION_TYPE & legal_actions)  // check-call
    {
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
    int legal_actions = round_state->legal_actions();  // mask representing the actions you are allowed to take
    //cout << legal_actions << endl;
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
    //cout << (game_state->round_num) << ' ' << (game_state->bankroll) << ' ' << (game_state->game_clock) << endl;
    if ( legal_actions == CHECK_ACTION_TYPE ) {
        return CheckAction();
    }
    if (((NUM_ROUNDS - game_state->round_num + 1)*3 + 1) / 2 < game_state->bankroll) {
        on_check_fold = true;
    }
    if (on_check_fold) {
        return check_fold(game_state, round_state, active);
    }

    mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	uniform_int_distribution<int> R(0,99);

    pair_cnt = 0;
    
    suited_cnt = 0;
    map<char, int> suit_cnts;
    suit_cnts[my_cards[0][1]]++;
    suit_cnts[my_cards[1][1]]++;
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
    int board_suited_cnt = 0;
    for (auto it : suit_cnts) {
        board_suited_cnt = max(board_suited_cnt, it.second);
    }
    if (play_passive) {
        if (street == 0) {
            if (continue_cost < 10 && (rank_map[my_cards[0][0]] > 9 || rank_map[my_cards[1][0]] > 9)) {
                return check_call(game_state, round_state, active);
            } else {
                return check_fold(game_state, round_state, active);
            }
        }
        if (street > 0 && pair_cnt == 0) {
            // flush draw
            /*if (suited_cnt == 4 && street < 5) {
                if (opp_stack == 0) {
                    if (jam_street == 100) jam_street = street;
                }
                return check_call(game_state, round_state, active);
            }*/
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
                    return check_call(game_state, round_state, active);
                }
            }
            if (suited_cnt == 4 && street == 43 && board_suited_cnt < suited_cnt) {
                if (R(rng) < 40) {
                    return min_raise(game_state, round_state, active);
                } else {
                    // this is weak
                    return check_call(game_state, round_state, active);
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
    }
    return jam(game_state, round_state, active);
}
