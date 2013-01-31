#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <cassert>
#include <sstream>

using namespace std;

const double EPS = 1e-5;

bool toBool(const string &s) {
	if (s == "TRUE")
		return true;
	else if (s == "FALSE")
		return false;
	assert(false);
}

int main() {
	
	vector<int> V(2,0);

	vector<bool> R0;
   	vector<int>	R1;

	string line;

	while (getline(cin, line)) {
		stringstream ss(line);
		if (line[0] == 'C') {
			if (line[1] == '0') {
				string s,x;
				ss >> s >> x;
				R0.push_back(toBool(x));
			}
			else {
				string s; int x;
				ss >> s >> x;
				R1.push_back(x);
			}

		}
		else {
			int x;
			ss >> x;
			V[x]++;
		}
	}

	vector<pair<bool,int> > R, U;
	assert(R0.size() == R1.size());

	for (int i = 0; i < R0.size(); ++i)
		R.push_back(make_pair(R0[i], R1[i]));
	sort(R.begin(), R.end());
	
	U.push_back(make_pair(false, V[0]));
	U.push_back(make_pair(true, V[1]));

	assert(U == R);

	return 0;
}
