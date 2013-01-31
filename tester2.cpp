#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <cassert>
#include <sstream>
#include <functional>

using namespace std;

const double EPS = 1e-5;

int main() {

	vector<int> V, R;
	string line;

	while (getline(cin, line)) {
		stringstream ss(line);
		int a,b; 
		string s;

		if (line[0] == 'C') {
			ss >> s >> a;
			R.push_back(a);
		}
		else {
			ss >> a >> b;
			V.push_back(a + b);
		}
	}

	assert(V == R);

	return 0;
}


