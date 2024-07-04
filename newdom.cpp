#include<iostream>
#include<vector>
#include<map>
#include<stdexcept>
#include<cassert>
#include<functional>
#include<cctype>
using namespace std;

using AttrMap = map<string,string>;


struct ElementData{
    string tag_name;
    AttrMap attributes;
};
struct nodeData{
    string text;
    ElementData element;
};
struct node{
    nodeData content;
    vector<node> children;
    node(string text){
        content.text = text;
    }
    node(string name,AttrMap attributes,vector<node> children){
        this->children = children;
        this-> content.element.attributes = attributes;
        this->content.element.tag_name = name;
    }
};
class parser{
    private:
        string input;
        unsigned int pos;
    public:
        parser(string str){
            this -> input = str;
            pos = 0;
        }
        char next_char(){
            return input[pos];
        }
        bool eof(){
            return pos>=input.length();
        }
        bool starts_with(string s){
            try{
                if(s.length()>(input.length()-pos-1)){
                    throw "length is input exceeded";
                }
                string tocomp = input.substr(pos,s.length());
                return tocomp == s;
            }
            catch(string s){
                cout<<"error - "<<s;
            }
            return false;
        }

        char consume_char(){
            if(pos>=input.length()){
                throw "position is out of bounds";
            }
            char curChar = input[pos];
            pos++;

            // Handle multi-byte characters (UTF-8)
            if ((curChar & 0x80) != 0) { // Check if the character is not ASCII
                while (pos < input.size() && (input[pos] & 0xC0) == 0x80) {
                    pos++;
                }
            }
            return curChar;
        }
        string  consume_while(const function<bool(char)> test){
            string ans = "";
            while((!this->eof())&&test(this->next_char())){
                ans.push_back(this->consume_char());
            }
            return ans;
        }
        void consume_whitespace(){
            auto lambda = [](char c) -> bool{return c==' '||c=='\n';};
            consume_while(lambda);
        }

        string parse_tag_name(){
            auto isAlphanumeric = [](char c) -> bool{
                return isalpha(static_cast<unsigned char>(c)) || isdigit(static_cast<unsigned char>(c));
            };
            string temp =  consume_while(isAlphanumeric);
            return temp;
        }
        node parse_text(){
            auto lamb = [](char c) -> bool{return c!='<';};
            node temp(consume_while(lamb));
            return temp;
        }
        string parse_attr_value(){
            char open_char = consume_char();

            assert(open_char=='"'||open_char == '\'');
            auto lamb = [open_char](char c){return c!=open_char;};
            string value = consume_while(lamb);
            assert(consume_char()==open_char);

            return value;
        }
        pair<string,string> parse_attr(){
            consume_whitespace();
            string name = parse_tag_name();
            consume_whitespace();
            assert(consume_char()=='=');//error is here
            consume_whitespace();
            string value = parse_attr_value();
            consume_whitespace();
            return make_pair(name,value);
        }
        AttrMap parse_attributes(){
            AttrMap attributes;
            while(true){
                consume_whitespace();
                if(next_char() == '>') break;
                pair<string,string> temp = parse_attr();
                attributes.insert(temp);
            }
            return attributes;
        }
        node parse_node(){
            if(next_char()=='<'){
                return parse_element();
            }else{
                return parse_text();
            }
        }
        vector<node> parse_nodes(){
            vector<node> children;
            while(true){
                consume_whitespace();
                if(eof()||starts_with("</")){
                    break;
                }
                children.push_back(parse_node());
            }
            return children;
        }
        node parse_element(){
            try{
                assert(consume_char()=='<'&&"opening character < not in place");
                string tagName = parse_tag_name();
                AttrMap attributes = parse_attributes();
                assert(consume_char()=='>'&&"closing character > not in place");

                vector<node> children = parse_nodes();

                assert(consume_char()=='<'&&"wrong opening char");
                assert(consume_char()=='/'&&"/ is not included or remove whitespace b/w < and /");
                assert(parse_tag_name()==tagName);
                assert(consume_char()=='>');

                return node(tagName,attributes,children);

            }catch(const exception& e){
                cerr<<e.what()<<endl;
            }
            node zero("something went wrong");
            return zero;
        }
};
node parse(string str){
    parser dom(str);
    vector<node> nodes = dom.parse_nodes();
    if(nodes.size()==1){
        return nodes[0];
    }else{
        AttrMap dummy;
        node root("html",dummy,nodes);
        return root;
    }
}
int main(){
    string str = "<html><body>Hello, world!</body></html>";
    node test = parse(str);
}