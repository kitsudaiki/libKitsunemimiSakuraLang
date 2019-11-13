/**
 *  @file    sakuraparser.y
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

%skeleton "lalr1.cc"

%defines

//requires 3.2 to avoid the creation of the stack.hh
%require "3.0.4"
%define parser_class_name {SakuraParser}

%define api.prefix {sakura}
%define api.namespace {Kitsunemimi::Sakura}
%define api.token.constructor
%define api.value.type variant

%define parse.assert

%code requires
{
#include <string>
#include <iostream>
#include <vector>
#include <libKitsunemimiCommon/common_items/data_items.h>

using Kitsunemimi::Common::DataItem;
using Kitsunemimi::Common::DataArray;
using Kitsunemimi::Common::DataValue;
using Kitsunemimi::Common::DataMap;

namespace Kitsunemimi
{
namespace Sakura
{

class SakuraParserInterface;

}  // namespace Sakura
}  // namespace Kitsunemimi
}

// The parsing context.
%param { Kitsunemimi::Sakura::SakuraParserInterface& driver }

%locations

%code
{
#include <sakura_parsing/sakura_parser_interface.h>
# undef YY_DECL
# define YY_DECL \
    Kitsunemimi::Sakura::SakuraParser::symbol_type sakuralex (Kitsunemimi::Sakura::SakuraParserInterface& driver)
YY_DECL;
}

// Token
%define api.token.prefix {Sakura_}
%token
    END  0  "end of file"
    LINEBREAK "lbreak"
    SEED  "seed"
    TREE  "tree"
    BRANCH  "branch"
    PARALLEL  "parallel"
    ASSIGN  ":"
    DOT  "."
    COMMA  ","
    DELIMITER  "|"
    ARROW   "->"
    MINUS   "-"
    LBRACK  "["
    RBRACK  "]"
    LBRACKBOW  "{"
    RBRACKBOW  "}"
    LROUNDBRACK  "("
    RROUNDBRACK  ")"
;

%token <std::string> IDENTIFIER "identifier"
%token <std::string> STRING "string"
%token <int> NUMBER "number"
%token <float> FLOAT "float"

%type  <std::string> name_item

%type  <DataMap*> blossom
%type  <DataArray*> blossom_set
%type  <DataMap*> blossom_subtype
%type  <DataArray*> blossom_subtype_set

%type  <std::pair<std::string, DataItem*>> item
%type  <DataMap*> item_set
%type  <DataArray*> string_array

%type  <std::pair<std::string, DataItem*>> static_option
%type  <DataMap*> static_options

%type  <DataMap*> branch
%type  <DataMap*> tree

%type  <DataMap*> tree_fork
%type  <DataMap*> tree_sequentiell
%type  <DataMap*> tree_parallel

%%
%start startpoint;

startpoint:
    tree
    {
        driver.setOutput($1);
    }
|
    branch
    {
        driver.setOutput($1);
    }

tree:
   "[" name_item "]" linebreaks item_set "{" linebreaks tree_sequentiell "}" linebreaks_sp
   {
       $$ = new DataMap();
       $$->insert("id", new DataValue($2));
       $$->insert("btype", new DataValue("tree"));
       $$->insert("items", $5);

       DataArray* tempItem = new DataArray();
       tempItem->append($8);
       $$->insert("parts", tempItem);
   }

branch:
   "[" name_item "]" linebreaks item_set blossom_set
   {
       $$ = new DataMap();
       $$->insert("id", new DataValue($2));
       $$->insert("btype", new DataValue("branch"));
       $$->insert("items", $5);
       $$->insert("parts", $6);
   }

blossom_set:
   blossom_set blossom
   {
       $1->append($2);
       $$ = $1;
   }
|
   blossom
   {
       $$ = new DataArray();
       $$->append($1);
   }

blossom:
   "identifier" "(" name_item ")" linebreaks blossom_subtype_set
   {
       $$ = new DataMap();
       $$->insert("btype", new DataValue("blossom"));
       $$->insert("name", new DataValue($3));
       $$->insert("blossom-type", new DataValue($1));
       $$->insert("blossom-subtypes", $6);
   }

blossom_subtype_set:
   blossom_subtype_set blossom_subtype
   {
       $1->append($2);
       $$ = $1;
   }
|
   blossom_subtype
   {
       $$ = new DataArray();
       $$->append($1);
   }

blossom_subtype:
   "->" "identifier" linebreaks
   {
        $$ = new DataMap();
        $$->insert("btype", new DataValue("blossom_subtype"));
        $$->insert("blossom-subtype", new DataValue($2));
   }
|
   "->" "identifier" ":" linebreaks item_set
   {
       $$ = new DataMap();
       $$->insert("btype", new DataValue("blossom_subtype"));
       $$->insert("blossom-subtype", new DataValue($2));
       $$->insert("items-input", $5);
   }

static_option:
   "identifier" ":" "identifier"
   {
       std::pair<std::string, DataItem*> tempItem;
       tempItem.first = $1;
       tempItem.second = new DataValue($3);
       $$ = tempItem;
   }
|
   "identifier" ":" "string"
   {
       std::pair<std::string, DataItem*> tempItem;
       tempItem.first = $1;
       tempItem.second = new DataValue(driver.removeQuotes($3));
       $$ = tempItem;
   }
|
   "identifier" ":" "number"
   {
       std::pair<std::string, DataItem*> tempItem;
       tempItem.first = $1;
       tempItem.second = new DataValue($3);
       $$ = tempItem;
   }
|
   "identifier" ":" "float"
   {
       std::pair<std::string, DataItem*> tempItem;
       tempItem.first = $1;
       tempItem.second = new DataValue($3);
       $$ = tempItem;
   }

item_set:
   %empty
   {
       $$ = new DataMap();
   }
|
   item_set  item  linebreaks
   {
       $1->insert($2.first, $2.second);
       $$ = $1;
   }
|
   item linebreaks
   {
       $$ = new DataMap();
       $$->insert($1.first, $1.second);
   }

item:
   "-" "identifier" ":" "{" "{" "}" "}"
   {
       // uset value
       std::string empty = "{{}}";
       std::pair<std::string, DataItem*> tempItem;
       tempItem.first = $2;
       tempItem.second = new DataValue(empty);
       $$ = tempItem;
   }
|
   "-" "identifier" ":" "identifier"
   {
       std::pair<std::string, DataItem*> tempItem;
       tempItem.first = $2;
       tempItem.second = new DataValue($4);
       $$ = tempItem;
   }
|
   "-" "identifier" ":" "string"
   {
       std::pair<std::string, DataItem*> tempItem;
       tempItem.first = $2;
       tempItem.second = new DataValue(driver.removeQuotes($4));
       $$ = tempItem;
   }
|
   "-" "identifier" ":" "number"
   {
       std::pair<std::string, DataItem*> tempItem;
       tempItem.first = $2;
       tempItem.second = new DataValue($4);
       $$ = tempItem;
   }
|
   "-" "identifier" ":" "float"
   {
       std::pair<std::string, DataItem*> tempItem;
       tempItem.first = $2;
       tempItem.second = new DataValue($4);
       $$ = tempItem;
   }
|
   "-" "identifier" ":" string_array
   {
       std::pair<std::string, DataItem*> tempItem;
       tempItem.first = $2;
       tempItem.second = $4;
       $$ = tempItem;
   }

string_array:
   string_array "," "identifier"
   {
       $1->append(new DataValue($3));
       $$ = $1;
   }
|
   string_array "identifier"
   {
       $1->append(new DataValue($2));
       $$ = $1;
   }
|
   "identifier" "identifier"
   {
       $$ = new DataArray();
       $$->append(new DataValue($1));
       $$->append(new DataValue($2));
   }
|
   "identifier" "," "identifier"
   {
       $$ = new DataArray();
       $$->append(new DataValue($1));
       $$->append(new DataValue($3));
   }
|
   "identifier"
   {
       $$ = new DataArray();
       $$->append(new DataValue($1));
   }

tree_sequentiell:
   tree_sequentiell tree_fork
   {
       DataArray* array = (DataArray*)$1->get("parts");
       array->append($2);
       $$ = $1;
   }
|
   tree_sequentiell linebreaks "{" linebreaks tree_parallel "}" linebreaks
   {
       DataArray* array = (DataArray*)$1->get("parts");
       array->append($5);
       $$ = $1;
   }
|
   tree_fork
   {
       $$ = new DataMap();
       $$->insert("btype", new DataValue("sequentiell"));

       DataArray* tempItem = new DataArray();
       tempItem->append($1);

       $$->insert("parts", tempItem);
   }
|
   "{" linebreaks tree_parallel "}" linebreaks
   {
       $$ = new DataMap();
       $$->insert("btype", new DataValue("sequentiell"));

       DataArray* tempItem = new DataArray();
       tempItem->append($3);

       $$->insert("parts", tempItem);
}

tree_parallel:
   "parallel" "(" ")" linebreaks "{" linebreaks tree_sequentiell "}" linebreaks
   {
       $$ = new DataMap();
       $$->insert("btype", new DataValue("parallel"));

       DataArray* tempItem = new DataArray();
       tempItem->append($7);

       $$->insert("parts", tempItem);
   }

tree_fork:
   "tree" "(" "identifier" ")" linebreaks item_set
   {
       DataMap* tempItem = new DataMap();
       tempItem->insert("btype", new DataValue("tree"));
       tempItem->insert("id", new DataValue($3));
       tempItem->insert("items-input", $6);
       $$ = tempItem;
   }
|
   "branch" "(" "identifier" ")" linebreaks item_set
   {
       DataMap* tempItem = new DataMap();
       tempItem->insert("btype", new DataValue("branch"));
       tempItem->insert("id", new DataValue($3));
       tempItem->insert("items-input", $6);
       $$ = tempItem;
   }
|
   "seed" "(" "identifier" ")" linebreaks item_set "{" linebreaks tree_sequentiell "}" linebreaks_sp
   {
       DataMap* tempItem = new DataMap();
       tempItem->insert("btype", new DataValue("seed"));
       tempItem->insert("id", new DataValue($3));
       tempItem->insert("connection", $6);
       tempItem->insert("subtree", $9);
       $$ = tempItem;
   }

name_item:
   "identifier"
   {
       $$ = $1;
   }
|
   "string"
   {
       $$ = driver.removeQuotes($1);
   }

linebreaks:
   linebreaks "lbreak"
|
   "lbreak"

linebreaks_sp:
   linebreaks_sp "lbreak"
|
   "lbreak"
|
   %empty

%%

void Kitsunemimi::Sakura::SakuraParser::error(const Kitsunemimi::Sakura::location& location,
                                          const std::string& message)
{
    driver.error(location, message);
}
