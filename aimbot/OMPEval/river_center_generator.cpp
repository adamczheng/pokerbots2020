#include <bits/stdc++.h>
#include "omp/EquityCalculator.h"
#include "omp/CardRange.h"
using namespace omp;
using namespace std;
/***** BEGIN KMEANS CODE *****/
/*double euclidean_dist_squared(const Point &a, const Point &b) {
	assert(a.size() == b.size());
	double res = 0;
	for (int d = 0; d < (int)a.size(); d++)
	{
		res += (a[d] - b[d]) * (a[d] - b[d]);
	}
	return res;
}
int findClusterID(vector<Point> &centers, Point pt)
{
	int cid = 0;
	double best_dist = euclidean_dist_squared(centers[0], pt);
	for (int i = 1; i < (int)centers.size(); i++)
	{
		double cur_dist = euclidean_dist_squared(centers[i], pt);
		if (cur_dist < best_dist)
		{
			cid = i;
			best_dist = cur_dist;
		}
	}
	return cid;
}
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
			min_dist[j] = euclidean_dist_squared(centers[0], A[j]);
			for (int k = 1; k < i; k++)
			{
				double cur_dist = euclidean_dist_squared(centers[k], A[j]);
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
			err+=euclidean_dist_squared(A[i], centers[cid]);
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
		cout<<"Cluster Center "<<i<< " : ";
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
		cout<<endl;
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



string pf_cluster[8] = {"23s,24s,25s,26s,27s,34s,35s,36s,37s,45s,46s,32o,43o,42o,54o,53o,52o,65o,64o,63o,62o,74o,73o,72o,83o,82o",
"28s,29s,2Ts,38s,39s,47s,48s,49s,75o,85o,84o,95o,94o,93o,92o,T5o,T4o,T3o,T2o,J3o,J2o",
"3Ts,4Ts,56s,57s,58s,59s,5Ts,67s,68s,69s,6Ts,78s,79s,89s,67o,68o,69o,6To,78o,79o,7To,89o,8To",
"22,J2s,J3s,J4s,J5s,J6s,Q2s,Q3s,Q4s,Q5s,K2s,J4o,J5o,J6o,J7o,Q2o,Q3o,Q4o,Q5o,Q6o,Q7o,K2o,K3o,K4o",
"6Qs,7Ts,7Js,7Qs,8Ts,8Js,8Qs,9Ts,9Js,9Qs,TJs,T9o,J8o,J9o,JTo,Q8o,Q9o,QTo,QJo",
"33,44,55,K3s,K4s,K5s,K6s,K7s,K8s,A2s,A3s,A4s,A5s,A6s,K5o,K6o,K7o,K8o,K9o,A2o,A3o,A4o,A5o,A6o,A7o,A8o",
"66,77,QTs,QJs,K9s,KTs,KJs,KQs,A7s,A8s,A9s,ATs,AJs,AQs,AKs,KTo,KJo,KQo,A9o,ATo,AJo,AQo,AKo",
"88,99,TT,JJ,QQ,KK,AA"};

int main() {
	bitset<52> bs;
	for (int hole1 = 0; hole1 < 52; hole1++) {
		bs[hole1] = 1;
		for (int hole2 = 0; hole2 < 52; hole2++) {
			if (bs[hole2]) continue;
			bs[hole2] = 1;
			for (int b1 = 0; b1 < 52; b1++) {
				if(bs[b1]) continue;
				for (int b2 = b1 + 1; b2 < 52; b2++) {
					if (bs[b2]) continue;
					for (int b3 = b2 + 1; b3 < 52; b3++) {
						if (bs[b3]) continue;
						for (int b4 = b3 + 1; b4 < 52; b4++) {
							if (bs[b4]) continue;
							for (int b5 = b4 + 1; b5 < 52; b5++) {
								if (bs[b5]) continue;
								long long boardmask = (1LL<<b1)+(1LL<<b2)+(1LL<<b3)+(1LL<<b4)+(1LL<<b5);
								for (int i = 0; i < 8; i++){
                                    /*EquityCalculator eq;
                                    auto callback = [&eq](const EquityCalculator::Results& results) {
                                        eq.stop();
                                    };
									eq.start({deconvert(hole1)+deconvert(hole2), pf_cluster[i]}, boardmask, 0, false, 0.001, callback, 0.001);
									eq.wait();
									auto r = eq.getResults();
									eqsum[i]+=r.equity[0];*/
								}
							}
						}
					}
				}
			}
			//cout << hole1 << ' ' << hole2 << ": ";
			for (int i = 0; i < 8; i++)
				cout << eqsum[i] << ' ';
			cout << endl;
			bs[hole2] = 0;
		}
		bs[hole1] = 0;
	}
	//eq.start({"AK", "QQ"});
    //eq.wait();
    //auto r = eq.getResults();
    //std::cout << r.equity[0] << " " << r.equity[1] << std::endl;
	return 0;
}