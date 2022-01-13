NamelistReader is a C++ header library that can be used to read and parse an input file containing a set of Fortran namelists. The namelists are stored in a NamelistReader object.

## Basic Functions

### `NamelistReader(const string& filename)`

Constructor that reads and parses the input file `filename`.

### `NamelistReader(const string& str_name, NLReader::ReadFromString)`

Constructor that reads and parses an input string `str_name` rather than a file.

### `void use_namelist(const string& namelist)`

Specifies which namelist the subsequent function calls are going to look in. Must be called before `present`, `get`, or `get_required`.

### `bool present(const string& param)`

Checks if the input parameter `param` is present in the last specified namelist.

### `template <typename T> T get(const string& param, const T default_val, int val_ind=0)`

If the input parameter `param` is present in the last specified namelist, returns the value.
If `param` is not present, returns `default_val`.
The third argument is optional. Use it to specify which value you want if there are multiple values in the input file. (The index is 0-indexed.)

`T` can be `bool`, `int`, `double`, `float`, or `std::string`.

### `template <typename T> T get_required(const string& param, int val_ind=0)`

Like `get`, but there is no default value for the parameter. If the parameter is not present in the namelist, the program will exit.

### `bool check_all_used()`

Checks whether every parameter in the input file has been accessed by `get` or `get_required`. Prints a warning listing the parameters that have not been used, if any.

## Advanced Functions

If you wish to manipulate your namelists or create them from scratch, you can use these functions.

### `NamelistReader(NLReader::CreateManually)`

Constructor that starts from scratch and enables modifications so you can construct your namelists from scratch.

### `void enable_modifications()`

Enables modifications via `add_namelist` and `add_to_namelist`.

### `void add_namelist(const string& namelist)`

Adds a new namelist. Modifications must be enabled.

### `void add_to_namelist(const string& param, const string& val)`

Adds a new input parameter to the final namelist. Modifications must be enabled.

## Example

Imagine you want to read this Fortran input file:

```
&first_namelist
my_int=10
my_float=1.0
my_unused_var=3
/

&second_namelist
my_string='hello'
my_other_string='world'
/

&third_namelist
double_list=3.14d0 2.72d0
/
```

Here is an example of a program that will read in and parse this input file:


```
#include "NamelistReader.hpp"
  
int main(){
    NLReader::NamelistReader nlr("input_file"); // All file reading is done here

    // Specify the namelist you want to read from
    nlr.use_namelist("first_namelist");

    // Provide a default value in case the parameter isn't found in the namelist
    int my_int = nlr.get<int>("my_int", 1);

    // Use get_required if you don't want a default value. The program will exit if it isn't present.
    float my_float = nlr.get_required<float>("my_float");

    // If you want to go to a different namelist, use use_namelist again:
    nlr.use_namelist("second_namelist");

    std::string my_string = nlr.get<std::string>("my_string", "default_string");
    std::string my_other_string = nlr.get<std::string>("my_other_string", "another_default");

    nlr.use_namelist("third_namelist");
    // my_double isn't present, so you'll get the default value 0.0
    double my_double = nlr.get<double>("my_double", 0.0);

    // The parameter "double_list" has two values. We can specify the index with a third, optional argument,
    // and populate a vector like so:
    std::vector<double> double_list(2);
    for (int i = 0; i<double_list.size(); i++){
        double_list[i] = nlr.get<double>("double_list",0.0,i);
    }

    // Lastly, unlike when reading in Fortran, you will not get an error if there are
    // parameters present in the namelist that are not used. You can check for such
    // parameters and exit or do whatever you want.
    // This will print a warning pointing out that my_unused_var is not used.
    if (!nlr.check_all_used()){
        printf("\nExiting because extra parameters were found in the namelist!\n");
        exit(1);
    }
}
```
