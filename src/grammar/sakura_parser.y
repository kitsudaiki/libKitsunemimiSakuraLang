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
    HOST  "HOST"
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

%type  <DataMap*> blossom
%type  <DataArray*> blossom_set

%type  <std::pair<std::string, DataItem*>> item
%type  <DataMap*> item_set
%type  <DataArray*> item_list

%type  <std::pair<std::string, DataItem*>> static_option
%type  <DataMap*> static_options

%type  <DataMap*> branch
%type  <DataMap*> tree

%type  <DataMap*> tree_branch
%type  <DataMap*> tree_sequentiell
%type  <DataMap*> tree_parallel

%%
%start startpoint;

startpoint:
    tree linebreaks_sp
    {
        driver.setOutput($1);
    }
|
    branch linebreaks_sp
    {
        driver.setOutput($1);
    }

tree:
   "[" name_item "]" linebreaks item_set tree_parallel
   {
       $$ = new DataMap();
       $$->insert("name", new DataValue($2));
       $$->insert("type", new DataValue("tree"));
       $$->insert("items", $5);

       DataArray* tempItem = new DataArray();
       tempItem->append($6);
       $$->insert("parts", tempItem);
   }

branch:
   "[" name_item "]" linebreaks item_set blossom_set
   {
       $$ = new DataMap();
       $$->insert("name", new DataValue($2));
       $$->insert("type", new DataValue("branch"));
       $$->insert("items", $5);
       $$->insert("parts", $6);
   }

blossom:
   "[" name_item "]" linebreaks static_options "-" "identifier" ":" linebreaks item_set
   {
       $$ = new DataMap();
       $$->insert("type", new DataValue("blossom"));
       $$->insert("name", new DataValue($2));
       $$->insert("static-options", $5);
       $$->insert("blossom-type", new DataValue($7));
       $$->insert("blossom-subtypes", new DataArray());
       $$->insert("items-input", $10);
   }
|
   "[" name_item "]" linebreaks static_options "-" "identifier" linebreaks "-" item_list ":" linebreaks item_set
   {
       $$ = new DataMap();
       $$->insert("type", new DataValue("blossom"));
       $$->insert("name", new DataValue($2));
       $$->insert("static-options", $5);
       $$->insert("blossom-type", new DataValue($7));
       $$->insert("blossom-subtypes", $10);
       $$->insert("items-input", $13);
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

static_options:
   %empty
   {
       $$ = new DataMap();
   }
|
   static_options static_option linebreaks
   {
       $1->insert($2.first, $2.second);
       $$ = $1;
   }
|
   static_option linebreaks
   {
       $$ = new DataMap();
       $$->insert($1.first, $1.second);
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

tree_subtree:


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
       $$ = new DataMap();
       $$->insert("type", new DataValue("sequentiell"));

       DataArray* tempItem = new DataArray();
       tempItem->append($1);

       $$->insert("parts", tempItem);
   }
|
   "[" linebreaks_sp tree_parallel linebreaks_sp "]"
   {
       $$ = new DataMap();
       $$->insert("type", new DataValue("sequentiell"));

       DataArray* tempItem = new DataArray();
       tempItem->append($3);

       $$->insert("parts", tempItem);
   }

tree_parallel:
   "{" linebreaks_sp tree_sequentiell linebreaks_sp "}"
   {
       $$ = new DataMap();
       $$->insert("type", new DataValue("parallel"));

       DataArray* tempItem = new DataArray();
       tempItem->append($3);

       $$->insert("parts", tempItem);
   }

tree_branch:
   "[" "TREE" ":" name_item "," "HOST" ":" name_item "]" linebreaks item_set
   {
       DataMap* tempItem = new DataMap();
       tempItem->insert("type", new DataValue("tree"));
       tempItem->insert("name", new DataValue(driver.removeQuotes($4)));
       tempItem->insert("host", new DataValue(driver.removeQuotes($8)));
       tempItem->insert("items-input", $11);
       $$ = tempItem;
   }
|
   "[" "BRANCH" ":" name_item "," "HOST" ":" name_item "]" linebreaks item_set
   {
       DataMap* tempItem = new DataMap();
       tempItem->insert("type", new DataValue("branch"));
       tempItem->insert("name", new DataValue(driver.removeQuotes($4)));
       tempItem->insert("host", new DataValue(driver.removeQuotes($8)));
       tempItem->insert("items-input", $11);
       $$ = tempItem;
   }
|
   "[" "TREE" ":" name_item "]" linebreaks item_set
   {
       DataMap* tempItem = new DataMap();
       tempItem->insert("type", new DataValue("tree"));
       tempItem->insert("name", new DataValue(driver.removeQuotes($4)));
       tempItem->insert("items-input", $7);
       $$ = tempItem;
   }
|
   "[" "BRANCH" ":" name_item "]" linebreaks item_set
   {
       DataMap* tempItem = new DataMap();
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

void Kitsunemimi::Sakura::SakuraParser::error(const Kitsunemimi::Sakura::location& location,
                                          const std::string& message)
{
    driver.error(location, message);
}
