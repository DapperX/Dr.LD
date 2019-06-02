#include <iostream>
#include <vector>
#include "../slice.hpp"
using namespace std;

int main()
{
	char a[] = "abcdef";
	slice<char*> s(a+1, 3);
	auto begin = s.begin();
	auto end = s.end();
	// double x = begin++;
	cout << "begin: " << *begin << endl;
	cout << "end: " << *end << endl;
	cout << "middle: " << s[1] << endl;
	cout << "size: " << s.size() << endl;
	for(auto i=begin; i!=end; ++i)
	{
		cout << *i << endl;
	}
	*(s.begin()+2) = 'z';
	for(slice<char*>::value_type k : s)
	{
		cout << k << endl;
	}
	// vector<int>::pointer p;
	// vector<int>::iterator it;
	// vector<int>::const_iterator cit;
	// vector<int>::reference ref;
	return 0;
}