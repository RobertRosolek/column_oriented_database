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

	vector<int> R0, R1;
	map<pair<int,int>, double> M1;

	while (getline(cin, line)) {
		stringstream ss(line);
		if (line[0] == 'C') {
			if (line[1] == '0') {
				string s; int x;
				ss >> s >> x;
				R0.push_back(x);
			}
			else if (line[1] == '1') {
				string s; int x;
				ss >> s >> x;
				R1.push_back(x);
			}	
			else
				assert(false);
		}
		else {
			int a, b; double c;
			ss >> a >> b >> c;
			if (b > 0)
				M1[make_pair(a,b)] += c;
		}
	}

	vector<pair<int,int> > U,V;

	assert(R0.size() == R1.size());
	for (int i = 0; i < R0.size(); ++i)
		U.push_back(make_pair(R0[i], R1[i]));

	map<int,int> M;

	for (map<pair<int,int>, double>::iterator it = M1.begin(); it != M1.end(); ++it) {
		if (it->second > 20)
			M[it->first.first]++;
	}

	for (map<int,int>::iterator it = M.begin(); it != M.end(); ++it)
		if (it->second < 5)
			V.push_back(*it);

	sort(U.begin(), U.end());
	sort(V.begin(), V.end());
	assert(U.size() == V.size());

	assert(U == V);


	return 0;
}
