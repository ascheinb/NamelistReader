#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

namespace NLReader{

using namespace std;

enum Expect {
    Ampersand=0,
    NLName,
    ParaNameOrEnd,
    Equals,
    Value,
    ValueOrEOL
};

// trim from left
inline string& ltrim(string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from right
inline string& rtrim(string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from left & right
inline string& trim(string& s, const char* t = " \t\n\r\f\v")
{
    return ltrim(rtrim(s, t), t);
}

struct Param{
    string name;
    bool used;
    vector<string> values;

    Param(string name)
        : name(name), used(false)
    {}
};

struct NameList{
    string name;
    vector<Param> params;

    NameList(string name)
        : name(name)
    {}

    void add_param(string word)
    {
        params.push_back(Param(word));
    }

    void add_value(string word)
    {
        params[params.size()-1].values.push_back(word);
    }
};

string get_first_word(string& str, int start){
    for (string::size_type i = start; i < str.size(); i++){
        if ((str[i]>=48 && str[i]<=57) || (str[i]>=65 && str[i]<=90) ||
            (str[i]>=97 && str[i]<=122) || str[i]==95){
            // still in word (0-9, A-Z, a-z, _)
        } else {
            // Not in word
            return str.substr(start, i-start);
        }
    }
    return str.substr(start,str.size()-start);
}

string get_first_value(string& str, int start){
    bool in_squote=false;
    bool in_dquote=false;
    for (string::size_type i = start; i < str.size(); i++){
        if (in_dquote==true || in_squote==true){
            if (str[i]==34) in_dquote=false;
            if (str[i]==39) in_squote=false;
        } else {
            if (str[i]==34) in_dquote=true;
            if (str[i]==39) in_squote=true;
            if (str[i]=='!' || str[i]==' ' || str[i]=='\t'|| str[i]=='\r'|| str[i]=='\f'|| str[i]=='\v'){
                // Not in word
                return str.substr(start, i-start);
            }
        }
    }
    return str.substr(start,str.size()-start);
}

template<typename T>
T string_to_param(string& param);

template<>
bool string_to_param<bool>(string& param){
    // Make lower case
    for (int i = 0; i<param.size(); i++)
        param[i]=tolower(param[i]);

    if (param==".false."){
        return false;
    } else if  (param==".true."){
        return true;
    } else {
        printf("Error, couldn't parse %s", param.c_str());
        return false;
    }
}

void change_D_to_E(string& param){
    for (int i = 0; i<param.size(); i++)
        if (param[i]=='D' || param[i]=='d') param[i]='e';
}

template<>
int string_to_param<int>(string& param){
    change_D_to_E(param);
    return stoi(param);
}

template<>
float string_to_param<float>(string& param){
    change_D_to_E(param);
    return stof(param);
}

template<>
double string_to_param<double>(string& param){
    change_D_to_E(param);
    return stod(param);
}

template<>
string string_to_param<string>(string& param){
    // Remove the quotes
    return param.substr(1,param.size()-2);
}

class NamelistReader{
    vector<NameList> namelists;
    int namelist_index;
    bool required;
    bool use_all;

    public:

    NamelistReader(const string& filename)
        : namelist_index(-1), required(false), use_all(false)
    {
        load(filename);
    }

    void begin_required(){
        required=true;
    }

    void begin_optional(){
        required=false;
    }

    bool check_all_used(){
        int n_unused=0;
        for (vector<NameList>::iterator nl = namelists.begin(); nl != namelists.end(); ++nl) {
            for (vector<Param>::iterator param = nl->params.begin(); param != nl->params.end(); ++param ) {
                if (param->used==false){
                    printf("Warning: '%s' was present in '%s' but was not used.\n",param->name.c_str(), nl->name.c_str());
                    n_unused++;
                }
            }
        }
        return (n_unused==0);
    }

    void load(const string& filename){
        bool verbose=false;
        fstream newfile;
        newfile.open(filename,ios::in); //open a file to perform read operation using file object
        if (newfile.is_open()){   //checking whether the file is open
            string line;
            Expect expect = Ampersand;
            int i_line=0;
            while(getline(newfile, line)){ //read data from file object and put it into string.
                i_line++;
                trim(line); // Remove leading and trailing whitespace
                for (string::size_type i = 0; i < line.size(); i++){
                    if (line[i]=='!'){
                        // Comment has begun
                        break;
                    }
                    if (expect==Ampersand){
                        if (line[i]=='&'){
                            expect=NLName;
                        }
                    } else if (expect==NLName){
                        string word = get_first_word(line,i);
                        if (word.size()>0){ // Found namelist name
                            expect = ParaNameOrEnd;
                            namelists.push_back(NameList(word));
                            if (verbose) cout << "Found namelist: " << word << "\n";
                            break;
                        }
                    } else if(expect==ParaNameOrEnd){
                        if (line[i]=='/'){ // End of namelist
                            expect=Ampersand;
                            break;
                        }

                        string word = get_first_word(line,i);
                        if (word.size()>0){
                            i+=word.size()-1;
                            expect = Equals;
                            namelists[namelists.size()-1].add_param(word);
                            if (verbose) cout << "  Found parameter: " << word << "\n";
                        }
                    } else if(expect==Equals){
                        if (line[i]=='='){
                            expect=Value;
                        }
                    } else if(expect==Value || expect == ValueOrEOL){
                        string word = get_first_value(line,i);
                        if (word.size()>0){
                            i+=word.size()-1;
                            expect = ValueOrEOL;
                            namelists[namelists.size()-1].add_value(word);
                            if (verbose) cout << "    Found Value: " << word << "\n";
                        }
                    }
                }
                // Got to end of line
                if (expect==NLName){
                    printf("\nParse failure reading %s at Line %d. Failed to find name of namelist.\n", filename.c_str(), i_line);
                    return;
                } else if(expect==Equals){
                    printf("\nParse failure reading %s at Line %d. Expected parameter assignment.\n", filename.c_str(), i_line);
                    return;
                } else if(expect==Value){
                    printf("\nParse failure reading %s at Line %d. Couldn't parse value assignment.\n", filename.c_str(), i_line);
                    return;
                }
                // Hit EOL, now expecting next parameter
                if (expect==ValueOrEOL) expect = ParaNameOrEnd;
            }
            newfile.close(); //close the file object.
        }
    }

    void use_namelist(const string& namelist){
        // Locate the requested namelist
        vector<NameList>::iterator it = namelists.begin();
        while (it!=namelists.end()) {
            if ((*it).name==namelist) break;
            ++it;
        }
        if (it != namelists.end())
            namelist_index = distance(namelists.begin(), it);
        else
            printf("\nNamelist '%s' not found in the file!",namelist.c_str());
    }

    template <typename T>
    T get(const string& param, const T default_val, int val_ind=0){
        if (namelist_index==-1){
            printf("\nNeed to choose which namelist to use with use_namelist(const std::string&)!\n");
        }

        // Locate the requested parameter
        int param_index = -1;
        vector<Param>::iterator it = namelists[namelist_index].params.begin();
        while (it!=namelists[namelist_index].params.end()) {
            if ((*it).name==param) break;
            ++it;
        }

        if (it != namelists[namelist_index].params.end())
            param_index = distance(namelists[namelist_index].params.begin(), it);
        else{
            if (required){
                printf("\nParameter '%s' not found in namelist '%s'!",param.c_str(),namelists[namelist_index].name.c_str());
                exit(1);
            }
            return default_val;
        }

        namelists[namelist_index].params[param_index].used=true;

        if (val_ind>=0 && val_ind<namelists[namelist_index].params[param_index].values.size()){
            string str_var = namelists[namelist_index].params[param_index].values[val_ind];
            return string_to_param<T>(str_var);
        } else {
            printf("\nParameter '%s' in namelist '%s' didn't have enough values!",param.c_str(),namelists[namelist_index].name.c_str());
            exit(1);
            return default_val;
        }
    }
};

}
