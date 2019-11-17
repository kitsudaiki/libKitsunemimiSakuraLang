/**
 * @file        sakura_parser.y
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
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
    IF  "if"
    ELSE  "else"
    ASSIGN  ":"
    DOT  "."
    COMMA  ","
    DELIMITER  "|"
    ARROW   "->"
    MINUS   "-"
    EQUAL   "="
    LBRACK  "["
    RBRACK  "]"
    LBRACKBOW  "{"
    RBRACKBOW  "}"
    LROUNDBRACK  "("
    RROUNDBRACK  ")"
    EQUAL_COMPARE "=="
    UNEQUAL_COMPARE "!="
    GREATER_EQUAL_COMPARE ">="
    SMALLER_EQUAL_COMPARE "<="
    GREATER_COMPARE ">"
    SMALLER_COMPARE "<"
    SHIFT_LEFT "<<"
    SHIFT_RIGHT ">>"
;

%token <std::string> IDENTIFIER "identifier"
%token <std::string> STRING "string"
%token <int> NUMBER "number"
%token <float> FLOAT "float"

%type  <std::string> name_item
%type  <std::string> compare_type
%type  <DataValue*>  value_item
%type  <DataArray*>  json_path

%type  <DataMap*> blossom_group
%type  <DataArray*> blossom_group_set
%type  <DataMap*> blossom
%type  <DataArray*> blossom_set

%type  <std::pair<std::string, DataItem*>> item
%type  <DataMap*> item_set
%type  <DataArray*> string_array

%type  <DataMap*> if_condition

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
   "[" name_item "]" linebreaks item_set blossom_group_set
   {
       $$ = new DataMap();
       $$->insert("id", new DataValue($2));
       $$->insert("btype", new DataValue("branch"));
       $$->insert("items", $5);
       $$->insert("parts", $6);
   }

if_condition:
   "if" "(" json_path compare_type value_item ")" linebreaks "{" linebreaks blossom_group_set "}" linebreaks "else" linebreaks "{" linebreaks blossom_group_set "}" linebreaks
   {
       $$ = new DataMap();
       $$->insert("btype", new DataValue("if"));
       $$->insert("if_type", new DataValue($4));
       $$->insert("left", $3);
       $$->insert("right", $5);

       $$->insert("if_parts", $10);
       $$->insert("else_parts", $17);
   }
|
    "if" "(" json_path compare_type value_item ")" linebreaks "{" linebreaks blossom_group_set "}" linebreaks
    {
        $$ = new DataMap();
        $$->insert("btype", new DataValue("if"));
        $$->insert("if_type", new DataValue($4));
        $$->insert("left", $3);
        $$->insert("right", $5);

        $$->insert("if_parts", $10);
        $$->insert("else_parts", new DataArray());
    }

blossom_group_set:
   blossom_group_set if_condition
   {
       $1->append($2);
       $$ = $1;
   }
|
   blossom_group_set blossom_group
   {
       $1->append($2);
       $$ = $1;
   }
|
   if_condition
   {
       $$ = new DataArray();
       $$->append($1);
   }
|
   blossom_group
   {
       $$ = new DataArray();
       $$->append($1);
   }

blossom_group:
   "identifier" "(" name_item ")" linebreaks item_set blossom_set
   {
       $$ = new DataMap();
       $$->insert("btype", new DataValue("blossom_group"));
       $$->insert("name", new DataValue($3));
       $$->insert("blossom-group-type", new DataValue($1));
       $$->insert("items-input", $6);
       $$->insert("blossoms", $7);
   }
|
   "identifier" "(" name_item ")" linebreaks blossom_set
   {
       $$ = new DataMap();
       $$->insert("btype", new DataValue("blossom_group"));
       $$->insert("name", new DataValue($3));
       $$->insert("blossom-group-type", new DataValue($1));
       $$->insert("items-input", new DataMap());
       $$->insert("blossoms", $6);
   }
|
   "identifier" "(" name_item ")" linebreaks item_set
   {
       $$ = new DataMap();
       $$->insert("btype", new DataValue("blossom_group"));
       $$->insert("name", new DataValue($3));
       $$->insert("blossom-group-type", new DataValue($1));
       $$->insert("items-input", $6);
       $$->insert("blossoms", new DataArray());
   }
|
  "identifier" "(" name_item ")" linebreaks
  {
      $$ = new DataMap();
      $$->insert("btype", new DataValue("blossom_group"));
      $$->insert("name", new DataValue($3));
      $$->insert("blossom-group-type", new DataValue($1));
      $$->insert("items-input", new DataMap());
      $$->insert("blossoms", new DataArray());
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
   "->" "identifier" linebreaks
   {
        $$ = new DataMap();
        $$->insert("btype", new DataValue("blossom"));
        $$->insert("blossom-type", new DataValue($2));
        $$->insert("items-input", new DataMap());
   }
|
   "->" "identifier" ":" linebreaks item_set
   {
       $$ = new DataMap();
       $$->insert("btype", new DataValue("blossom"));
       $$->insert("blossom-type", new DataValue($2));
       $$->insert("items-input", $5);
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
   "-" "identifier" "=" "{" "{" "}" "}"
   {
       // uset value
       std::string empty = "{{}}";
       std::pair<std::string, DataItem*> tempItem;
       tempItem.first = $2;
       tempItem.second = new DataValue(empty);
       $$ = tempItem;
   }
|
   "-" "identifier" "=" value_item
   {
       std::pair<std::string, DataItem*> tempItem;
       tempItem.first = $2;
       tempItem.second = $4;
       $$ = tempItem;
   }
|
   "-" "identifier" "=" "[" string_array "]"
   {
       std::pair<std::string, DataItem*> tempItem;
       tempItem.first = $2;
       tempItem.second = $5;
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

json_path:
   json_path "." "identifier"
   {
       $1->append(new DataValue($3));
       $$ = $1;
   }
|
   "identifier"
   {
       DataArray* tempItem = new DataArray();
       tempItem->append(new DataValue($1));
       $$ = tempItem;
   }

value_item:
   "float"
   {
      $$ = new DataValue($1);
   }
|
   "number"
   {
       $$ = new DataValue($1);
   }
|
   "identifier"
   {
       $$ = new DataValue($1);
   }
|
   "string"
   {
       $$ = new DataValue(driver.removeQuotes($1));
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

compare_type:
   "=="
   {
       $$ = "==";
   }
|
   ">="
   {
       $$ = ">=";
   }
|
   "<="
   {
       $$ = "<=";
   }
|
   ">"
   {
       $$ = ">";
   }
|
   "<"
   {
       $$ = "<";
   }
|
   "!="
   {
       $$ = "!=";
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
