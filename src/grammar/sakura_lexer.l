%{ /* -*- C++ -*- */
# include <cerrno>
# include <climits>
# include <cstdlib>
# include <string>
# include <sakura_parsing/sakura_parser_interface.hpp>
# include <sakura_parser.hpp>

# undef yywrap
# define yywrap() 1


// The location of the current token.
# ifdef YY_DECL
# undef YY_DECL
# endif
# define YY_DECL \
    Kitsune::Sakura::SakuraParser::symbol_type sakuralex (Kitsune::Sakura::SakuraParserInterface& driver)
YY_DECL;
Kitsune::Sakura::location sakuraloc;
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
[\n]        return Kitsune::Sakura::SakuraParser::make_LINEBREAK (sakuraloc);
"FOREST"    return Kitsune::Sakura::SakuraParser::make_FOREST (sakuraloc);
"TREE"      return Kitsune::Sakura::SakuraParser::make_TREE (sakuraloc);
"BRANCH"    return Kitsune::Sakura::SakuraParser::make_BRANCH (sakuraloc);
"-"         return Kitsune::Sakura::SakuraParser::make_MINUS (sakuraloc);
"["         return Kitsune::Sakura::SakuraParser::make_LBRACK (sakuraloc);
"]"         return Kitsune::Sakura::SakuraParser::make_RBRACK (sakuraloc);
"{"         return Kitsune::Sakura::SakuraParser::make_LBRACKBOW (sakuraloc);
"}"         return Kitsune::Sakura::SakuraParser::make_RBRACKBOW (sakuraloc);
":"         return Kitsune::Sakura::SakuraParser::make_ASSIGN (sakuraloc);
"."         return Kitsune::Sakura::SakuraParser::make_DOT (sakuraloc);
","         return Kitsune::Sakura::SakuraParser::make_COMMA (sakuraloc);

\"(\$\{.*\}|\\.|[^\"\\])*\" {
    return Kitsune::Sakura::SakuraParser::make_STRING(yytext, sakuraloc);
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
    return Kitsune::Sakura::SakuraParser::make_NUMBER(length, sakuraloc);
}

{int}+"."{int}*	{
    float value = atof( yytext );
    return Kitsune::Sakura::SakuraParser::make_FLOAT(value, sakuraloc);
}

{id}       return Kitsune::Sakura::SakuraParser::make_IDENTIFIER(yytext, sakuraloc);
.          driver.error(sakuraloc, "invalid character");
<<EOF>>    return Kitsune::Sakura::SakuraParser::make_END(sakuraloc);

%%


void Kitsune::Sakura::SakuraParserInterface::scan_begin(const std::string &inputString)
{
    Kitsune::Sakura::location newSakuraloc;
    sakuraloc = newSakuraloc;
    yy_flex_debug = m_traceParsing;
    yy_scan_string(inputString.c_str());
}

void Kitsune::Sakura::SakuraParserInterface::scan_end()
{
    yy_delete_buffer(YY_CURRENT_BUFFER);
}


