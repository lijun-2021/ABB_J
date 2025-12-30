#pragma once
#include "Process.h"
#include"Place.h"
#include"Transition.h"
#include<iostream>
#include<string>
#include<memory>
#include<map>
#include<fstream>

using namespace std;
class TokenAttribute;
class Node;
class Token {
public:
	string inplace;//所在库所
	int waiting_time = 0;//Token在该库所内的已等待时间

	shared_ptr<TokenAttribute> token_attribute;//token属性

	Token() = default;
	Token(const string& inplace, shared_ptr<TokenAttribute>token_attribute, int waiting_time = 0) : inplace(inplace), token_attribute(token_attribute) {};
	static void print_token(const shared_ptr<Token>& token);
	static void print_token(const shared_ptr<Token>& token, std::ostream& os);

};


