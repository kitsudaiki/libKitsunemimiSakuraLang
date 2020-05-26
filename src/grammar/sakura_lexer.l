%{ /* -*- C++ -*- */
/**
 * @file        sakura_lexer.l
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

# include <cerrno>
# include <climits>
# include <cstdlib>
# include <string>
# include <sakura_parsing/sakura_parser_interface.h>
# include <sakura_parser.h>

# undef yywrap
# define yywrap() 1

# ifdef YY_DECL
# undef YY_DECL
# endif
# define YY_DECL \
    Kitsunemimi::Sakura::SakuraParser::symbol_type sakuralex (Kitsunemimi::Sakura::SakuraParserInterface& driver)
YY_DECL;
Kitsunemimi::Sakura::location sakuraloc;
%}


%option noyywrap nounput batch debug yylineno prefix="sakura"

id    [a-zA-Z][a-zA-Z_0-9]*
long   -?([0-9]+)
blank [ \t]

%{
    # define YY_USER_ACTION  sakuraloc.columns (yyleng);
%}

%%

%{
    sakuraloc.step();
%}

#.*$            sakuraloc.step();
{blank}+        sakuraloc.step();
[\n]            { sakuraloc.lines(1); sakuraloc.step(); }
"true"          return Kitsunemimi::Sakura::SakuraParser::make_BOOL_TRUE (sakuraloc);
"false"         return Kitsunemimi::Sakura::SakuraParser::make_BOOL_FALSE (sakuraloc);
"seed"          return Kitsunemimi::Sakura::SakuraParser::make_SEED (sakuraloc);
"subtree"       return Kitsunemimi::Sakura::SakuraParser::make_SUBTREE (sakuraloc);
"parallel_for"  return Kitsunemimi::Sakura::SakuraParser::make_PARALLEL_FOR (sakuraloc);
"parallel"      return Kitsunemimi::Sakura::SakuraParser::make_PARALLEL (sakuraloc);
"if"            return Kitsunemimi::Sakura::SakuraParser::make_IF (sakuraloc);
"else"          return Kitsunemimi::Sakura::SakuraParser::make_ELSE (sakuraloc);
"for"           return Kitsunemimi::Sakura::SakuraParser::make_FOR (sakuraloc);
"->"            return Kitsunemimi::Sakura::SakuraParser::make_ARROW (sakuraloc);
"=="            return Kitsunemimi::Sakura::SakuraParser::make_EQUAL_COMPARE (sakuraloc);
"!="            return Kitsunemimi::Sakura::SakuraParser::make_UNEQUAL_COMPARE (sakuraloc);
">="            return Kitsunemimi::Sakura::SakuraParser::make_GREATER_EQUAL_COMPARE (sakuraloc);
"<="            return Kitsunemimi::Sakura::SakuraParser::make_SMALLER_EQUAL_COMPARE (sakuraloc);
"<<"            return Kitsunemimi::Sakura::SakuraParser::make_SHIFT_LEFT (sakuraloc);
">>"            return Kitsunemimi::Sakura::SakuraParser::make_SHIFT_RIGHT (sakuraloc);
">"             return Kitsunemimi::Sakura::SakuraParser::make_GREATER_COMPARE (sakuraloc);
"<"             return Kitsunemimi::Sakura::SakuraParser::make_SMALLER_COMPARE (sakuraloc);
"-"             return Kitsunemimi::Sakura::SakuraParser::make_MINUS (sakuraloc);
"+"             return Kitsunemimi::Sakura::SakuraParser::make_PLUS (sakuraloc);
"="             return Kitsunemimi::Sakura::SakuraParser::make_EQUAL (sakuraloc);
"("             return Kitsunemimi::Sakura::SakuraParser::make_LROUNDBRACK (sakuraloc);
")"             return Kitsunemimi::Sakura::SakuraParser::make_RROUNDBRACK (sakuraloc);
"["             return Kitsunemimi::Sakura::SakuraParser::make_LBRACK (sakuraloc);
"]"             return Kitsunemimi::Sakura::SakuraParser::make_RBRACK (sakuraloc);
"{"             return Kitsunemimi::Sakura::SakuraParser::make_LBRACKBOW (sakuraloc);
"}"             return Kitsunemimi::Sakura::SakuraParser::make_RBRACKBOW (sakuraloc);
":"             return Kitsunemimi::Sakura::SakuraParser::make_ASSIGN (sakuraloc);
";"             return Kitsunemimi::Sakura::SakuraParser::make_SEMICOLON (sakuraloc);
"."             return Kitsunemimi::Sakura::SakuraParser::make_DOT (sakuraloc);
","             return Kitsunemimi::Sakura::SakuraParser::make_COMMA (sakuraloc);

\"(\$\{.*\}|\\.|[^\"\\])*\" {
    return Kitsunemimi::Sakura::SakuraParser::make_STRING(yytext, sakuraloc);
}

{long}      {
    errno = 0;
    long length = strtol(yytext, NULL, 10);
    if (!(LONG_MIN <= length
        && length <= LONG_MAX
        && errno != ERANGE))
    {
        driver.error(sakuraloc, "integer is out of range");
    }
    return Kitsunemimi::Sakura::SakuraParser::make_NUMBER(length, sakuraloc);
}

{long}+"."{long}*	{
    double value = strtod( yytext , NULL );
    return Kitsunemimi::Sakura::SakuraParser::make_FLOAT(value, sakuraloc);
}

{id}       return Kitsunemimi::Sakura::SakuraParser::make_IDENTIFIER(yytext, sakuraloc);

[a-zA-Z_0-9]* {
    return Kitsunemimi::Sakura::SakuraParser::make_STRING_PLN(yytext, sakuraloc);
}

.          driver.error(sakuraloc, "invalid character");
<<EOF>>    return Kitsunemimi::Sakura::SakuraParser::make_END(sakuraloc);

%%


void Kitsunemimi::Sakura::SakuraParserInterface::scan_begin(const std::string &inputString)
{
    Kitsunemimi::Sakura::location newSakuraloc;
    sakuraloc = newSakuraloc;
    yy_flex_debug = m_traceParsing;
    yy_scan_string(inputString.c_str());
}

void Kitsunemimi::Sakura::SakuraParserInterface::scan_end()
{
    yy_delete_buffer(YY_CURRENT_BUFFER);
}


