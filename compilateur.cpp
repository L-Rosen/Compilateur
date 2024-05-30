//  A compiler from a very simple Pascal-like structured language LL(k)
//  to 64-bit 80x86 Assembly langage
//  Copyright (C) 2019 Pierre Jourlin
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Build with "make compilateur"


#include <string>
#include <iostream>
#include <cstdlib>
#include <set>

using namespace std;

char current, lookedAhead;                // Current char    
int NLookedAhead=0;
int tag=0;
set<string> DeclaredVariables;

void ReadChar(void){
    if(NLookedAhead>0){
        current=lookedAhead;    // Char has already been read
        NLookedAhead--;
    }
    else
        // Read character and skip spaces until 
        // non space character is read
        while(cin.get(current) && (current==' '||current=='\t'||current=='\n'));
}

void LookAhead(void){
    while(cin.get(lookedAhead) && (lookedAhead==' '||lookedAhead=='\t'||lookedAhead=='\n'));
    NLookedAhead++;
}

void Error(string s){
	cerr<< s << endl;
	exit(-1);
}

// AdditiveOperator := "+" | "-" | "||"
void AdditiveOperator(void){
	if(current=='+'||current=='-')
		ReadChar();
	else
		if(current=='|'){
			ReadChar();
			if(current=='|')
				ReadChar();
			else
				Error("L'operrateur s'écrit comme ceci : ||");	   // Additive operator expected
		}
		else
		Error("Opérateur additif attendu mais "+string(1, current)+" a la place");	   // Additive operator expected
}

// MultiplicativeOperator := "*" | "/" | "%" | "&&"
void MultiplicativeOperator(void){
	if(current=='*'||current=='/'||current=='%'){
		ReadChar();
	}
	else 
		if(current=='&'){
			ReadChar();
			if(current=='&'){
				ReadChar();
			}
			else
				Error("L'operrateur s'écrit comme ceci : &&");	   // Multiplicative operator expected
	}
	else
		Error("Operateur multiplicatif attendu mais "+string(1, current)+" a la place");	   // Multiplicative operator expected
}

// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
void RelationalOperator(void){
	if(current=='='||current=='!'||current=='<'||current=='>'){
		ReadChar();
		if(current=='='){
			ReadChar();
		}
	}
	else
		Error("Operateur de comparaison attendu mais "+string(1, current)+" a la place");	   // Relational operator expected
}

// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
void Digit(void){
	if((current<'0')||(current>'9'))
		Error("Chiffre attendu mais "+string(1, current)+" a la place");		   // Digit expected
	else{
		cout << "\tpush $"<<current<<endl;
		ReadChar();
	}
}

// Letter := "a"|...|"z"
void Letter(void){
	if((current<'a')||(current>'z'))
		Error("Lettre attendue mais "+string(1, current)+" a la place");		   // Letter expected
	else{
		cout << "\tpush $"<<current<<endl;
		ReadChar();
	}
}

//Number := Digit{Digit}
void Number(void){ //La fonction number rend obsolète la fonction digit
	//On déclare une variable pour stocker le nombre en cours de lecture
	unsigned long long number;
	//On lit le premier chiffre, ensuite , tant qu'on lit des chiffres on multuplie le nombre par 10 et on ajoute le chiffre lu
	//Exemple : 123 : 1 ->  1*10 + 2= 12 -> 12*10 + 3 = 123
	if((current<'0')||(current>'9'))
		Error("Chiffre attendu mais "+string(1, current)+" a la place");		   // Digit expected
	else{
		number=current-'0'; // On convertit le caractère en chiffre puis on le stocke dans number
		ReadChar();
		while((current>='0')&&(current<='9')){ // Tant qu'on lit des chiffres on les ajoute au nombre
			number=number*10+(current-'0');
			ReadChar();
		}
		cout << "\tpush $"<<number<<endl;
	}
}

void Expression(void);

//Factor:= Number | Letter | "(" Expression ")" | "!" Factor
void Factor(void){
	
	if (current>='0' && current <='9'){ //Gestion du nombre
		Number();
	}
	
	else if (current>='a' && current <='z'){//Gestion de la lettre
		Letter();
	}

	else if (current=='('){
		ReadChar(); //On passe au caractère suivant
		Expression();
		if (current!=')'){
			Error(") était attendu mais "+string(1, current)+" a la place");
		}
		ReadChar();
	}
	//Reste a implémenter la négation du facteur
}


//Term := Factor {MultiplicativeOperator Factor}
void Term(void){
	char mulop;
	Factor();
	while (current=='*' || current=='/' || current=='%' || current=='&'){
		mulop=current;
		MultiplicativeOperator();
		Factor();
		cout << "\tpop %rbx"<<endl;	// Opérande 2
		cout << "\tpop %rax"<<endl;	// Opérande 1

		if (mulop=='*' || mulop=='&')
		{
			cout << "\tmulq %rbx" <<endl;
			cout << "\tpush %rax" <<endl;
		}

		else if (mulop=='/'){
			//divq % divise rdx:rax par le contenu du registre fournis , le quotient est dans rax et le reste est dans rdx:
			//On met d'abord 0 dans rdx pour que la partie haute soit nule puis on divise et récupere rax
			cout << "\tmovq $0, %rdx" <<endl;
			cout << "\tdivq %rbx"<<endl;
			cout << "\tpush %rax"<<endl;
		}

		else if (mulop=='%'){
			//Idem mais on garde le reste
			cout << "\tmovq $0, %rdx" <<endl;
			cout << "\tdivq %rbx"<<endl;
			cout << "\tpush %rdx"<<endl;
		}

		else {
			Error("Operateur multiplicatif attendu mais "+string(1, current)+" a la place");
		}
	}
}

// SimpleExpression := Term {AdditiveOperator Term}
void SimpleExpression(void){
	char adop;
	Term();
	while(current=='+'||current=='-' ||current=='|'){
		adop=current;		// Save operator in local variable
		AdditiveOperator();
		Term();
		cout << "\tpop %rbx"<<endl;	// get first operand
		cout << "\tpop %rax"<<endl;	// get second operand
		if(adop=='+' || adop=='|')
			cout << "\taddq	%rbx, %rax"<<endl;	// add both operands
		else
			cout << "\tsubq	%rbx, %rax"<<endl;	// substract both operands
		cout << "\tpush %rax"<<endl;			// store result
	}

}

//Expression := SimpleExpression [RelationalOperator SimpleExpression]
void Expression(void){
	char relop;
	SimpleExpression();
	if (current=='='||current=='!'||current=='<'||current=='>'){
		relop=current;
		LookAhead();
		if (current=='<' && lookedAhead=='=') relop='<=';
		else if (current=='>' && lookedAhead=='=') relop='>=';
		RelationalOperator();
		SimpleExpression();
		//On recupere les opérandes et on les compares:

		cout << "\tpop %rbx"<<endl;	// get first operand
		cout << "\tpop %rax"<<endl;	// get second operand
		cout << "\tcmpq %rbx, %rax"<<endl;//Comparaison
		
		switch(relop){
			case '=':
				cout << "\tje Vrai"<<++tag<<endl;
				break;
			
			case '!':
				cout << "\tjne Vrai"<<++tag<<endl;
				break;

			case '<':
				cout << "\tjb Vrai"<<++tag<<endl;
				break;

			case '<=':
				cout << "\tjbe Vrai"<<++tag<<endl;
				break;

			case '>':
				cout << "\tja Vrai"<<++tag<<endl;
				break;

			case '>=':
				cout << "\tjae Vrai"<<++tag<<endl;
				break;
			
			default:
				Error("Operateur de comparaison inconnu");
			
		}

		cout << "\tpush $0"<<endl;
		cout << "\tjmp Suite"<<tag<<endl;
		cout << "Vrai"<<tag<<":\tpush $0xFFFFFFFFFFFFFFFF"<<endl;
		cout << "Suite"<<tag<<":"<<endl;
	}

}

bool IsDeclared(char c){
	return DeclaredVariables.find(string(1,c))!=DeclaredVariables.end();
}


// AssignementStatement := Letter "=" Expression
void AssignementStatement(void){
	char letter;
	if(current<'a'||current>'z') Error("Lettre minuscule attendue mais "+string(1, current)+" a la place");
	letter=current;
	if(!IsDeclared(letter)){
		cerr << "Erreur : Variable '"<<letter<<"' non déclarée"<<endl;
		exit(-1);
	}
	ReadChar();
	if(current!='=') Error("Caractère '=' attendu mais "+string(1, current)+" a la place");
	ReadChar();
	Expression(); //L'expression vas push sa valeur
	cout <<"\tpop "<<letter<<endl; //On la fait correspondre a l'etiquette letter
}

//Statement := AssignementStatement
void Statement(void){
	AssignementStatement();
}

//StatementPart := Statement {";" Statement} "."
void StatementPart(void){
	cout << "\t.text\t\t# The following lines contain the program"<<endl;
	cout << "\t.globl main\t# The main function must be visible from outside"<<endl;
	cout << "main:\t\t\t# The main function body :"<<endl;
	cout << "\tmovq %rsp, %rbp\t# Save the position of the stack's top"<<endl;
	Statement();
	while (current==';'){
		ReadChar();
		Statement();
	}
	if (current!='.'){
		Error("Caractère . attendu mais "+string(1, current)+" a la place");
	}
	ReadChar();
}

// DeclarationPart := "[" Letter {"," Letter} "]"
void DeclarationPart(void){
	if(current!='[') Error("Le caractere [ est attendu mais "+string(1, current)+" a la place");
	cout << "\t.data"<<endl;

	ReadChar();
	if(current<'a'||current>'z') Error("Une lettre minuscule était attendue mais "+string(1, current)+" a la place");
	cout <<current<<":\t.quad 0"<<endl;
	DeclaredVariables.insert(string(1,current));
	ReadChar();

	while(current==','){
		ReadChar();
		if(current<'a'||current>'z') Error("Une lettre minuscule était attendue mais "+string(1, current)+" a la place");
		cout <<current<<":\t.quad 0"<<endl;
		DeclaredVariables.insert(string(1,current));
		ReadChar();
	}
	ReadChar();
}

// Program := [DeclarationPart] StatementPart
void Program(void){
	if(current=='[')
		DeclarationPart();
	StatementPart();
}


int main(void){	// First version : Source code on standard input and assembly code on standard output
	// Header for gcc assembler / linker
	cout << "\t\t\t\t# This code was produced by the CERI Compiler"<<endl;

	ReadChar();
	Program();

	// Trailer for the gcc assembler / linker
	cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top"<<endl;
	cout << "\tret\t\t\t# Return from main function"<<endl;
	if(cin.get(current)){
		cerr <<"Caractères en trop à la fin du programme : ["<<current<<"]";
		Error("."); // unexpected characters at the end of program
	}

}