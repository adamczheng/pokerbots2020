/**
 * Simple example pokerbot, written in C++.
 */
#include "player.hpp"
#include <bits/stdc++.h>
using namespace std;

/**
 * Called when a new game starts. Called exactly once.
 */
Player::Player()
{
}
bool on_check_fold;
bool play_passive;
int pair_cnt;
int suited_cnt;
int jam_street;
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
    jam_street = 100;
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
        if (a[0] == b[0]) {
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
    int my_delta = terminal_state->deltas[active];  // your bankroll change from this round
    if (my_delta < -100) {
        cout << "Lost " << my_delta << " on hand #" << (game_state->round_num) << " with " << "TODO" << '\n';
        cout << "jam street: " << jam_street << '\n';
    }
    //game_state = new GameState(game_state->bankroll + my_delta, game_state->game_clock, game_state->round_num + 1, )->bankroll += my_delta;
    //RoundState* previous_state = (RoundState*) terminal_state->previous_state;  // RoundState before payoffs
    //int street = previous_state->street;  // 0, 3, 4, or 5 representing when this round ended
    //std::array<std::string, 2> my_cards = previous_state->hands[active];  // your cards
    //std::array<std::string, 2> opp_cards = previous_state->hands[1-active];  // opponent's cards or "" if not revealed
}

Action check_fold(GameState* game_state, RoundState* round_state, int active) {
    int legal_actions = round_state->legal_actions();
    if (CHECK_ACTION_TYPE & legal_actions)  // check-call
    {
        return CheckAction();
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

Action jam(GameState* game_state, RoundState* round_state, int active) {
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
    //int my_stack = round_state->stacks[active];  // the number of chips you have remaining
    int opp_stack = round_state->stacks[1-active];  // the number of chips your opponent has remaining
    int continue_cost = opp_pip - my_pip;  // the number of chips needed to stay in the pot
    //int my_contribution = STARTING_STACK - my_stack;  // the number of chips you have contributed to the pot
    //int opp_contribution = STARTING_STACK - opp_stack;  // the number of chips your opponent has contributed to the pot
    //cout << (game_state->round_num) << ' ' << (game_state->bankroll) << ' ' << (game_state->game_clock) << endl;
    if (((NUM_ROUNDS - game_state->round_num + 1)*3 + 1) / 2 < game_state->bankroll) {
        on_check_fold = true;
    }
    if (on_check_fold) {
        return check_fold(game_state, round_state, active);
    }
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
    if (play_passive) {
        if (street == 0) {
            if (continue_cost < 10) {
                return check_call(game_state, round_state, active);
            } else {
                return check_fold(game_state, round_state, active);
            }
        }
        if (street > 0 && pair_cnt == 0) {
            if (suited_cnt == 4 && street < 5) {
                if (opp_stack == 0) {
                    if (jam_street == 100) jam_street = street;
                }
                return check_call(game_state, round_state, active);
            }
            if (suited_cnt >= 5) {
                if (jam_street == 100) jam_street = street;
                return jam(game_state, round_state, active);
            }
            return check_fold(game_state, round_state, active);
        }
        if (pair_cnt > 1 || suited_cnt >= 5) {
            if (jam_street == 100) jam_street = street;
            return jam(game_state, round_state, active);
        }
        if (pair_cnt == 1) {
            if (opp_stack == 0) {
                if (jam_street == 100) jam_street = street;
            }
            return check_call(game_state, round_state, active);
        }
    }
    if (jam_street == 100) jam_street = street;
    return jam(game_state, round_state, active);
}
