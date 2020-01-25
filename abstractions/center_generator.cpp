#include <bits/stdc++.h>
//using namespace omp;
using namespace std;
/***** BEGIN KMEANS CODE *****/
typedef vector<double> Point;
const int POINT_DIM = 51;
double euclidean_dist_squared(const Point &a, const Point &b) {
	assert(a.size() == b.size());
	double res = 0;
	for (int d = 0; d < (int)a.size(); d++)
	{
		res += (a[d] - b[d]) * (a[d] - b[d]);
	}
	return res;
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

int findClusterID(vector<Point> &centers, Point pt)
{
	int cid = 0;
	double best_dist = EMD(centers[0], pt);
	for (int i = 1; i < (int)centers.size(); i++)
	{
		double cur_dist = EMD(centers[i], pt);
		if (cur_dist < best_dist)
		{
			cid = i;
			best_dist = cur_dist;
		}
	}
	return cid;
}
ofstream fout;
void kmeans(vector<Point> &A, int K)
{
	mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	//init
	vector<Point> centers(K);
	// kmeans++ initialization
	uniform_int_distribution<int> int_dist(0, A.size()-1);
	centers[0] = A[int_dist(rng)];
	for (int i = 1; i < K; i++)
	{
		double sum = 0;
		vector<double> min_dist(A.size());
		for (int j = 0; j < (int)A.size(); j++) {
			min_dist[j] = EMD(centers[0], A[j]);
			for (int k = 1; k < i; k++)
			{
				double cur_dist = EMD(centers[k], A[j]);
				if (cur_dist < min_dist[j])
				{
					min_dist[j] = cur_dist;
				}
			}
			sum += min_dist[j];
		}
		uniform_real_distribution<double> real_dist(0, sum);
		double sample = real_dist(rng);
		for (int j = 0; j < (int)A.size(); j++) {
			sample -= min_dist[j];
			if (sample > 0) continue;
			centers[i] = A[j];
			break;
		}
	}
	vector<int> cluster(A.size(),-1);
	double lastErr = 0;
	while(true)
	{
		//assigning to cluster
		for(int i = 0; i<A.size(); i++)
		{
			int cid = findClusterID(centers, A[i]);
			cluster[i] = cid;
		}
		//recalculate centers per cluster
		vector<int> cnt(K, 0);
		vector<Point> sum(K, Point(centers[0].size(), 0));
		double err=0;
		for(int i = 0; i<A.size(); i++)
		{
			int cid = cluster[i];
			cnt[cid]++;
			for (int d = 0; d < (int)A[i].size(); d++)
			{
				sum[cid][d]+=A[i][d];
			}
		
			//error
			err+=EMD(A[i], centers[cid]);
		}
		double delta = abs(lastErr - err);
		cout<<"error "<<err<< " delta " << delta <<endl;
		if(delta < 0.1)break;
		lastErr = err;
		//assign new centers
		for(int i =0; i<K; i++)
		{
			for (int d = 0; d < (int)centers[i].size(); d++)
				centers[i][d] = sum[i][d]/cnt[i];
		}
	}
	// print out results
	for(int i = 0; i<K; i++)
	{
		for (int d = 0; d < POINT_DIM; d++)
			fout << centers[i][d] << ' ';
		fout << "\n";
		/*cout<<"Cluster Center "<<i<< " : ";
		cout<<"(" << centers[i][0];
		for (int d = 1; d < (int)centers[i].size(); d++)
			cout<<", "<<centers[i][d];
		cout<<")";
		cout<<endl;
		cout<<"Cluster Elements: ";
		for(int j=0; j<cluster.size(); j++)
		{
			if(cluster[j]==i)
			{
				cout << "(" << A[j][0];
				for (int d = 1; d < (int)A[j].size(); d++)
					cout<<", "<<A[j][d];
				cout << ") ";
			}
		}
		cout<<endl;*/
	}
}
/***** BEGIN 7-CARD-EVAL CODE *****/
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
int GetHandValue(int* pCards)
{
    int p = HR[53 + *pCards++];
    p = HR[p + *pCards++];
    p = HR[p + *pCards++];
    p = HR[p + *pCards++];
    p = HR[p + *pCards++];
    p = HR[p + *pCards++];
    return HR[p + *pCards++];
}

// Set up a 7-card poker hand, perform the lookup into the table, and
// extract the category (full house, two pair, flush, etc.) and rank
// (within that category) of the hand. These numbers can be used to
// compare the hand to other hands.
void DoSomeWork()
{
    int myCards[] = { 1, 5, 9, 13, 51, 49 };
    int handInfo = GetHandValue(myCards);
    int handCategory = handInfo >> 12;
    int rankWithinCategory = handInfo & 0x00000FFF;
}

int convert(string s) {
	char rank = s[0];
	char suit = s[1];
	string ranks = "23456789TJQKA";
	string suits = "shcd";
	int r, ss;
	for (int i = 0; i < 13; i++)
		if (rank == ranks[i])
			r = i;
	for (int i = 0; i < 4; i++)
		if (suit == suits[i])
			ss = i;
	return 4 * r + ss + 1;
}

string deconvert(int n) {
	int r = n/4;
	int s = n%4;
	string ranks = "23456789TJQKA";
	string suits = "shcd";
	string res = "";
	res += ranks[r]+suits[s];
	return res;
}

// AKo or AKs or AA to a vector of card indices (0-indexed)
vector<pair<int, int> > convertt(string s) {
	string ranks = "23456789TJQKA";
	string suits = "shcd";
	map<char,int> rank_rev, suit_rev;
	int i = 0;
	for (char c : ranks) rank_rev[c] = i++;
	i = 0;
	for (char c : suits) suit_rev[c] = i++;
	vector<pair<int, int> > res;
	if (s.size() == 2) {
		assert(s[0] == s[1]);
		// pocket pairs
		int f = 4 * rank_rev[s[0]];
		for (int i = 0; i < 4; i++)
			for (int j = i + 1; j < 4; j++)
				res.push_back(make_pair(f + i, f + j));
		return res;
	}
	assert(s.size() == 3);
	int r1 = 4 * rank_rev[s[0]];
	int r2 = 4 * rank_rev[s[1]];
	if (s.back() == 's') {
		// suited
		for (int i = 0; i < 4; i++)
			res.push_back(make_pair(r1 + i, r2 + i));
	} else {
		assert(s.back() == 'o');
		// unsuited
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				if (i == j) continue;
				res.push_back(make_pair(r1 + i, r2 + j));
			}
		}
	}
	return res;
}

string pf_cluster[8] = {"23s,24s,25s,26s,27s,34s,35s,36s,37s,45s,46s,32o,43o,42o,54o,53o,52o,65o,64o,63o,62o,74o,73o,72o,83o,82o",
"28s,29s,2Ts,38s,39s,47s,48s,49s,75o,85o,84o,95o,94o,93o,92o,T5o,T4o,T3o,T2o,J3o,J2o",
"3Ts,4Ts,56s,57s,58s,59s,5Ts,67s,68s,69s,6Ts,78s,79s,89s,67o,68o,69o,6To,78o,79o,7To,89o,8To",
"22,J2s,J3s,J4s,J5s,J6s,Q2s,Q3s,Q4s,Q5s,K2s,J4o,J5o,J6o,J7o,Q2o,Q3o,Q4o,Q5o,Q6o,Q7o,K2o,K3o,K4o",
"6Qs,7Ts,7Js,7Qs,8Ts,8Js,8Qs,9Ts,9Js,9Qs,TJs,T9o,J8o,J9o,JTo,Q8o,Q9o,QTo,QJo",
"33,44,55,K3s,K4s,K5s,K6s,K7s,K8s,A2s,A3s,A4s,A5s,A6s,K5o,K6o,K7o,K8o,K9o,A2o,A3o,A4o,A5o,A6o,A7o,A8o",
"66,77,QTs,QJs,K9s,KTs,KJs,KQs,A7s,A8s,A9s,ATs,AJs,AQs,AKs,KTo,KJo,KQo,A9o,ATo,AJo,AQo,AKo",
"88,99,TT,JJ,QQ,KK,AA"};
vector<pair<int, int> > preflop_cluster[8];

void split_clusters() {
	for (int i = 0; i < 8; i++) {
		vector<string> split_cluster;
		istringstream ss(pf_cluster[i]);
		string token;
		while(getline(ss,token,',')) {
			split_cluster.push_back(token);
			//cout << token << endl;
		}
		for (string s : split_cluster) {
			vector<pair<int, int> > tmp = convertt(s);
			for (pair<int, int>  x : tmp) preflop_cluster[i].push_back(x);
		}
	}
}

int isomorphize(int h1, int h2) {
	int r1 = h1 / 4;
	int r2 = h2 / 4;
	int s1 = h1 % 4;
	int s2 = h2 % 4;
	if (r1 > r2) swap(r1, r2);
	int suited = 0;
	if (s1 == s2) suited = 1;
	return (r1 * 100 + r2) * 10 + suited;
}

int main() {
	/*set<int> ssss;
	for (int i = 0; i < 52; i++)
		for (int j = i+1; j < 52; j++)
			ssss.insert(isomorphize(i, j));
	cout << ssss.size() << endl;*/
	ifstream fin("flophistograms.txt");
	fout.open("flopcenters.txt");
	fout << fixed << setprecision(4);
	vector<Point> A;
	for (int i = 0; i < 200000; i++) {
		Point p(POINT_DIM);
		for (int j = 0; j < POINT_DIM; j++)
			fin >> p[j];
		A.push_back(p);
	}
	cout << "Starting" << endl;
	int K = 100;
	kmeans(A, K);

	InitTheEvaluator();
	split_clusters();	

	// generate river OCHS
	vector<vector<double> > values;
	for (int iter = 0; iter < 200000; iter++) {
		bitset<53> bs;
		auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
		mt19937 rng(seed);
		uniform_int_distribution<int> distribution(1, 52);
		int a[7];
		for (int i = 0; i < 7; i++) {
			int x;
			do {
				x = distribution(rng);
			} while(bs[x]);
			a[i] = x;
			bs[x] = 1;
		}
		int my_value = GetHandValue(a);
		vector<double> ochs(8);
		for (int i = 0; i < 8; i++){
			int tot=0,my_tot=0;
			for (pair<int, int> p : preflop_cluster[i]) {
				if (bs[p.first + 1] || bs[p.second + 1]) continue;
				a[5] = p.first + 1;
				a[6] = p.second + 1;
				int opp_value = GetHandValue(a);
				if (my_value > opp_value) my_tot += 2;
				else if (my_value == opp_value) my_tot += 1;
				tot += 2;
			}
			ochs[i] = 1.0*my_tot/tot;
		}
		values.push_back(ochs);
		bs.reset();
	}
	return 0;
}

