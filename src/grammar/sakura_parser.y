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
%define api.namespace {Kitsune::Sakura}
%define api.token.constructor
%define api.value.type variant

%define parse.assert

%code requires
{
#include <string>
#include <iostream>
#include <vector>
#include <data_structure/dataItems.hpp>

using Kitsune::Common::DataItem;
using Kitsune::Common::DataArray;
using Kitsune::Common::DataValue;
using Kitsune::Common::DataObject;

namespace Kitsune
{
namespace Sakura
{

class SakuraParserInterface;

}  // namespace Sakura
}  // namespace Kitsune
}

// The parsing context.
%param { Kitsune::Sakura::SakuraParserInterface& driver }

%locations

%code
{
#include <sakura_parsing/sakura_parser_interface.hpp>
# undef YY_DECL
# define YY_DECL \
    Kitsune::Sakura::SakuraParser::symbol_type sakuralex (Kitsune::Sakura::SakuraParserInterface& driver)
YY_DECL;
}

// Token
%define api.token.prefix {Sakura_}
%token
    END  0  "end of file"
    LINEBREAK "lbreak"
    FOREST  "FOREST"
    TREE  "TREE"
    BRANCH  "BRANCH"
    ASSIGN  ":"
    DOT  "."
    COMMA  ","
    DELIMITER  "|"
    MINUS   "-"
    LBRACK  "["
    RBRACK  "]"
    LBRACKBOW  "{"
    RBRACKBOW  "}"
;

%token <std::string> IDENTIFIER "identifier"
%token <std::string> STRING "string"
%token <int> NUMBER "number"
%token <float> FLOAT "float"

%type  <std::string> name_item

%type  <DataObject*> blossom
%type  <DataArray*> blossom_set

%type  <std::pair<std::string, DataItem*>> item
%type  <DataObject*> item_set
%type  <DataArray*> item_list
%type  <DataArray*> forest_parts
%type  <DataObject*> forest_part

%type  <std::pair<std::string, DataItem*>> setting
%type  <DataObject*> setting_set

%type  <DataObject*> branch
%type  <DataObject*> tree
%type  <DataObject*> forest

%type  <DataObject*> tree_branch
%type  <DataObject*> tree_sequentiell
%type  <DataObject*> tree_parallel

%%
%start startpoint;

startpoint:
    forest linebreaks_sp
    {
        driver.setOutput($1);
    }
|
    tree linebreaks_sp
    {
        driver.setOutput($1);
    }
|
    branch linebreaks_sp
    {
        driver.setOutput($1);
    }

forest:
   "[" "FOREST" ":" name_item "]" linebreaks item_set forest_parts
   {
       $$ = new DataObject();
       $$->insert("name", new DataValue($4));
       $$->insert("type", new DataValue("forest"));
       $$->insert("items", $7);
       $$->insert("parts", $8);
   }

tree:
   "[" "TREE" ":" name_item "]" linebreaks item_set tree_parallel
   {
       $$ = new DataObject();
       $$->insert("name", new DataValue($4));
       $$->insert("type", new DataValue("tree"));
       $$->insert("items", $7);

       DataArray* tempItem = new DataArray();
       tempItem->append($8);
       $$->insert("parts", tempItem);
   }

branch:
   "[" "BRANCH" ":" name_item "]" linebreaks item_set blossom_set
   {
       $$ = new DataObject();
       $$->insert("name", new DataValue($4));
       $$->insert("type", new DataValue("branch"));
       $$->insert("items", $7);
       $$->insert("parts", $8);
   }

blossom:
   "[" name_item "]" linebreaks setting_set "-" "identifier" ":" linebreaks item_set
   {
       $$ = new DataObject();
       $$->insert("type", new DataValue("blossom"));
       $$->insert("name", new DataValue($2));
       $$->insert("common-settings", $5);
       $$->insert("blossom-type", new DataValue($7));
       $$->insert("blossom-subtypes", new DataArray());
       $$->insert("items-input", $10);
   }
|
   "[" name_item "]" linebreaks setting_set "-" "identifier" linebreaks "-" item_list ":" linebreaks item_set
   {
       $$ = new DataObject();
       $$->insert("type", new DataValue("blossom"));
       $$->insert("name", new DataValue($2));
       $$->insert("common-settings", $5);
       $$->insert("blossom-type", new DataValue($7));
       $$->insert("blossom-subtypes", $10);
       $$->insert("items-input", $13);
   }

forest_parts:
   forest_parts forest_part
   {
       $1->append($2);
       $$ = $1;
   }
|
   forest_part
   {
       $$ = new DataArray();
       $$->append($1);
   }

forest_part:
   "[" name_item "]" linebreaks setting_set "-" "TREE" name_item ":" linebreaks item_set
   {
       $$ = new DataObject();
       $$->insert("type", new DataValue("forest"));
       $$->insert("name", new DataValue($2));
       $$->insert("common-settings", $5);
       $$->insert("item-name", new DataValue($8));
       $$->insert("item-type", new DataValue("tree"));
       $$->insert("items-input", $11);
   }
|
   "[" name_item "]" linebreaks setting_set "-" "BRANCH" name_item ":" linebreaks item_set
   {
       $$ = new DataObject();
       $$->insert("type", new DataValue("forest"));
       $$->insert("name", new DataValue($2));
       $$->insert("common-settings", $5);
       $$->insert("item-name", new DataValue($8));
       $$->insert("item-type", new DataValue("branch"));
       $$->insert("items-input", $11);
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

setting_set:
   %empty
   {
       $$ = new DataObject();
   }
|
   setting_set setting linebreaks
   {
       $1->insert($2.first, $2.second);
       $$ = $1;
   }
|
   setting linebreaks
   {
       $$ = new DataObject();
       $$->insert($1.first, $1.second);
   }

setting:
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
       $$ = new DataObject();
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
       $$ = new DataObject();
       $$->insert($1.first, $1.second);
   }

item:
   "identifier" ":" "{" "{" "}" "}"
   {
       // uset value
       std::string empty = "{{}}";
       std::pair<std::string, DataItem*> tempItem;
       tempItem.first = $1;
       tempItem.second = new DataValue(empty);
       $$ = tempItem;
   }
|
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
|
   "identifier" ":" item_list
   {
       std::pair<std::string, DataItem*> tempItem;
       tempItem.first = $1;
       tempItem.second = $3;
       $$ = tempItem;
   }

item_list:
   item_list "," "identifier"
   {
       $1->append(new DataValue($3));
       $$ = $1;
   }
|
   item_list "identifier"
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
   tree_sequentiell linebreaks_sp tree_branch
   {
       DataArray* array = (DataArray*)$1->get("parts");
       array->append($3);
       $$ = $1;
   }
|
   tree_sequentiell linebreaks_sp "[" linebreaks_sp tree_parallel linebreaks_sp "]"
   {
       DataArray* array = (DataArray*)$1->get("parts");
       array->append($5);
       $$ = $1;
   }
|
   tree_branch
   {
       $$ = new DataObject();
       $$->insert("type", new DataValue("sequentiell"));

       DataArray* tempItem = new DataArray();
       tempItem->append($1);

       $$->insert("parts", tempItem);
   }
|
   "[" linebreaks_sp tree_parallel linebreaks_sp "]"
   {
       $$ = new DataObject();
       $$->insert("type", new DataValue("sequentiell"));

       DataArray* tempItem = new DataArray();
       tempItem->append($3);

       $$->insert("parts", tempItem);
   }

tree_parallel:
   "{" linebreaks_sp tree_sequentiell linebreaks_sp "}"
   {
       $$ = new DataObject();
       $$->insert("type", new DataValue("parallel"));

       DataArray* tempItem = new DataArray();
       tempItem->append($3);

       $$->insert("parts", tempItem);
   }

tree_branch:
   "[" "BRANCH" ":" name_item "]" linebreaks item_set
   {
       DataObject* tempItem = new DataObject();
       tempItem->insert("type", new DataValue("branch"));
       tempItem->insert("name", new DataValue(driver.removeQuotes($4)));
       tempItem->insert("items-input", $7);
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

void Kitsune::Sakura::SakuraParser::error(const Kitsune::Sakura::location& location,
                                          const std::string& message)
{
    driver.error(location, message);
}
