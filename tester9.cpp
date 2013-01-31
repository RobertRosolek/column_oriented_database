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

	vector<double> V;
	map<int,double> M;

	while (getline(cin, line)) {
		stringstream ss(line);
		if (line[0] == 'C') {
			string s; double x;
			ss >> s >> x;
			V.push_back(x); 	
		}
		else {
			int a; double b, c;
			ss >> a >> b >> c;
			if (b > c)
				M[a] += b - c;
		}
	}

	vector<double> U;

	for (map<int,double>::iterator it = M.begin(); it != M.end(); ++it)
		U.push_back(it->first + it->second);

	sort(U.begin(), U.end());
	sort(V.begin(), V.end());

	assert(U.size() == V.size());

	for (int i = 0; i < U.size(); ++i) {
		if (!isEq(V[i], U[i]))
			cout << V[i] << " " << U[i] << endl;
		assert(isEq(V[i], U[i]));
	}

	return 0;
}
