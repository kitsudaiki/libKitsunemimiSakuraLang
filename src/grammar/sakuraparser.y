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
#include <jsonObjects.h>

namespace Kitsune
{
namespace Sakura
{

class SakuraParserInterface;
using namespace Kitsune::Json;

}  // namespace Sakura
}  // namespace Kitsune
}

// The parsing context.
%param { Kitsune::Sakura::SakuraParserInterface& driver }

%locations

%code
{
#include <sakura_parsing/sakuraParserInterface.h>
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

%type  <JsonObject*> blossom
%type  <JsonArray*> blossom_set

%type  <std::pair<std::string, AbstractJson*>> item
%type  <JsonObject*> item_set
%type  <JsonArray*> item_list
%type  <JsonArray*> forest_parts
%type  <JsonObject*> forest_part

%type  <std::pair<std::string, AbstractJson*>> setting
%type  <JsonObject*> setting_set

%type  <JsonObject*> branch
%type  <JsonObject*> tree
%type  <JsonObject*> forest

%type  <JsonObject*> tree_branch
%type  <JsonObject*> tree_sequentiell
%type  <JsonObject*> tree_parallel

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
       $$ = new JsonObject();
       $$->insert("name", new JsonValue($4));
       $$->insert("type", new JsonValue("forest"));
       $$->insert("items", $7);
       $$->insert("parts", $8);
   }

tree:
   "[" "TREE" ":" name_item "]" linebreaks item_set tree_parallel
   {
       $$ = new JsonObject();
       $$->insert("name", new JsonValue($4));
       $$->insert("type", new JsonValue("tree"));
       $$->insert("items", $7);

       JsonArray* tempItem = new JsonArray();
       tempItem->append($8);
       $$->insert("parts", tempItem);
   }

branch:
   "[" "BRANCH" ":" name_item "]" linebreaks item_set blossom_set
   {
       $$ = new JsonObject();
       $$->insert("name", new JsonValue($4));
       $$->insert("type", new JsonValue("branch"));
       $$->insert("items", $7);
       $$->insert("parts", $8);
   }

blossom:
   "[" name_item "]" linebreaks setting_set "-" "identifier" ":" linebreaks item_set
   {
       $$ = new JsonObject();
       $$->insert("type", new JsonValue("blossom"));
       $$->insert("name", new JsonValue($2));
       $$->insert("common-settings", $5);
       $$->insert("blossom-type", new JsonValue($7));
       $$->insert("blossom-subtypes", new JsonArray());
       $$->insert("items-input", $10);
   }
|
   "[" name_item "]" linebreaks setting_set "-" "identifier" linebreaks "-" item_list ":" linebreaks item_set
   {
       $$ = new JsonObject();
       $$->insert("type", new JsonValue("blossom"));
       $$->insert("name", new JsonValue($2));
       $$->insert("common-settings", $5);
       $$->insert("blossom-type", new JsonValue($7));
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
       $$ = new JsonArray();
       $$->append($1);
   }

forest_part:
   "[" name_item "]" linebreaks setting_set "-" "TREE" name_item ":" linebreaks item_set
   {
       $$ = new JsonObject();
       $$->insert("type", new JsonValue("forest"));
       $$->insert("name", new JsonValue($2));
       $$->insert("common-settings", $5);
       $$->insert("item-name", new JsonValue($8));
       $$->insert("item-type", new JsonValue("tree"));
       $$->insert("items-input", $11);
   }
|
   "[" name_item "]" linebreaks setting_set "-" "BRANCH" name_item ":" linebreaks item_set
   {
       $$ = new JsonObject();
       $$->insert("type", new JsonValue("forest"));
       $$->insert("name", new JsonValue($2));
       $$->insert("common-settings", $5);
       $$->insert("item-name", new JsonValue($8));
       $$->insert("item-type", new JsonValue("branch"));
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
       $$ = new JsonArray();
       $$->append($1);
   }

setting_set:
   %empty
   {
       $$ = new JsonObject();
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
       $$ = new JsonObject();
       $$->insert($1.first, $1.second);
   }

setting:
   "identifier" ":" "identifier"
   {
       std::pair<std::string, AbstractJson*> tempItem;
       tempItem.first = $1;
       tempItem.second = new JsonValue($3);
       $$ = tempItem;
   }
|
   "identifier" ":" "string"
   {
       std::pair<std::string, AbstractJson*> tempItem;
       tempItem.first = $1;
       tempItem.second = new JsonValue(driver.removeQuotes($3));
       $$ = tempItem;
   }
|
   "identifier" ":" "number"
   {
       std::pair<std::string, AbstractJson*> tempItem;
       tempItem.first = $1;
       tempItem.second = new JsonValue($3);
       $$ = tempItem;
   }
|
   "identifier" ":" "float"
   {
       std::pair<std::string, AbstractJson*> tempItem;
       tempItem.first = $1;
       tempItem.second = new JsonValue($3);
       $$ = tempItem;
   }

item_set:
   %empty
   {
       $$ = new JsonObject();
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
       $$ = new JsonObject();
       $$->insert($1.first, $1.second);
   }

item:
   "identifier" ":" "{" "{" "}" "}"
   {
       // uset value
       std::string empty = "{{}}";
       std::pair<std::string, AbstractJson*> tempItem;
       tempItem.first = $1;
       tempItem.second = new JsonValue(empty);
       $$ = tempItem;
   }
|
   "identifier" ":" "identifier"
   {
       std::pair<std::string, AbstractJson*> tempItem;
       tempItem.first = $1;
       tempItem.second = new JsonValue($3);
       $$ = tempItem;
   }
|
   "identifier" ":" "string"
   {
       std::pair<std::string, AbstractJson*> tempItem;
       tempItem.first = $1;
       tempItem.second = new JsonValue(driver.removeQuotes($3));
       $$ = tempItem;
   }
|
   "identifier" ":" "number"
   {
       std::pair<std::string, AbstractJson*> tempItem;
       tempItem.first = $1;
       tempItem.second = new JsonValue($3);
       $$ = tempItem;
   }
|
   "identifier" ":" "float"
   {
       std::pair<std::string, AbstractJson*> tempItem;
       tempItem.first = $1;
       tempItem.second = new JsonValue($3);
       $$ = tempItem;
   }
|
   "identifier" ":" item_list
   {
       std::pair<std::string, AbstractJson*> tempItem;
       tempItem.first = $1;
       tempItem.second = $3;
       $$ = tempItem;
   }

item_list:
   item_list "," "identifier"
   {
       $1->append(new JsonValue($3));
       $$ = $1;
   }
|
   item_list "identifier"
   {
       $1->append(new JsonValue($2));
       $$ = $1;
   }
|
   "identifier" "identifier"
   {
       $$ = new JsonArray();
       $$->append(new JsonValue($1));
       $$->append(new JsonValue($2));
   }
|
   "identifier" "," "identifier"
   {
       $$ = new JsonArray();
       $$->append(new JsonValue($1));
       $$->append(new JsonValue($3));
   }
|
   "identifier"
   {
       $$ = new JsonArray();
       $$->append(new JsonValue($1));
   }

tree_sequentiell:
   tree_sequentiell linebreaks_sp tree_branch
   {
       JsonArray* array = (JsonArray*)$1->get("parts");
       array->append($3);
       $$ = $1;
   }
|
   tree_sequentiell linebreaks_sp "[" linebreaks_sp tree_parallel linebreaks_sp "]"
   {
       JsonArray* array = (JsonArray*)$1->get("parts");
       array->append($5);
       $$ = $1;
   }
|
   tree_branch
   {
       $$ = new JsonObject();
       $$->insert("type", new JsonValue("sequentiell"));

       JsonArray* tempItem = new JsonArray();
       tempItem->append($1);

       $$->insert("parts", tempItem);
   }
|
   "[" linebreaks_sp tree_parallel linebreaks_sp "]"
   {
       $$ = new JsonObject();
       $$->insert("type", new JsonValue("sequentiell"));

       JsonArray* tempItem = new JsonArray();
       tempItem->append($3);

       $$->insert("parts", tempItem);
   }

tree_parallel:
   "{" linebreaks_sp tree_sequentiell linebreaks_sp "}"
   {
       $$ = new JsonObject();
       $$->insert("type", new JsonValue("parallel"));

       JsonArray* tempItem = new JsonArray();
       tempItem->append($3);

       $$->insert("parts", tempItem);
   }

tree_branch:
   "[" "BRANCH" ":" name_item "]" linebreaks item_set
   {
       JsonObject* tempItem = new JsonObject();
       tempItem->insert("type", new JsonValue("branch"));
       tempItem->insert("name", new JsonValue(driver.removeQuotes($4)));
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
