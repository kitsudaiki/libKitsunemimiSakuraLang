%{ /* -*- C++ -*- */
# include <cerrno>
# include <climits>
# include <cstdlib>
# include <string>
# include <sakura_parsing/sakura_parser_interface.h>
# include <sakura_parser.h>

# undef yywrap
# define yywrap() 1


// The location of the current token.
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
int   -?([0-9]+)
blank [ \t]

%{
    # define YY_USER_ACTION  sakuraloc.columns (yyleng);
%}

%%

%{
    sakuraloc.step();
%}

{blank}+    sakuraloc.step();
[\n]        return Kitsunemimi::Sakura::SakuraParser::make_LINEBREAK (sakuraloc);
"HOST"      return Kitsunemimi::Sakura::SakuraParser::make_HOST (sakuraloc);
"TREE"      return Kitsunemimi::Sakura::SakuraParser::make_TREE (sakuraloc);
"BRANCH"    return Kitsunemimi::Sakura::SakuraParser::make_BRANCH (sakuraloc);
"-"         return Kitsunemimi::Sakura::SakuraParser::make_MINUS (sakuraloc);
"["         return Kitsunemimi::Sakura::SakuraParser::make_LBRACK (sakuraloc);
"]"         return Kitsunemimi::Sakura::SakuraParser::make_RBRACK (sakuraloc);
"{"         return Kitsunemimi::Sakura::SakuraParser::make_LBRACKBOW (sakuraloc);
"}"         return Kitsunemimi::Sakura::SakuraParser::make_RBRACKBOW (sakuraloc);
":"         return Kitsunemimi::Sakura::SakuraParser::make_ASSIGN (sakuraloc);
"."         return Kitsunemimi::Sakura::SakuraParser::make_DOT (sakuraloc);
","         return Kitsunemimi::Sakura::SakuraParser::make_COMMA (sakuraloc);

\"(\$\{.*\}|\\.|[^\"\\])*\" {
    return Kitsunemimi::Sakura::SakuraParser::make_STRING(yytext, sakuraloc);
}

{int}      {
    errno = 0;
    long length = strtol(yytext, NULL, 10);
    if (!(INT_MIN <= length
        && length <= INT_MAX
        && errno != ERANGE))
    {
        driver.error(sakuraloc, "integer is out of range");
    }
    return Kitsunemimi::Sakura::SakuraParser::make_NUMBER(length, sakuraloc);
}

{int}+"."{int}*	{
    float value = atof( yytext );
    return Kitsunemimi::Sakura::SakuraParser::make_FLOAT(value, sakuraloc);
}

{id}       return Kitsunemimi::Sakura::SakuraParser::make_IDENTIFIER(yytext, sakuraloc);
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


