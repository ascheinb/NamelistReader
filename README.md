NamelistReader can be used to read in a set of Fortran namelists from a file. The namelists are stored in a NamelistReader object.

# Basic Functions

## `NamelistReader(const string& filename)`

Reads and parses the input file `filename`.

## `NamelistReader(const string& str_name, ReadFrom read_from)`

Reads and parses an input string `str_name` rather than a file, if `read_from` is `NLReader::ReadFromString`.

## `void use_namelist(const string& namelist)`

Specifies which namelist the subsequent function calls are going to look in. Must be called before `present`, `get`, or `get_required`.

## `bool present(const string& param)`

Checks if the input parameter `param` is present in the last specified namelist.

## `template <typename T> T get(const string& param, const T default_val, int val_ind=0)`

If the input parameter `param` is present in the last specified namelist, returns the value.
If `param` is not present, returns `default_val`.
The third argument is optional. Use it to specify which value you want if there are multiple values in the input file. (The index is 0-indexed.)

## `template <typename T> T get_required(const string& param, int val_ind=0)`

Like `get`, but there is no default value for the parameter. If the parameter is not present in the namelist, the program will exit.

## `bool check_all_used()`

Checks whether every parameter in the input file has been accessed by `get` or `get_required`. Prints a warning listing the parameters that have not been used, if any.

# Advanced Functions

If you wish to manipulate your namelists or create them from scratch, you can use these functions.

## `NamelistReader(Create create)`

If `create` is `NLReader::CreateManually`, starts from scratch and enables modifications so you can construct your namelists from scratch.

## `void enable_modifications()`

Enables modifications via `add_namelist` and `add_to_namelist`.

## `void add_namelist(const string& namelist)`

Adds a new namelist. Modifications must be enabled.

## `void add_to_namelist(const string& param, const string& val)`

Adds a new input parameter to the final namelist. Modifications must be enabled.

# A simple use example

```
#include "NamelistReader.hpp"
  
int main(){
    NLReader::NamelistReader nlr("input_file"); // All file reading is done here

    // Specify the namelist you want to read from
    nlr.use_namelist("first_namelist");

    // Set your variable to the matching string in the namelist
    // Set currently accepts bool, int, double, float, and std::string
    // Provide a default value in case the parameter isn't found in the namelist
    int my_int = nlr.get("my_int", 1);

    // Use get_required if there is not default value. The program will exit if it isn't present.
    float my_float = nlr.get_required("my_float");

    // If you want to go to a different namelist, use use_namelist again:
    nlr.use_namelist("second_namelist");
    std::string my_string = nlr.get("my_string", "default_string");

    string my_other_string = nlr.get("my_other_string", "another_default"); // (Still in "second_namelist")

    nlr.use_namelist("third_namelist");
    double my_double = nlr.get("my_double", 0.0);

    // If the parameter in the namelist has multiple values, you have to read them individually,
    // by specifying the index of the value as a third, optional argument. e.g. if your namelist has:
    // double_list = 3.14d0 2.72d0
    // and you want it to populate a vector, then do the following:
    std::vector<double> double_list(2);

    for (int i = 0; i<double_list.size(); i++){
        double_list[i] = nlr.get<double>("double_list",0.0,i);
    }

    // Lastly, unlike when reading in Fortran, you will not get an error if there are
    // parameters present in the namelist that are not used. You can check for such
    // parameters:
    if (!nlr.check_all_used()) printf("\nExtra parameters in the namelist!\n");
    
}
```
