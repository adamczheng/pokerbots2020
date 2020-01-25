double utility_recursive(State state) {
    if (state.is_terminal) {
        return state.evaluation;
    }
    double utility = 0;
    for (int action : state.actions) {
        utility += this.sigma[state.info_set][action] * this.utility_recursive(state.play(action));
    }
    return utility;
}

double cfr_utility_recursive(State state, double reach_a, double reach_b) {
    map<int, double> child_state_utilities;
    if (state.is_terminal) {
        return state.evaluation;
    }
    if (state.is_chance) {
        vector<State> outcomes;
        for (int action : state.actions) {
            outcomes.push_back(state.play(action));
        }
        double res = 0;
        // add P(c)*cfr_utility_recursive(c,reach_a,reach_b) ofr c in outcomes
        return res;
    }
    double utility = 0;
    double child_reach_a, child_reach_b;
    double child_state_utility;
    for (int action : state.actions) {
        if (state.to_move == A) {
            child_reach_a = reach_a * this.sigma[state.info_set][action];
            child_reach_b = 1;
        } else {
            assert(state.to_move == -A)
            child_reach_a = 1;
            child_reach_b = reach_b * this.sigma[state.info_set][action];
        }
        child_state_utility = this.cfr_utility_recursive(state.play(action), child_reach_a, child_reach_b);
        utility += this.sigma[state.info_set][action] * child_state_utility;
        child_state_utilities[action] = child_state_utility;
    }
    double cfr_reach = (state.to_move == A ? reach_b : reach_a);
    for (int action : state.actions) {
        action_cfr_regret = state.to_move * cfr_reach * child_state_utilities[action];
    }
    return utility;
}