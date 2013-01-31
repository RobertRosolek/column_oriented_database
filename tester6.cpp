#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <cassert>
#include <sstream>
#include <cmath>
#include <functional>

using namespace std;

const double EPS = 1e-3;

bool isEq(double x, double y) {
	return x < y + EPS && y < x + EPS;
}

int main() {

	vector<double> V0, V1;
	vector<double> R0, R1;
	string line;

	while (getline(cin,line)) {
		stringstream ss(line);
		string s; double a; int b, c; bool d;
		if (line[0] == 'C') {
			if (line[1] == '0') {
				ss >> s >> a;
				R0.push_back(a);
			}
			else if (line[1] == '1') {
				ss >> s >> a;
				R1.push_back(a);
			}
			else
				assert(false);
		}
		else {
			ss >> a >> b >> c >> d;
			if (b > c || a > c || d) {
				V0.push_back(log(a));
				V1.push_back(log(b));
			}
		}
	}

	assert(V0.size() == V1.size() && V1.size() == R0.size() && R0.size() == R1.size());

	for (int i = 0; i < V0.size(); ++i) {
		assert(isEq(V0[i], R0[i]));
	   
		if (!isEq(V1[i], R1[i]))
			cout << V1[i] << " " << R1[i] << " " << i << endl;
		assert(isEq(V1[i], R1[i]));
	}
	

	return 0;
}


