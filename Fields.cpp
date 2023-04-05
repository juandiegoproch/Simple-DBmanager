#pragma once

#include <string>
#include <vector>

// Enums and predeclarations
enum FieldType  {invalid_ft, integer_ft, floating_ft, varchar_ft};

class Table;
class MaterializedTable;
class FileTable;

//
class Field 
{
protected:

private:
    //int parentPos; // parent register on a filetable

protected:
    Field() = default; // non-instantiable!

public:
    virtual FieldType getType() = 0;

    virtual void SetValue(void* data)  = 0;
    virtual void* GetValue() = 0;
    virtual std::string repr_ASCII() = 0;

};

class IntegerField : public Field
{
    int value;
public:

    FieldType getType() override
    {
        return integer_ft;
    }

    void SetValue(void* data)
    {
        value = *((int*)data);
    }

    void* GetValue()
    {
        return &value;
    }

    std::string repr_ASCII() override
    {
        return std::to_string(value);
    }
};

class FloatField : public Field
{
    double value;

public:
    FieldType getType() override
    {
        return floating_ft;
    }

    void SetValue(void* data)
    {
        value = *((double*)data);
    }

    void* GetValue()
    {
        return &value;
    }

    std::string repr_ASCII() override
    {
        return std::to_string(value);
    }
};

class VarcharField : public Field
{
    int size;
    char* value = nullptr;

public:
    FieldType getType() override
    {
        return varchar_ft;
    }

    VarcharField()
    {
        size = 0;
        value = nullptr;
    }

    VarcharField(const VarcharField& other)
    {
        // copy stuff over
        size = other.size;
        value = new char[other.size];

        for (int i = 0; i<other.size; i++)
        {
            value[i] = other.value[i];
        }
    }

    ~VarcharField()
    {
        delete value;
    }

    VarcharField& operator=(VarcharField& other)
    {
        // free my resources
        delete value;

        // copy stuff over
        size = other.size;
        value = new char[other.size];

        for (int i = 0; i<other.size; i++)
        {
            value[i] = other.value[i];
        }

        return *this;
    }

    void SetValue(void* data) override // copies over from C string.
    {
        char* size_meter = (char*)data;

        while(*size_meter != '\0') size_meter++;
        size = size_meter - (char*)data;

        delete value;

        value = new char[size];
        memcpy(value,data,size);
    }

    void* GetValue() override
    {
        return value;
    }

    std::string repr_ASCII() override
    {
        return std::string((const char* const)value);
    }

};

struct TableRowTemplate
{
    std::vector<FieldType> fieldTypes;
    std::vector<std::string> field_names;
    int num_fields = 0;

    TableRowTemplate& field(FieldType f,std::string n)
    {
        fieldTypes.push_back(f);
        field_names.push_back(n);
        num_fields++;
        return *this;
    }
};

// register whole.

class TableRow
{
    public:
    friend MaterializedTable;

    private:
    int position;
    Field** fields;
    unsigned int field_number;
    // must have reference to its parent.

    public:
            // "type" of this template           
    TableRow(TableRowTemplate& rowtemp)
    {
        field_number = rowtemp.num_fields;
        fields = new Field*[field_number];

        for (int i = 0; i<field_number; i++)
        {
            if (rowtemp.fieldTypes[i] == varchar_ft)
            {
                fields[i] = new VarcharField;
            }
            else if (rowtemp.fieldTypes[i] == integer_ft)
            {
                fields[i] = new IntegerField;
            }
            else if (rowtemp.fieldTypes[i] == floating_ft)
            {
                fields[i] = new FloatField;
            }
            else
            {
                // error condition!
            }
        }
    }

    TableRow(const TableRow& other)
    {
        position = other.position;//? mabe?
        field_number = other.field_number;

        // deep copying
        fields = new Field*[field_number];

        for (int i = 0; i<field_number;i++)
        {
            switch (other.fields[i]->getType())
            {
            case varchar_ft:
                fields[i] = new VarcharField(*(VarcharField*)other.fields[i]);
                break;
            case integer_ft:
                fields[i] = new IntegerField(*(IntegerField*)other.fields[i]);
                break;
            case floating_ft:
                fields[i] = new FloatField(*(FloatField*)other.fields[i]);
                break;
            default:
                // error situation!
                break;
            }
        }
    }
    
    FieldType GetFieldType(int index)
    {
        return fields[index]->getType();
    }

    void SetPosition(int num)
    {
        position = num;
    }

    Field* GetField(int index)
    {
        return fields[index];
    }

    ~TableRow()
    {
        for (int i = 0; i<field_number; i++)
            delete fields[i];
        
        delete fields;
    }

    std::string repr_ASCII()
    {
        std::string rep;
        for (int i = 0; i<field_number; i++)
            rep += fields[i]->repr_ASCII() + ", ";
        return rep;
    }

};

class Table
{
protected:
    TableRowTemplate register_template;
public:
    Table(TableRowTemplate templ)
    {
        register_template = templ;
    }
    virtual int insert(TableRow& reg) = 0;
    virtual std::string repr_readable() = 0;
    //int update(function who, function what) = 0;
    //int remove(function who) = 0;


};

// this is a table with asociated storage
class FileTable : public Table
{
    // must support CRUD

    // must contain indexes and metadata
};

class MaterializedTable : public Table 
{
private:
    std::vector<TableRow> rows;
public:

    MaterializedTable(TableRowTemplate t):Table(t)
    {

    }

    int insert(TableRow& reg) override
    {
        // first check if register matches the appropiate template
        if (reg.field_number != register_template.fieldTypes.size())
        {
            return -1; // signal error: mismatched sizes on insert
        }
        
        for (int i = 0; i<reg.field_number;i++)
        {
            if (register_template.fieldTypes[i] != reg.fields[i]->getType())
                return -2; // signal error: mismatched types
        }
        TableRow row = reg; // get a copy
        rows.emplace_back(row); //emplace it!
        return 1;
    }

    std::string repr_readable() override
    {
        std::string s;
        for (int i=0;i<register_template.num_fields;i++)
        {
            s += register_template.field_names[i] + " | ";
        }
        s+= "\n--------------------------------------------\n";
        for(auto i:rows)
        {
            s+= i.repr_ASCII();
            s+= '\n';
        }
        return s;
    }
    // this is a table living entirelly within RAM. It is the result of a query. 
    // Modification operations are not supported 

    // In the future it might use a tempfile

    // must be queriable and non-modifiable
};

#include <iostream>

Table* make_sure_destructors_run()
{
    TableRowTemplate templ; // creamos el template: el template guarda los nombres y los tipos de los campos
    // llenamos el template de los campos. _Creo_ que hay una forma mas bonita de hacer esto!!
            // notas al implementador: La idea es que tanto la construccion de registros como de templates sea con
            // este metodo de poner miles de metodos que van construyendo al coso poco a poco.
    templ.field(varchar_ft,"name").field(integer_ft,"age").field(floating_ft,"balance");
    

    TableRow reg1(templ);

    int temp1 = 42;
    double temp2 = 3.141592;
    reg1.GetField(0)->SetValue((void*)"Juan");
    reg1.GetField(1)->SetValue((void*)&temp1);
    reg1.GetField(2)->SetValue((void*)&temp2);

    Table* t = new MaterializedTable(templ); // esta es la tabla. la tabla necesita 
    t->insert(reg1);

    return t;
}

int main(void)
{
/*
    // *******************
    //  * Example usage  *
    //  ******************

     TableRowTemplate templ; // creamos el template: el template guarda los nombres y los tipos de los campos
    // llenamos el template de los campos. _Creo_ que hay una forma mas bonita de hacer esto!!
            // notas al implementador: La idea es que tanto la construccion de registros como de templates sea con
            // este metodo de poner miles de metodos que van construyendo al coso poco a poco.
    templ.field(varchar_ft,"name").field(integer_ft,"age").field(floating_ft,"balance");
    

    TableRow reg1(templ); // aca se instancia el registro con un template!!

    int temp1 = 42;
    double temp2 = 3.141592;
    reg1.GetField(0)->SetValue((void*)"Juan"); // to set the value of a field you pass a pointer to the appropiate type of object
    reg1.GetField(1)->SetValue((void*)&temp1);
    reg1.GetField(2)->SetValue((void*)&temp2);

    std::cout << reg1.repr_ASCII() << std::endl;

    Table* t = new MaterializedTable(templ); // esta es la tabla. la tabla necesita su template 
    t->insert(reg1);
    */
    Table* t = make_sure_destructors_run();
    std::cout << t->repr_readable();

    return 0;
}