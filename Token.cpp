#include "Token.h"
using namespace std;



void Token::print_token(const shared_ptr<Token>& token) {//token输出至控制台
    if (!token) {
        cout << "[null-token]";
        return;
    }

    cout << "[Token place=" << token->inplace
        << ", wait=" << token->waiting_time;

    cout << ", Token Color=" << token->token_attribute->color_to_string(token->token_attribute->color);

    if (token->token_attribute->workpiece) {
        cout << ", wp_id=" << token->token_attribute->workpiece->numid
            << ", wp_name=" << token->token_attribute->workpiece->ID;
    }

    else {
        cout << ", wp=null";
    }

    cout << "]";
}

void Token::print_token(const std::shared_ptr<Token>& token, std::ostream& os) {//token输出至txt
    if (!token) {
        os << "[null-token]";  // 把cout改成os
        return;
    }

    os << "[Token place=" << token->inplace
        << ", wait=" << token->waiting_time;  // 替换cout为os

    os << ", Token Color=" << token->token_attribute->color_to_string(token->token_attribute->color);  // 替换cout为os

    if (token->token_attribute->workpiece) {
        os << ", wp_id=" << token->token_attribute->workpiece->numid
            << ", wp_name=" << token->token_attribute->workpiece->ID;  // 替换cout为os
    }
    else {
        os << ", wp=null";  // 替换cout为os
    }

    os << "]";  // 替换cout为os

};