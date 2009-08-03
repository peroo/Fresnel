#include "JSDatabase.h"

#include <sqlite3.h> // TODO: Remove sqlite include
#include <v8.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>

using namespace v8;

Handle<Value> JSDatabase::GetRowsAffected(Local<String> str, const AccessorInfo& info)
{
    JSDatabase *jsdb = UnwrapDb(info);
    return Number::New(jsdb->rows_affected());
}

Handle<Value> JSDatabase::GetInsertId(Local<String> str, const AccessorInfo& info)
{
    JSDatabase *jsdb = UnwrapDb(info);
    return Number::New(jsdb->last_insert_id());
}

Handle<Value> JSDatabase::GetRow(uint32_t index, const AccessorInfo& info)
{
    HandleScope scope;
    JSDatabase *jsdb = UnwrapDb(info);
    Handle<Value> val = jsdb->ReadRow((int)index);
    return val;
}

JSDatabase* JSDatabase::UnwrapDb(const AccessorInfo& info)
{
    Handle<External> field = Handle<External>::Cast(info.Holder()->GetInternalField(0));
    void *ptr = field->Value();
    return static_cast<JSDatabase*>(ptr);
}

Handle<Value> JSDatabase::ReadRow(int index)
{
    HandleScope handle_scope;

    while(!done && index >= rows.size()) {
        // TODO: Investigate zero-row queries
        
        if(columns.size() < 1)
            SaveColNames();

        Handle<Object> asd = Object::New();
        Persistent<Object> row = Persistent<Object>::New(asd);
        int cols = ColCount();
        for(int i=0; i < cols; ++i) {
            std::string n = ColName(i);
            Handle<String> nam = String::New(n.c_str());
            Persistent<String> name = Persistent<String>::New(nam);
            Handle<Value> val;

            switch(ColType(i)) {
                case SQLITE_TEXT:
                    val = String::New(getString().c_str());
                    break;
                case SQLITE_INTEGER:
                    val = Number::New(getInt());
                    break;
                case SQLITE_FLOAT:
                    val = Number::New(getFloat());
                    break;
                case SQLITE_NULL:
                    getVoid();
                    val = Null();
                    break;
            }
            Persistent<Value> value = Persistent<Value>::New(val);
            row->Set(name, value);
        }

        rows.push_back(row);
        done = !step();
    }
        
    if(index >= rows.size()) {
        return v8::Undefined();
    }

    return rows[index];
}

void JSDatabase::SaveColNames()
{
    int len = ColCount();
    for(int i=0; i < len; ++i) {
        columns[ColName(i)] = i;
    }

}

Handle<Value> JSDatabase::Query(const Arguments& arg)
{
    JSDatabase *jsdb = new JSDatabase();
    return jsdb->Execute(arg);
}

Handle<Value> JSDatabase::Execute(const Arguments& arg)
{
    if(arg.Length() < 1)
        return v8::Undefined();

    String::Utf8Value val(arg[0]);
    query(*val);

    for(int i = 1; i < arg.Length(); ++i) {
        if(arg[i]->IsNumber()) {
            bindInt(arg[0]->ToNumber()->Value());
        }
        else if(arg[i]->IsString()) {
            String::Utf8Value str(arg[i]);
            bindString(*str);
        }
        else {
            return v8::Undefined();
        }
    }
    
    done = !step();

    return MakeObject();
}

Handle<Object> JSDatabase::MakeObject()
{
    HandleScope handle_scope;

    Handle<ObjectTemplate> temp = ObjectTemplate::New();
    Handle<External> db_ptr = External::New(this);
    temp->SetInternalFieldCount(1);

    temp->SetAccessor(String::New("insertId"), GetInsertId);
    temp->SetAccessor(String::New("rowsAffected"), GetRowsAffected);

    Handle<ObjectTemplate> rows = ObjectTemplate::New();
    rows->SetIndexedPropertyHandler(GetRow);
    rows->SetInternalFieldCount(1);

    Handle<Object> rowObj = rows->NewInstance();
    rowObj->SetInternalField(0, db_ptr);

    temp->Set("rows", rowObj);
    Handle<Object> result = temp->NewInstance();
    result->SetInternalField(0, db_ptr);

    return handle_scope.Close(result);
}
