/*
	*** Lexical Analyzer
	*** src/parser/lexer.cpp
	Copyright T. Youngs 2007-2009

	This file is part of Aten.

	Aten is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Aten is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Aten.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "parser/tree.h"
#include "parser/treenode.h"
#include "parser/grammar.h"
#include "parser/integer.h"
#include "parser/character.h"
#include "parser/real.h"
#include "base/sysfunc.h"
#include <ctype.h>
#include <string.h>

// Lexical Analyser - Used the getchar function of the current active Tree (stored in static member Tree::currentTree)
int yylex()
{
	if (Tree::currentTree == NULL)
	{
		printf("Lexer called when no current tree pointer available.\n");
		return 0;
	}

	int c, length, n;
	bool done, integer, hasexp;
	static char token[256];
	length = 0;
	token[0] = '\0';

	// Skip over whitespace
	while ((c = Tree::currentTree->getChar()) == ' ' || c == '\t');

	if (c == EOF) return 0;

	/*
	// Number Detection - Either '.' or  a digit begins a number
	*/
	if (c == '.' || isdigit (c))
	{
		integer = TRUE;
		hasexp = FALSE;
		token[length++] = c;
		done = FALSE;
		do
		{
			c = Tree::currentTree->getChar();
			if (isdigit(c)) token[length++] = c;
			else if (c == '.')
			{
				integer = FALSE;
				token[length++] = '.';
			}
			else if ((c == 'e') || (c == 'E'))
			{
				// Check for previous exponential in number
				if (hasexp)
				{
					msg.print("Parse error: Number has two exponentiations (e/E).\n");
					return 0;
				}
				hasexp = TRUE;
				token[length++] = 'E';
			}
			else if ((c == '-') || (c == '+'))
			{
				// We allow '-' or '+' only as part of an exponentiation, so if it is not preceeded by 'E' we stop parsing
				if (token[length-1] != 'E')
				{
					Tree::currentTree->unGetChar();
					token[length] = '\0';
					done = TRUE;
				}
				else token[length++] = c;
			}
			else
			{
				Tree::currentTree->unGetChar();
				token[length] = '\0';
				done = TRUE;
			}
		} while (!done);
		// We now have the number, so create a constant and return it
		if (integer)
		{
			NuIntegerVariable *var = new NuIntegerVariable(atoi(token), TRUE);
			yylval.node = var;
			return INTCONST;
		}
		else
		{
			NuRealVariable *var = new NuRealVariable(atof(token), TRUE);
			yylval.node = var;
			return REALCONST;
		}
	}

	/*
	// Literal Character String - surrounded by ""
	*/
	if (c == '"')
	{
		// Just read everything until we find another '"'
		done = FALSE;
		do
		{
			c = Tree::currentTree->getChar();
			if (c == '"')
			{
				// Check for null string...
				if (length == 0) done = TRUE;
				else if (token[length-1] == '\\') token[length++] = '"';
				else done = TRUE;
			}
			else token[length++] = c;
		} while (!done);
		token[length] = '\0';
		NuCharacterVariable *var = new NuCharacterVariable(token, TRUE);
		yylval.node = var;
		return CHARCONST;
	}

	/* Char starts an identifier => read the name.	*/
	if (isalpha (c))
	{
		done = FALSE;
		do
		{
			/* Add this character to the buffer.	  */
			token[length++] = c;
			/* Get another character.			  */
			c = Tree::currentTree->getChar();
		}
		while (isalnum (c));
		Tree::currentTree->unGetChar();
		token[length] = '\0';

		// Is this a recognised high-level keyword?
		if (strcmp(token,"integer") == 0) return INTEGER;
		else if (strcmp(token,"real") == 0) return REAL;
		else if (strcmp(token,"character") == 0) return CHARACTER;

		// If we get to here then its not a high-level keyword.
		// Is it a function keyword?
		for (n=0; n<NuCommand::nFunctions; n++) if (strcmp(token,NuCommand::data[n].keyword) == 0) break;
		if (n != NuCommand::nFunctions)
		{
			printf("Command is [%s]\n", token);
			yylval.functionId = n;
			return FUNCTION;
		}

		// The token isn't a high- or low-level function.
		// Is it a variable? Search the lists currently in scope...
		NuVariable *v = Tree::currentTree->isVariableInScope(token);
		
		printf("Lexer found an unknown token name = [%s]\n", token);
		
	}

	/* Any other character is a token by itself.	 */
	return c;
}

