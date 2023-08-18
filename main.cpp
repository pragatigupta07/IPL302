
#include "scanner.hh"
#include "parser.tab.hh"
#include <fstream>
#include <iomanip>

using namespace std;

int tabL = 4;
SymbTab gst, gstfun; 
int labelCount = 0;
string filename;
extern std::map<string, vector<string>> funcPrintStr;
std::map<string, vector<int>> stringLocation;

extern std::map<string,abstract_astnode*> ast;
std::map<std::string, datatype> predefined {
            {"printf", createtype(VOID_TYPE)},
            {"scanf", createtype(VOID_TYPE)},
            {"mod", createtype(INT_TYPE)}
        };
int main(int argc, char **argv)
{
	fstream in_file, out_file;
	

	in_file.open(argv[1], ios::in);

	IPL::Scanner scanner(in_file);

	IPL::Parser parser(scanner);

#ifdef YYDEBUG
	parser.set_debug_level(1);
#endif
parser.parse();
// create gstfun with function entries only

for (const auto &entry : gst.Entries)
{
	if (entry.second.varfun == "fun")
	gstfun.Entries.insert({entry.first, entry.second});
}

cout << " .section" << " .rodata" << endl;

int printfCount = 0;

for (auto it = gstfun.Entries.begin(); it != gstfun.Entries.end(); ++it)
{
	// add string label
	for(auto a : funcPrintStr[it->first]){
		cout << ".LC" << printfCount << ":" << endl;
		cout << "  " << ".string" << "  " << a  << endl;
		stringLocation[it->first].push_back(printfCount);
		printfCount++;
	}

	//prelude
	cout << "  " << ".text" << endl;
	cout << "  " << ".global" << "  " << it->first << endl; 

	cout << it->first << ":" << endl;
	cout << "  " << "pushl" << "  " << "%ebp" << endl;
	cout << "  " << "movl" << "  " << "%esp, %ebp" << endl;
	SymbTabEntry temp = gst.Entries[it->first];
	int tot_size = 0;
    for(auto x : temp.symbtab->Entries){
        if(x.second.scope == "local"){
            tot_size+=x.second.size;
        }
    }
	cout << "  " << "subl" << "  " << "$" << tot_size << ", %esp"<< endl;

	ast[it->first]->print(it->first, 0);
	// cout << "  " << "addl" << "  " << "$" << tot_size << ", %esp"<< endl;
	
	
	//postlude
}

}
