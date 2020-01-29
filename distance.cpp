#include <bits/stdc++.h>

using namespace std;

int reinsert_dist(const vector<int> &perm1, const vector<int> &perm2){
    int n = perm1.size();
    array<array<int, 14>, 14> dp;
    for (int i = 0; i < n+1; ++i){
        for (int j = 0; j < n+1; ++j){
            dp[i][j] = 0;
        }
    }
    for (int i = 0; i < n; ++i){
        for (int j = 0; j < n; ++j){
            if (perm1[i] == perm2[j]){
                dp[i+1][j+1] = dp[i][j]+1;
            }
            else{
                dp[i+1][j+1] = max(dp[i][j+1],dp[i+1][j]);
            }
        }
    }
    return n - dp[n][n];
}

int swap_dist(const vector<int> &perm1, const vector<int> &perm2){
    array<int, 13> inverse_p;
    array<int, 13> invert_q;
    int n = perm1.size();
    for (int i = 0; i < n; ++i){
        auto it = find(perm1.begin(), perm1.end(), i);
        int p = distance(perm1.begin(),it);
        inverse_p[i] = p;
        //cout << inverse_p[i] << endl;
    }
    for (int i = 0; i < n; ++i){
        int q = perm2[inverse_p[i]];
        invert_q[i] = q;
        //cout << invert_q[i] << endl;
    }
    array<bool, 13> not_visited;
    for (int i = 0; i < n; ++i){
        not_visited[i] = true;
    }
    int cycles = 0;
    for (int i = 0; i < n; ++i){
        int curr_pos = i;
        if (not_visited[curr_pos]){
            cycles++;
        }
        while (not_visited[curr_pos]){
            not_visited[curr_pos] = false;
            curr_pos = invert_q[curr_pos];
        }
    }
    return n - cycles;
}

int main(){
    vector<int> p2 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    vector<int> p1 = {12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 0, 10};
    int rein = reinsert_dist(p1, p2);
    int swap = swap_dist(p1, p2);
    cout << rein << endl;
    cout << swap << endl;
    return 0;
}
