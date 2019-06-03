#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <set>
#include <string>
#define NUM_FILE 3
#define NUM_SYMBOL_PER_FILE 5
#define MAX_LEN_STRING 10
#define MAX_NUM_CALL 5
using namespace std;

vector<set<string>> symbol_all;
vector<string> symbol_scatter;

string get_random_string()
{
	string str;
	int n = rand()%MAX_LEN_STRING + 1;
	for(int i=0; i<n; ++i)
	{
		char c = rand()%26 + 'A';
		if(rand()&1) c = c-'A'+'a';
		str += c;
	}
	return str;
}

void gen_symbol()
{
	for(int i=0; i<NUM_FILE; ++i)
	{
		set<string> symbol_this;
		while(symbol_this.size()<NUM_SYMBOL_PER_FILE)
			symbol_this.insert(get_random_string());
		symbol_all.push_back(move(symbol_this));
	}
}

void gen_header(int id)
{
	string code;
	auto& symbol_this = symbol_all[id];
	auto macro = "_" + to_string(id) + "_H_";
	code += "#ifndef " + macro + "\n";
	code += "#define " + macro + "\n";
	for(auto& k : symbol_this)
	{
		code += "void " + k + "();\n";
	}
	code += "#endif //" + macro + "\n";

	printf("header #%d:\n%s\n", id, code.c_str());
	ofstream file("code/" + to_string(id) + ".h");
	file << code;
}

void gen_source(int id)
{
	string code;
	auto& symbol_this = symbol_all[id];
	for(int i=0; i<NUM_FILE; ++i)
		code += "#include \"" + to_string(i) +".h\"\n";
	for(auto& k : symbol_this)
	{
		code += "void " + k + "(){\n";
		int cnt_call = rand()%MAX_NUM_CALL;
		for(int j=0; j<cnt_call; ++j)
		{
			code += symbol_scatter[rand()%symbol_scatter.size()]+"();\n";
		}
		code += "}\n";
	}
	// code += "}\n";
	printf("source #%d:\n%s\n", id, code.c_str());
	ofstream file("code/" + to_string(id) + ".c");
	file << code;
}

void gen_code()
{
	for(int i=0; i<NUM_FILE; ++i)
	{
		gen_header(i);
		gen_source(i);
	}
}

int main()
{
	srand(time(NULL));
	gen_symbol();
	for(auto& k : symbol_all)
	{
		symbol_scatter.insert(symbol_scatter.end(), k.begin(), k.end());
	}
	gen_code();
	return 0;
}