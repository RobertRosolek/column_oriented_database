#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <cassert>
#include <sstream>
#include <cmath>
#include <functional>

using namespace std;

const double EPS = 1e-5;

bool isEq(double x, double y) {
	return x < y + EPS && y < x + EPS;
}

int main() {

	vector<int> V, R;
	string line;

	while (getline(cin,line)) {
		stringstream ss(line);
		string s; int a; double b;
		if (line[0] == 'C') {
			ss >> s >> a;
			R.push_back(a);
		}
		else {
			ss >> a >> b;
			if (b > 0.0)
				V.push_back(a);
		}
	}

	assert(V == R);

	return 0;
}


