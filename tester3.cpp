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

	vector<double> V, R;
	string line;

	while (getline(cin,line)) {
		stringstream ss(line);
		string s; double x;
		if (line[0] == 'C') {
			ss >> s >> x;
			R.push_back(x);
		}
		else {
			ss >> x;
			V.push_back(log(fabs(log(fabs(x)))));
		}
	}

	assert(V.size() == R.size());

	for (int i = 0; i < V.size(); ++i) {
		if (!isEq(V[i], R[i]))
			cout << V[i] << " " << R[i] << endl;
		assert(isEq(V[i], R[i]));
	}


	return 0;
}


