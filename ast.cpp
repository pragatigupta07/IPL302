#include "ast.hh"
#include "symbtab.hh"
#include <cstdarg>


empty_astnode::empty_astnode() : statement_astnode()
{
	astnode_type = EmptyNode;
}

void empty_astnode::print(string fname, int level)
{
	return;
}

//////////////////////////

seq_astnode::seq_astnode() : statement_astnode()
{

	astnode_type = SeqNode;
}


void seq_astnode::pushback(statement_astnode *child)
{
	children_nodes.push_back(child);
}


void seq_astnode::print(string fname, int level)
{
	for(auto a: children_nodes){
		a->print(fname, 0);
	}
}

///////////////////////////////////

assignS_astnode::assignS_astnode(exp_astnode *l, exp_astnode *r, string tc) : statement_astnode()
{
	typecast = tc;
	left = l;
	right = r;
	id = "Ass";
	astnode_type = AssNode;
}


// using %eax register but its temporary not saving
void assignS_astnode::print(string fname, int level)
{
	SymbTabEntry temp = gst.Entries[fname];
	string lname = left->id;
	SymbTabEntry l_exp = temp.symbtab->Entries[lname];
	if(right->is_calculable){
		cout << "  movl" << "  $" << right->int_val << ", " << l_exp.offset << "(%ebp)"<< endl;
	}
	else{
		right->print(fname, 0);
		cout << "  pop  %eax" << endl;
		cout << "  movl" << "  %eax, " << l_exp.offset << "(%ebp)"<< endl;
	}
}

///////////////////////////////////

return_astnode::return_astnode(exp_astnode *c) : statement_astnode()
{
	child = c;
	id = "Return";
	astnode_type = ReturnNode;
}


//return value should be stored in %eax register
void return_astnode::print(string fname, int level)
{
    int ret_addr = 8; 
	child->print(fname, 0);
	cout << "  pop  %eax" << endl;
    cout << "  movl  %eax, " << ret_addr << "(%ebp)\n";
    if(fname != "main"){
		//postlude
		cout << "  movl  %ebp, %esp\n";
		cout << "  popl	 %ebp\n";
		cout << "  " <<"nop" << endl;
        cout<< "  " << "ret" << endl;
	}
    else{
        cout<< "  " << "leave" << endl;
        cout<< "  " << "ret" << endl;
    }
}

////////////////////////////////////

if_astnode::if_astnode(exp_astnode *l, statement_astnode *m, statement_astnode *r) : statement_astnode()
{
	left = l;
	middle = m;
	right = r;
	id = "If";
	astnode_type = IfNode;
}

void if_astnode::print(string fname, int level)
{
	int a = labelCount;
	labelCount = labelCount + 2;
	left->print(fname, 0);
	cout << "  popl  %eax\n";
	cout << "  cmpl  $0, %eax\n";
	cout << "  je  .L" << a <<endl;
	middle->print(fname, 0);
	cout << "  " << "jmp  .L" << a+1 << endl;
	cout << ".L" << a << ":" << endl;
	right->print(fname, 0);
	cout << ".L" << a+1 << ":" << endl;
	
}
////////////////////////////////////

while_astnode::while_astnode(exp_astnode *l, statement_astnode *r) : statement_astnode()
{
	left = l;
	right = r;
	id = "While";
	astnode_type = WhileNode;
}

void while_astnode::print(string fname, int level)
{
	cout << ".L" << labelCount << ":" << endl;
	int a = labelCount;
	labelCount = labelCount + 2;
	left->print(fname, 0);
	cout << "  popl  %eax\n";
	cout << "  cmpl  $0, %eax\n";
	cout << "  je  .L" << a+1 <<endl;
	right->print(fname, 0);
	cout << "  " << "jmp .L" << a << endl;
	cout << ".L" << a+1 << ":" << endl;
}
/////////////////////////////////

for_astnode::for_astnode(exp_astnode *l, exp_astnode *m1, exp_astnode *m2, statement_astnode *r) : statement_astnode()
{
	left = l;
	middle1 = m1;
	middle2 = m2;
	right = r;
	id = "For";
	astnode_type = ForNode;
}

void for_astnode::print(string fname, int level)
{
	left->print(fname, 0);
	cout << ".L" << labelCount << ":" << endl;
	int a = labelCount;
	labelCount = labelCount + 2;
	middle1->print(fname, 0);
	cout << "  popl  %eax\n";
	cout << "  cmpl  $0, %eax\n";
	cout << "  je  .L" << a+1 <<endl;
	right->print(fname, 0);
	middle2->print(fname, 0);
	cout << "  " << "jmp .L" << a << endl;
	cout << ".L" << a+1 << ":" << endl;
}

//////////////////////////////////

// exp_astnode::exp_astnode() : abstract_astnode()
// {
// }

//////////////////////////////////
string exp_astnode::idname()
{
	return id;
};


op_binary_astnode::op_binary_astnode(string val, exp_astnode *l, exp_astnode *r) : exp_astnode()
{
	id = val;
	left = l;
	right = r;
	astnode_type = OpBinaryNode;
}

void op_binary_astnode::print(string fname, int level)
{
	switch (level)
	{
	case 0:

		right->print(fname, 0);
		left->print(fname, 0);
		if (id == "OR_OP"){
			cout << "  popl  %eax\n";
			cout << "  popl  %edx\n";
			cout << "  or  %eax, %edx\n";
			cout << "  pushl  $0\n";
			cout << "  cmpl  $0, %edx\n";
			cout << "  je  .L" << labelCount << endl;
			cout << "  addl  $1, (%esp)\n";
			cout << ".L" << labelCount++ << ":" << endl;
		}
		else if (id == "AND_OP"){
			cout << "  popl  %eax\n";
			cout << "  popl  %edx\n";
			cout << "  imull  %eax, %edx\n";
			cout << "  cmpl  $0, %edx\n";
			int a = labelCount;
			labelCount = labelCount + 2;
			cout << "  je  .L" << a << endl;
			cout << "  pushl  $1\n";
			cout << "  jmp  .L" << a+1 << endl;
			cout << ".L" << a << ":\n";
			cout << "  pushl  $0\n";
			cout << ".L" << a+1 << ":\n";
		}
		else if (id == "EQ_OP_INT"){
			cout << "  popl  %eax\n";
			cout << "  popl  %edx\n";
			cout << "  pushl  $0\n";
			cout << "  cmpl  %eax, %edx\n";
			cout << "  jne  .L" << labelCount << endl;
			cout << "  addl  $1, (%esp)\n";
			cout << ".L" << labelCount++ << ":" << endl;
		}
		else if (id == "NE_OP_INT"){
			cout << "  popl  %eax\n";
			cout << "  popl  %edx\n";
			cout << "  pushl  $0\n";
			cout << "  cmpl  %eax, %edx\n";
			cout << "  je  .L" << labelCount << endl;
			cout << "  addl  $1, (%esp)\n";
			cout << ".L" << labelCount++ << ":" << endl;
		}
		else if (id == "LT_OP_INT"){
			cout << "  popl  %eax\n";
			cout << "  popl  %edx\n";
			cout << "  pushl  $0\n";
			cout << "  cmpl  %eax, %edx\n";
			cout << "  jle  .L" << labelCount << endl;
			cout << "  addl  $1, (%esp)\n";
			cout << ".L" << labelCount++ << ":" << endl;
		}
		else if (id == "GT_OP_INT"){
			cout << "  popl  %eax\n";
			cout << "  popl  %edx\n";
			cout << "  pushl  $0\n";
			cout << "  cmpl  %eax, %edx\n";
			cout << "  jge  .L" << labelCount << endl;
			cout << "  addl  $1, (%esp)\n";
			cout << ".L" << labelCount++ << ":" << endl;
		}
		else if (id == "LE_OP_INT"){
			cout << "  popl  %eax\n";
			cout << "  popl  %edx\n";
			cout << "  pushl  $0\n";
			cout << "  cmpl  %eax, %edx\n";
			cout << "  jl  .L" << labelCount << endl;
			cout << "  addl  $1, (%esp)\n";
			cout << ".L" << labelCount++ << ":" << endl;
		}
		else if (id == "GE_OP_INT"){
			cout << "  popl  %eax\n";
			cout << "  popl  %edx\n";
			cout << "  pushl  $0\n";
			cout << "  cmpl  %eax, %edx\n";
			cout << "  jg  .L" << labelCount << endl;
			cout << "  addl  $1, (%esp)\n";
			cout << ".L" << labelCount++ << ":" << endl;
		}
		else if (id == "PLUS_INT"){
			cout << "  popl  %eax\n";
            cout << "  popl  %edx\n";
            cout << "  addl  %eax, %edx\n";
            cout << "  pushl  %edx\n";
		}
		else if (id == "MINUS_INT"){
			cout << "  popl  %eax" << endl;
			cout << "  popl  %edx" << endl;
			cout << "  subl  %edx, %eax\n";
			cout << "  pushl  %eax\n";
		}
		else if (id == "MULT_INT"){
			cout << "  popl  %edx" << endl;
			cout << "  popl  %eax" << endl;
			cout << "  imull  %edx, %eax\n";
			cout << "  pushl  %eax\n";
		}
		else if (id == "DIV_INT"){
			cout << "  movl  $0, %edx\n";
			cout << "  popl  %eax" << endl;
			cout << "  popl  %ebx" << endl;
			cout << "  cltd" << endl;
			cout << "  idivl  %ebx" << endl;
			cout << "  pushl  %eax\n";
		}
		break;
	}
	
}

///////////////////////////////////

op_unary_astnode::op_unary_astnode(string val) : exp_astnode()
{
	id = val;
	astnode_type = OpUnaryNode;
}

void op_unary_astnode::print(string fname, int level)
{
	switch (level)
	{
	case 0:
		if(id == "UMINUS"){
			child->print(fname, 0);
			cout << "  popl  %eax" << endl;
			cout << "  negl  %eax" << endl;
			cout << "  pushl  %eax" << endl;
		}
		else if(id == "NOT"){
			child->print(fname, 0);
			cout << "  popl  %eax\n";
			cout << "  pushl  $0\n";
			cout << "  cmpl  $0, %eax\n";
			cout << "  jne  .L" << labelCount << endl;
			cout << "  addl  $1, (%esp)\n";
			cout << ".L" << labelCount++ << ":\n";
		}
		else if(id == "PP"){
			child->print(fname, 0); 	//can't have ++++
			SymbTabEntry temp = gst.Entries[fname];
			SymbTabEntry l_exp = temp.symbtab->Entries[id];
			cout << "  addl  $1, " << l_exp.offset << "(%ebp)" << endl;
		}
		else{	//nedd to other cases corresponding to * and &
			child->print(fname, 0);
		}
		break;
	}
}

op_unary_astnode::op_unary_astnode(string val, exp_astnode *l) : exp_astnode()
{
	id = val;
	child = l;
	astnode_type = OpUnaryNode;
}

string op_unary_astnode::getoperator()
{
	return id;
}
///////////////////////////////////

assignE_astnode::assignE_astnode(exp_astnode *l, exp_astnode *r) : exp_astnode()
{
	left = l;
	right = r;
	astnode_type = AssignNode;
}

void assignE_astnode::print(string fname, int level)
{
	SymbTabEntry temp = gst.Entries[fname];
	string lname = left->id;
	SymbTabEntry l_exp = temp.symbtab->Entries[lname];
	if(right->is_calculable){
		cout << "  movl" << "  $" << right->int_val << ", " << l_exp.offset << "(%ebp)"<< endl;
	}
	else{
		right->print(fname, 0);
		cout << "  pop  %eax" << endl;
		cout << "  movl" << "  %eax, " << l_exp.offset << "(%ebp)"<< endl;
	}
}

///////////////////////////////////

funcall_astnode::funcall_astnode() : exp_astnode()
{
	astnode_type = FunCallNode;
}

funcall_astnode::funcall_astnode(identifier_astnode *child)
{
	funcname = child;
	astnode_type = FunCallNode;
}

void funcall_astnode::setname(string name)
{
	funcname = new identifier_astnode(name);
}

void funcall_astnode::pushback(exp_astnode *subtree)
{
	children.push_back(subtree);
}

void funcall_astnode::print(string fname, int level)
{
	int sz = 0;
    for (int i = children.size() - 1; i >= 0; --i) {
        if (children[i]->astnode_type != EmptyNode && children[i]->astnode_type!=StringConstNode){
            children[i]->print(fname, 0);
            sz+=4;
        }
    }
    cout << "  subl  $4, %esp\n"; //return address
    cout << "  " << "call" << "  " << funcname->id << endl;
    cout << "  movl  (%esp), %ebx\n";
    if(sz!=0){
        cout << "  addl  $" << sz+4 << ", %esp\n"; 
    }
    cout << "  pushl  %ebx\n";
}


proccall_astnode::proccall_astnode (funcall_astnode *fc)
{
	procname = fc->funcname;
	children = fc->children;
}


void proccall_astnode::print(string fname, int level)
{
	if(procname->id=="printf"){
		vector<int> temp = stringLocation[fname];
		int index = temp[0];
		temp.erase(temp.begin());
		stringLocation[fname] = temp;

		int sz = 0;
		for (int i = children.size() - 1; i >= 0; --i) {
			if (children[i]->astnode_type != EmptyNode && children[i]->astnode_type!=StringConstNode){
				children[i]->print(fname, 0);
				sz+=4;
			}
		}
		cout << "  " << "pushl" << "  " << "$.LC" << index << endl;	
		cout << "  " << "call" << "  " << "printf" << endl;
		if(sz!=0){
			cout << "  addl  $" << sz+4 << ", %esp\n"; 
		}
	}
    else{
		int sz = 0;
        for (int i = children.size() - 1; i >= 0; --i) {
            if (children[i]->astnode_type != EmptyNode && children[i]->astnode_type!=StringConstNode){
                children[i]->print(fname, 0);
                sz+=4;
            }
        }
        cout << "  " << "call" << "  " << procname->id << endl;
        if(sz!=0){
            cout << "  addl  $" << sz << ", %esp\n"; 
        }
    }
}
/////////////////////////////////////

intconst_astnode::intconst_astnode(int val) : exp_astnode()
{
	value = val;
	astnode_type = IntConstNode;
}

void intconst_astnode::print(string fname, int level)
{
	cout << "  pushl  $" << value << endl;
}
/////////////////////////////////////
floatconst_astnode::floatconst_astnode(float val) : exp_astnode()
{
	value = val;
	astnode_type = FloatConstNode;
}

void floatconst_astnode::print(string fname, int level)
{
	return;
}
///////////////////////////////////
stringconst_astnode::stringconst_astnode(string val) : exp_astnode()
{
	value = val;
	astnode_type = StringConstNode;
}

void stringconst_astnode::print(string fname, int level)
{
	return;
}


/////////////////////////////////

identifier_astnode::identifier_astnode(string val) : ref_astnode()
{
	id = val;
	astnode_type = IdentifierNode;
}

void identifier_astnode::print(string fname, int level)
{
	SymbTabEntry temp = gst.Entries[fname];
	SymbTabEntry l_exp = temp.symbtab->Entries[id];
	cout << "  pushl  " << l_exp.offset << "(%ebp)" << endl;
}

////////////////////////////////

arrayref_astnode::arrayref_astnode(exp_astnode *l, exp_astnode *r) : ref_astnode() // again, changed from ref to exp
{
	left = l;
	right = r;
	id = "ArrayRef";
	astnode_type = ArrayRefNode;
}

void arrayref_astnode::print(string fname, int level)
{
	return;
	// printAst("arrayref", "aa", "array", left, "index", right);
}

////////////////////////////////

deref_astnode::deref_astnode(ref_astnode *c) : ref_astnode()
{
	child = c;
	id = "Deref";
	astnode_type = DerefNode;
}

void deref_astnode::print(string fname, int level)
{
	return;
	// printAst("", "a", "deref", child);
}

/////////////////////////////////

member_astnode::member_astnode(exp_astnode *l, identifier_astnode *r) // change from ref to exp(1st arg)
{
	left = l;
	right = r;
	astnode_type = MemberNode;
}

void member_astnode::print(string fname, int level)
{
	return;
	// printAst("member", "aa", "struct", left, "field", right);
}

/////////////////////////////////

arrow_astnode::arrow_astnode(exp_astnode *l, identifier_astnode *r)
{
	left = l;
	right = r;
	astnode_type = ArrowNode;
}

void arrow_astnode::print(string fname, int level)
{
	return;
	//printAst("arrow", "aa", "pointer", left, "field", right);
}


void printblanks(int blanks)
{
	for (int i = 0; i < blanks; i++)
		cout << " ";
}


char *stringTocharstar(string str)
{
	char *charstar = const_cast<char *>(str.c_str());
	return charstar;
}