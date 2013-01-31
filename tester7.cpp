#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <cassert>
#include <sstream>

using namespace std;

const double EPS = 1e-5;

int main() {
	
	map<int,double> M;

	vector<double> u,v;

	string line;

	while (getline(cin, line)) {
		stringstream ss(line);
		if (line[0] == 'C') {
			string s; double x;
			ss >> s >> x;
			v.push_back(x);
		}
		else {
			double a; int b;
			ss >> a >> b;
			M[b] += a;
		}
	}

	for (map<int,double>::iterator it = M.begin(); it != M.end(); ++it)
		u.push_back(it->second);

	sort(u.begin(), u.end());
	sort(v.begin(), v.end());

	for (int i = 0; i < u.size(); ++i) {
		bool check = (u[i] < v[i] + EPS && v[i] < u[i] + EPS);
		if (!check)
			cout << u[i] << " " << v[i] << endl;
		assert(check);
	}
}


