#include "JSDatabase.h"

#include <sqlite3.h> // TODO: Remove sqlite include
#include <v8.h>
#include <string>
#include <vector>
#include <map>

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
    return scope.Close(val);
}

JSDatabase* JSDatabase::UnwrapDb(const AccessorInfo& info)
{
    Handle<External> field = Handle<External>::Cast(info.Holder()->GetInternalField(0));
    void *ptr = field->Value();
    return static_cast<JSDatabase*>(ptr);
}
std::map<std::string, int>* JSDatabase::UnwrapCols(const AccessorInfo& info)
{
    Handle<External> field = Handle<External>::Cast(info.Holder()->GetInternalField(0));
    void *ptr = field->Value();
    return static_cast<std::map<std::string, int>*>(ptr);
}
std::vector< Handle<Value> >* JSDatabase::UnwrapRow(const AccessorInfo& info)
{
    Handle<External> field = Handle<External>::Cast(info.Holder()->GetInternalField(0));
    void *ptr = field->Value();
    return static_cast<std::vector< Handle<Value> >*>(ptr);
}

Handle<Value> JSDatabase::MapGet(Local<String> property, const AccessorInfo& info)
{
    std::vector< Handle<Value> > *row = UnwrapRow(info);
    std::map<std::string, int> *columns = UnwrapCols(info);

    String::Utf8Value name(property);
    std::map<std::string, int>::iterator iter = columns->find(*name);
    if(iter == columns->end())
        return v8::Undefined();
    else
        return (*row)[iter->second];
}

Handle<Array> JSDatabase::MapEnumerate(const AccessorInfo& info)
{
    HandleScope scope;

    std::map<std::string, int> *columns = UnwrapCols(info);

    Handle<Array> arr = Array::New(columns->size());
    std::map<std::string, int>::iterator iter;
    for(iter = columns->begin(); iter != columns->end(); ++iter) {
        arr->Set(Number::New(iter->second), String::New(iter->first.c_str()));
    }

    return scope.Close(arr);
}

Handle<Boolean> JSDatabase::MapQuery(Local<String> property, const AccessorInfo& info)
{
    std::map<std::string, int> *columns = UnwrapCols(info);

    String::Utf8Value val(property);
    if(columns->find(*val) != columns->end())
        return v8::True();
    else
        return v8::False();

}

Handle<Value> JSDatabase::ReadRow(int index)
{
    HandleScope handle_scope;

    while(!done && index >= rows.size()) {
        int state = step();
        if(state == SQLITE_DONE) {
            // TODO: Investigate zero-row queries
            done = true;
        }
        if(columns.size() < 1 && !done)
            SaveColNames();

        std::vector<Handle<Value> > row;
        int cols = ColCount();
        for(int i=0; i < cols; ++i) {
            Persistent<Value> col;
            switch(ColType(i)) {
                case SQLITE_TEXT:
                    col = Persistent<String>::New(String::New(getString().c_str()));
                    row.push_back(col);
                    break;
                case SQLITE_INTEGER:
                    col = Persistent<Number>::New(Number::New(getInt()));
                    row.push_back(col);
                    break;
                case SQLITE_FLOAT:
                    col = Persistent<Number>::New(Number::New(getFloat()));
                    row.push_back(col);
                    break;
                case SQLITE_NULL:
                    col = Persistent<Primitive>::New(Null());
                    row.push_back(col);
                    break;
            }
        }

        rows.push_back(row);
    }
        
    if(index >= rows.size()) {
        return v8::Undefined();
    }

    Handle<ObjectTemplate> row = ObjectTemplate::New();
    row->SetNamedPropertyHandler(MapGet);
    //row->SetNamedPropertyHandler(MapGet, 0, MapQuery, 0, MapEnumerate);
    row->SetInternalFieldCount(2);

    Handle<Object> r = row->NewInstance();

    Handle<External> col_ptr = External::New(&columns);
    Handle<External> row_ptr = External::New(&rows[index]);
    r->SetInternalField(0, col_ptr);
    r->SetInternalField(1, row_ptr);

    handle_scope.Close(r);
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
    
    done = false;

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
