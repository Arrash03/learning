#include <bits/stdc++.h>

using namespace std;

int main ()
{
    // in stack
	int a[10][10] = {};

	a[1][2] = 12;

	cout << &a[0][0] << " " << &a[1][2] << " " << (&a[0][0] + 12) << std::endl;


    // in heap
	auto c = new int[10][20];

	cout << &c[0][0] << " " << &c[1][2] << " " << (&c[0][0] + 12) << std::endl;
	cout << c[0] << " " << c[1] << endl;

}
