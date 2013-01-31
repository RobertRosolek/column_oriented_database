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
	assert(s == "FALSE");
	return false;
}

int main() {
	
	vector<int> V0, V1; vector<double> V2; vector<bool> V3;
	vector<int> R0, R1; vector<double> R2; vector<bool> R3;
	string line;

	while (getline(cin, line)) {
		stringstream ss(line);
		int a,b; double c; string d; bool e;
		string s;

		if (line[0] == 'C') 
			switch (line[1]) {
				case '0':
					ss >> s >> a;
					R0.push_back(a);
					break;
				case '1':
					ss >> s >> b;
					R1.push_back(b);
					break;
				case '2':
					ss >> s >> c;
					R2.push_back(c);
					break;
				case '3':
					ss >> s >> d;
					R3.push_back(toBool(d));
					break;
				default:
					assert(false);
			}
		else {
			ss >> a >> b >> c >> e;
			V0.push_back(a);
			V1.push_back(b);
			V2.push_back(c);
			V3.push_back(e);
		}
	}

	assert(V0 == R0 && V1 == R1 && V3 == R3);
	for (size_t i = 0; i < V2.size(); ++i)
		assert(V2[i] < R2[i] + EPS && R2[i] < V2[i] + EPS);

	return 0;
}


