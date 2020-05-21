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
%locations

%define parse.assert

%code requires
{
#include <string>
#include <iostream>
#include <vector>
#include <libKitsunemimiCommon/common_items/data_items.h>
#include <libKitsunemimiSakuraParser/sakura_items.h>
#include <libKitsunemimiSakuraParser/value_items.h>

using Kitsunemimi::DataItem;
using Kitsunemimi::DataArray;
using Kitsunemimi::DataValue;
using Kitsunemimi::DataMap;

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
    BOOL_TRUE  "true"
    BOOL_FALSE "false"
    SEED  "seed"
    TREE  "tree"
    SUBTREE  "subtree"
    PARALLEL_FOR "parallel_for"
    PARALLEL  "parallel"
    IF  "if"
    ELSE  "else"
    FOR  "for"
    ASSIGN  ":"
    SEMICOLON  ";"
    DOT  "."
    COMMA  ","
    DELIMITER  "|"
    ARROW   "->"
    MINUS   "-"
    PLUS    "+"
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
%token <std::string> STRING_PLN "string_pln"
%token <long> NUMBER "number"
%token <double> FLOAT "float"

%type  <std::string> name_item
%type  <std::string> compare_type
%type  <ValueItem>  value_item
%type  <std::vector<ValueItem>*>  value_item_list
%type  <std::string> regiterable_identifier;

%type  <BlossomGroupItem*> blossom_group
%type  <SequentiellPart*> blossom_group_set
%type  <BlossomItem*> blossom
%type  <std::vector<BlossomItem*>*> blossom_set

%type  <ValueItemMap*> item_set

%type  <FunctionItem> access
%type  <std::vector<FunctionItem>*> access_list
%type  <FunctionItem> function
%type  <std::vector<FunctionItem>*> function_list

%type  <IfBranching*> if_condition
%type  <ForEachBranching*> for_each_loop
%type  <ForBranching*> for_loop

%type  <ParallelPart*> parallel

%type  <TreeItem*> tree
%type  <SeedItem*> seed_fork
%type  <SubtreeItem*> subtree_fork

%type  <DataItem*> json_abstract
%type  <DataValue*> json_value
%type  <DataArray*> json_array
%type  <DataArray*> json_array_content
%type  <DataMap*> json_object
%type  <DataMap*> json_object_content

%%
%start startpoint;

startpoint:
    tree
    {
        driver.setOutput($1);
    }

tree:
    "[" name_item "]" item_set blossom_group_set
    {
        $$ = new TreeItem();
        $$->id = $2;
        $$->values = *$4;
        delete $4;
        $$->childs = $5;
    }

if_condition:
    "if" "(" value_item compare_type value_item ")" "{" blossom_group_set "}" "else" "{" blossom_group_set "}"
    {
        $$ = new IfBranching();
        $$->leftSide = $3;
        $$->rightSide = $5;

        if($4 == "==") {
            $$->ifType = IfBranching::EQUAL;
        }
        if($4 == "!=") {
            $$->ifType = IfBranching::UNEQUAL;
        }

        $$->ifContent = $8;
        $$->elseContent = $12;
    }
|
    "if" "(" value_item compare_type value_item ")" "{" blossom_group_set "}"
    {
        $$ = new IfBranching();
        $$->leftSide = $3;
        $$->rightSide = $5;

        if($4 == "==") {
            $$->ifType = IfBranching::EQUAL;
        }
        if($4 == "!=") {
            $$->ifType = IfBranching::UNEQUAL;
        }

        $$->ifContent = $8;
        $$->elseContent = nullptr;
    }

for_each_loop:
    "for" "(" regiterable_identifier ":" value_item ")" item_set "{" blossom_group_set "}"
    {
        $$ = new ForEachBranching();
        $$->tempVarName = "";
        $$->iterateArray = $5;
        $$->values = *$7;
        delete $7;
        $$->content = $9;
    }
|
    "parallel_for" "(" regiterable_identifier ":" value_item ")" item_set "{" blossom_group_set "}"
    {
        $$ = new ForEachBranching();
        $$->tempVarName = "";
        $$->iterateArray = $5;
        $$->values = *$7;
        delete $7;
        $$->content = $9;
        $$->parallel = true;
    }

for_loop:
    "for" "(" regiterable_identifier "=" value_item ";" "identifier" "<" value_item ";" "identifier" "+" "+" ")" item_set "{" blossom_group_set "}"
    {
        if($7 != $3)
        {
            driver.error(yyla.location,
                         "undefined identifier \"" + $7 + "\"",
                         true);
            return 1;
        }
        if($11 != $3)
        {
            driver.error(yyla.location,
                         "undefined identifier \"" + $11 + "\"",
                         true);
            return 1;
        }

        $$ = new ForBranching();
        $$->tempVarName = $3;
        $$->start = $5;
        $$->end = $9;
        $$->values = *$15;
        delete $15;
        $$->content = $17;
    }
|
    "parallel_for" "(" regiterable_identifier "=" value_item ";" "identifier" "<" value_item ";" "identifier" "+" "+" ")" item_set "{" blossom_group_set "}"
    {
        if($7 != $3)
        {
            driver.error(yyla.location,
                         "undefined identifier \"" + $7 + "\"",
                         true);
            return 1;
        }
        if($11 != $3)
        {
            driver.error(yyla.location,
                         "undefined identifier \"" + $11 + "\"",
                         true);
            return 1;
        }

        $$ = new ForBranching();
        $$->tempVarName = $3;
        $$->start = $5;
        $$->end = $9;
        $$->values = *$15;
        delete $15;
        $$->content = $17;
        $$->parallel = true;
    }

parallel:
    "parallel" "(" ")" "{" blossom_group_set "}"
    {
        $$ = new ParallelPart();
        $$->childs = dynamic_cast<SequentiellPart*>($5);
    }



blossom_group_set:
    blossom_group_set if_condition
    {
        $1->childs.push_back($2);
        $$ = $1;
    }
|
    blossom_group_set for_loop
    {
        $1->childs.push_back($2);
        $$ = $1;
    }
|
    blossom_group_set parallel
    {
        $1->childs.push_back($2);
        $$ = $1;
    }
|
    blossom_group_set for_each_loop
    {
        $1->childs.push_back($2);
        $$ = $1;
    }
|
    blossom_group_set subtree_fork
    {
        $1->childs.push_back($2);
        $$ = $1;
    }
|
    blossom_group_set blossom_group
    {
        $1->childs.push_back($2);
        $$ = $1;
    }
|
    blossom_group_set seed_fork
    {
        $1->childs.push_back($2);
        $$ = $1;
    }
|
    if_condition
    {
        $$ = new SequentiellPart();
        $$->childs.push_back($1);
    }
|
    for_loop
    {
        $$ = new SequentiellPart();
        $$->childs.push_back($1);
    }
|
    parallel
    {
        $$ = new SequentiellPart();
        $$->childs.push_back($1);
    }
|
    for_each_loop
    {
        $$ = new SequentiellPart();
        $$->childs.push_back($1);
    }
|
    blossom_group
    {
        $$ = new SequentiellPart();
        $$->childs.push_back($1);
    }
|
    seed_fork
    {
        $$ = new SequentiellPart();
        $$->childs.push_back($1);
    }
|
    subtree_fork
    {
        $$ = new SequentiellPart();
        $$->childs.push_back($1);
    }

blossom_group:
    "identifier" "(" name_item ")" item_set blossom_set
    {
        $$ = new BlossomGroupItem();
        $$->blossomGroupType = $1;
        $$->id = $3;
        $$->values = *$5;
        delete $5;
        $$->blossoms = *$6;
        delete $6;
    }
|
    "identifier" "(" name_item ")" blossom_set
    {
        $$ = new BlossomGroupItem();
        $$->blossomGroupType = $1;
        $$->id = $3;
        $$->blossoms = *$5;
        delete $5;
    }
|
    "identifier" "(" name_item ")" item_set
    {
        $$ = new BlossomGroupItem();
        $$->blossomGroupType = $1;
        $$->id = $3;
        $$->values = *$5;
        delete $5;
    }
|
    "identifier" "(" name_item ")"
    {
        $$ = new BlossomGroupItem();
        $$->blossomGroupType = $1;
        $$->id = $3;
    }

blossom_set:
   blossom_set blossom
   {
       $1->push_back($2);
       $$ = $1;
   }
|
   blossom
   {
       $$ = new std::vector<BlossomItem*>();
       $$->push_back($1);
   }

blossom:
   "->" "identifier"
   {
       $$ = new BlossomItem();
       $$->blossomType = $2;
   }
|
   "->" "identifier" ":" item_set
   {
       $$ = new BlossomItem();
       $$->blossomType = $2;
       $$->values = *$4;
       delete $4;
   }


regiterable_identifier:
   "identifier"
   {
       driver.m_registeredKeys.push_back($1);
       $$ = $1;
   }

item_set:
    %empty
    {
        $$ = new ValueItemMap();
    }
|
    item_set  "-" regiterable_identifier "=" "{" "{" "}" "}"
    {
        std::string empty = "{{}}";
        ValueItem newItem;
        newItem.item = new DataValue(empty);

        $1->insert($3, newItem);
        $$ = $1;
    }
|
    item_set  "-" regiterable_identifier "=" value_item
    {
        $1->insert($3, $5);
        $$ = $1;
    }
|
    item_set  "-" regiterable_identifier "=" json_abstract
    {
        ValueItem newItem;
        newItem.item = $5;

        $1->insert($3, newItem);
        $$ = $1;
    }
|
    item_set  "-" value_item ">>" "identifier"
    {
        if(driver.isKeyRegistered($5) == false)
        {
            driver.error(yyla.location,
                         "undefined identifier \"" + $5 + "\"",
                         true);
            return 1;
        }

        ValueItem newItem = $3;
        newItem.type = ValueItem::OUTPUT_PAIR_TYPE;

        $1->insert($5, newItem);
        $$ = $1;
    }
|
    item_set  "-" "identifier" compare_type value_item
    {
        ValueItem newItem = $5;

        if($4 == "==") {
            newItem.type = ValueItem::COMPARE_EQUAL_PAIR_TYPE;
        }
        if($4 == "!=") {
            newItem.type = ValueItem::COMPARE_EQUAL_PAIR_TYPE;
        }

        $1->insert($3, newItem);
        $$ = $1;
    }
|
    "-" regiterable_identifier "=" "{" "{" "}" "}"
    {
        $$ = new ValueItemMap();

        std::string empty = "{{}}";
        ValueItem newItem;
        newItem.item = new DataValue(empty);

        $$->insert($2, newItem);
    }
|
    "-" regiterable_identifier "=" value_item
    {
        $$ = new ValueItemMap();
        $$->insert($2, $4);
    }
|
    "-" regiterable_identifier "=" json_abstract
    {
        $$ = new ValueItemMap();

        ValueItem newItem;
        newItem.item = $4;

        $$->insert($2, newItem);
    }
|
    "-" value_item ">>" "identifier"
    {
        if(driver.isKeyRegistered($4) == false)
        {
            driver.error(yyla.location,
                         "undefined identifier \"" + $4 + "\"",
                         true);
            return 1;
        }

        $$ = new ValueItemMap();

        ValueItem newItem = $2;
        newItem.type = ValueItem::OUTPUT_PAIR_TYPE;

        $$->insert($4, newItem);
    }
|
    "-" "identifier" compare_type value_item
    {
        $$ = new ValueItemMap();

        ValueItem newItem = $4;

        if($3 == "==") {
            newItem.type = ValueItem::COMPARE_EQUAL_PAIR_TYPE;
        }
        if($3 == "!=") {
            newItem.type = ValueItem::COMPARE_EQUAL_PAIR_TYPE;
        }

        $$->insert($2, newItem);
    }

subtree_fork:
    "subtree" "(" name_item ")" item_set
    {
        $$ = new SubtreeItem();
        $$->nameOrPath = $3;
        $$->values = *$5;
        delete $5;
    }

seed_fork:
    "seed" "(" name_item "," name_item ")" item_set
    {
        $$ = new SeedItem();
        $$->tag = $3;
        $$->treeId = $5;
        $$->values = *$7;
        delete $7;
    }

value_item_list:
    value_item_list ","  value_item
    {
        $1->push_back($3);
        $$ = $1;
    }
|
    value_item
    {
        $$ = new std::vector<ValueItem>();
        $$->push_back($1);
    }

value_item:
    "float"
    {
        ValueItem newItem;
        newItem.item = new DataValue($1);
        $$ = newItem;
    }
|
    "number"
    {
        ValueItem newItem;
        newItem.item = new DataValue($1);
        $$ = newItem;
    }
|
    "true"
    {
        ValueItem newItem;
        newItem.item = new DataValue(true);
        $$ = newItem;
    }
|
    "false"
    {
        ValueItem newItem;
        newItem.item = new DataValue(false);
        $$ = newItem;
    }
|
    "string"
    {
        ValueItem newItem;
        newItem.item = new DataValue($1);
        $$ = newItem;
    }
|
    "identifier"
    {
        if(driver.isKeyRegistered($1) == false)
        {
            driver.error(yyla.location,
                         "undefined identifier \"" + $1 + "\"",
                         true);
            return 1;
        }

        ValueItem newItem;
        newItem.item = new DataValue($1);
        newItem.isIdentifier = true;
        $$ = newItem;
    }
|
    "identifier" function_list
    {
        if(driver.isKeyRegistered($1) == false)
        {
            driver.error(yyla.location,
                         "undefined identifier \"" + $1 + "\"",
                         true);
            return 1;
        }

        ValueItem newItem;
        newItem.item = new DataValue($1);
        newItem.isIdentifier = true;
        newItem.functions = *$2;
        delete $2;
        $$ = newItem;
    }
|
    "identifier" access_list
    {
        if(driver.isKeyRegistered($1) == false)
        {
            driver.error(yyla.location,
                         "undefined identifier \"" + $1 + "\"",
                         true);
            return 1;
        }

        ValueItem newItem;
        newItem.item = new DataValue($1);
        newItem.isIdentifier = true;
        newItem.functions = *$2;
        delete $2;
        $$ = newItem;
    }

function_list:
    function_list function
    {
        $1->push_back($2);
        $$ = $1;
    }
|
    function
    {
        $$ = new std::vector<FunctionItem>();
        $$->push_back($1);
    }

function:
    "." "identifier" "(" ")"
    {
        FunctionItem newItem;
        newItem.type = $2;
        $$ = newItem;
    }
|
    "." "identifier" "(" value_item_list ")"
    {
        FunctionItem newItem;
        newItem.type = $2;
        newItem.arguments = *$4;
        delete $4;
        $$ = newItem;
    }

access_list:
    access_list access
    {
        $1->push_back($2);
        $$ = $1;
    }
|
    access
    {
        $$ = new std::vector<FunctionItem>();
        $$->push_back($1);
    }

access:
    "[" "identifier" "]"
    {
        FunctionItem newItem;
        newItem.type = "get";

        ValueItem value;
        value.item = new DataValue($2);
        newItem.arguments.push_back(value);

        $$ = newItem;
    }
|
    "[" "number" "]"
    {
        FunctionItem newItem;
        newItem.type = "get";

        ValueItem value;
        value.item = new DataValue($2);
        newItem.arguments.push_back(value);

        $$ = newItem;
    }
|
    "[" "string" "]"
    {
        FunctionItem newItem;
        newItem.type = "get";

        ValueItem value;
        value.item = new DataValue($2);
        newItem.arguments.push_back(value);

        $$ = newItem;
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


json_abstract:
   json_object
   {
       $$ = (DataItem*)$1;
   }
|
   json_array
   {
       $$ = (DataItem*)$1;
   }
|
   json_value
   {
       $$ = (DataItem*)$1;
   }

json_object:
   "{" json_object_content "}"
   {
       $$ = $2;
   }
|
   "{" "}"
   {
       $$ = new DataMap();
   }

json_object_content:
   json_object_content "," "identifier" ":" json_abstract
   {
       $1->insert($3, $5);
       $$ = $1;
   }
|
   "identifier" ":" json_abstract
   {
       $$ = new DataMap();
       $$->insert($1, $3);
   }
|
   json_object_content "," "string_pln" ":" json_abstract
   {
       $1->insert(driver.removeQuotes($3), $5);
       $$ = $1;
   }
|
   "string_pln" ":" json_abstract
   {
       $$ = new DataMap();
       $$->insert(driver.removeQuotes($1), $3);
   }
|
   json_object_content "," "string" ":" json_abstract
   {
       $1->insert(driver.removeQuotes($3), $5);
       $$ = $1;
   }
|
   "string" ":" json_abstract
   {
       $$ = new DataMap();
       $$->insert(driver.removeQuotes($1), $3);
   }

json_array:
   "[" json_array_content "]"
   {
       $$ = $2;
   }
|
   "[" "]"
   {
       $$ = new DataArray();
   }

json_array_content:
   json_array_content "," json_abstract
   {
       $1->append($3);
       $$ = $1;
   }
|
   json_abstract
   {
       $$ = new DataArray();
       $$->append($1);
   }

json_value:
   "number"
   {
       $$ = new DataValue($1);
   }
|
   "float"
   {
       $$ = new DataValue($1);
   }
|
   "string"
   {
       $$ = new DataValue(driver.removeQuotes($1));
   }
|
   "true"
   {
       $$ = new DataValue(true);
   }
|
   "false"
   {
       $$ = new DataValue(false);
   }

%%

void Kitsunemimi::Sakura::SakuraParser::error(const Kitsunemimi::Sakura::location& location,
                                              const std::string& message)
{
    driver.error(location, message, false);
}
