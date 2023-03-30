#include <fstream>
#include <string>
#include <vector>

// A register represents a single entry in a table.
class Register
{
};

// The Field class and all its associated subclasses permit runtime polymorphism for register types
class Field 
{
    // must keep a reference to itself in secondary storage
    // might possibly need to keep a reference to its parent Register.
};

class IntegerField
{
    int value;
};

class FloatField
{
    double value;
};

class StringField
{
    std::string value;
};


// superclass for all tables that exist. 
// This should be the smallest unit visible by the user.
class Table
{
    // must store register format
    // must be queriable
};

// this is a table with asociated storage
class FileTable : public Table
{
    // must support CRUD

    // must contain indexes and metadata
};

class MaterializedTable : public Table 
{
    // this is a table living entirelly within RAM. It is the result of a query. 
    // Modification operations are not supported 

    // In the future it might use a tempfile

    // must be queriable and non-modifiable
};

// Use cases:

// insert a specific register

// retrieve registers complying with a specific predicate efficiently (B+)

// perform efficient join operations between tables and return a new table (Hash)

// modify registers complying with a specific predicate in a way that depends on that specific registers information

// delete a register