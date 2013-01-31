#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <cassert>
#include <sstream>

using namespace std;

const double EPS = 1e-4;

bool toBool(const string &s) {
	if (s == "TRUE")
		return true;
	else if (s == "FALSE")
		return false;
	assert(false);
}

bool isEq(double a, double b) {
	return a < b + EPS && b < a + EPS;
}

int main() {
	
	string line;

	vector<bool> R0; vector<double> R1;
	map<bool,double> S, C;

	while (getline(cin, line)) {
		stringstream ss(line);
		if (line[0] == 'C') {
			if (line[1] == '0') {
				string s, x;
				ss >> s >> x;
				R0.push_back(toBool(x));
			}
			else if (line[1] == '1') {
				string s; double x;
				ss >> s >> x;
				R1.push_back(x);
			}	
			else
				assert(false);
		}
		else {
			bool a; double b;
			ss >> a >> b;
			S[a] += b;
			C[a]++;
		}
	}

	vector<pair<bool,double> >U, V;

	assert(R0.size() == R1.size());

	for (int i = 0; i < R0.size(); ++i)
		V.push_back(make_pair(R0[i], R1[i]));

	assert(S.size() == 2);

	for (map<bool,double>::iterator it = S.begin(), jt = C.begin(); it != S.end(); ++it, ++jt)
		if  (it->second == jt->second)
			U.push_back(make_pair(it->first,1.0));

	sort(U.begin(), U.end());
	sort(V.begin(), V.end());

	assert(U.size() == V.size());

	for (int i = 0; i < U.size(); ++i) {
		assert(isEq(V[i].second, U[i].second) && V[i].first == U[i].first);
	}

	return 0;
}
