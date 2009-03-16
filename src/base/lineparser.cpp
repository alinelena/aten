/*
	*** Line Parsing Routines
	*** src/base/lineparser.cpp
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

#include "base/lineparser.h"
#include "base/sysfunc.h"
#include "base/mathfunc.h"
#include "base/constants.h"
#include "base/messenger.h"
#include <string.h>

// Parse options
const char *ParseOptionKeywords[LineParser::nParseOptions] = { "defaults", "usequotes", "skipblanks", "stripbrackets", "noexpressions", "noescapes" };
LineParser::ParseOption LineParser::parseOption(const char *s)
{
	return (LineParser::ParseOption) (1 << enumSearch("line parser option", LineParser::nParseOptions, ParseOptionKeywords, s));
}

// Constructor
LineParser::LineParser()
{
	// Private variables
	endOfLine_ = FALSE;
	lineLength_ = 0;
	linePos_ = 0;
	optionMask_ = LineParser::Defaults;
	lastLine_ = 0;
}

// Returns number of arguments grabbed from last parse
int LineParser::nArgs()
{
	return arguments_.nItems();
}

// Returns the specified argument as a character string
const char *LineParser::argc(int i)
{
	if ((i < 0) || (i >= nArgs()))
	{
		printf("Warning: Argument %i is out of range - returning \"NULL\"...\n");
		return "NULL";
	}
	return arguments_[i]->get();
}

// Returns the specified argument as an integer
int LineParser::argi(int i)
{
	if ((i < 0) || (i >= nArgs()))
	{
		printf("Warning: Argument %i is out of range - returning 0...\n");
		return 0;
	}
	return arguments_[i]->asInteger();
}

// Returns the specified argument as a double
double LineParser::argd(int i)
{
	if ((i < 0) || (i >= nArgs()))
	{
		printf("Warning: Argument %i is out of range - returning 0.0...\n");
		return 0.0;
	}
	return arguments_[i]->asDouble();
}

// Returns the specified argument as a bool
bool LineParser::argb(int i)
{
	if ((i < 0) || (i >= nArgs()))
	{
		printf("Warning: Argument %i is out of range - returning FALSE...\n");
		return FALSE;
	}
	return arguments_[i]->asBool();
}

// Returns the specified argument as a float
float LineParser::argf(int i)
{
	if ((i < 0) || (i >= nArgs()))
	{
		printf("Warning: Argument %i is out of range - returning 0.0f...\n");
		return 0.0f;
	}
	return (float) argd(i);
}

// Returns whether the specified argument is empty
bool LineParser::isBlank(int i)
{
	if ((i < 0) || (i >= nArgs()))
	{
		printf("Warning: Argument %i is out of range (for isBlank) - returning FALSE...\n");
		return FALSE;
	}
	return (arguments_[i]->get()[0] == '\0' ? TRUE : FALSE);
}

// Set argument manually
//void LineParser::setArg(int i, const char *s)
//{
//	arguments_[i] = s;
//}

// Return pointer to current line
const char *LineParser::line()
{
	return line_;
}

// Return integer line number of last read line
int LineParser::lastLine()
{
	return lastLine_;
}

// Open file for parsing
bool LineParser::openFile(const char *filename)
{
	msg.enter("LineParser::openFile");
	// Check existing input file
	if (sourceFile_.is_open())
	{
		printf("Warning - LineParser already appears to have an open file...\n");
		sourceFile_.close();
	}
	// Open new file
	sourceFile_.open(filename, ios::in);
	if (!sourceFile_.good())
	{
		sourceFile_.close();
		msg.print("Failed to open file '%s'.\n", filename);
		msg.exit("LineParser::openFile");
		return FALSE;
	}
	// Reset variables
	lastLine_ = 0;
	msg.exit("LineParser::openFile");
	return TRUE;
}

// Close file 
void LineParser::closeFile()
{
	if (sourceFile_.is_open())
	{
		sourceFile_.close();
		lastLine_ = 0;
	}
	else printf("Warning: LineParser tried to close file again.\n");
}

// Return whether current file source is good for reading/writing
bool LineParser::isFileGood()
{
	return sourceFile_.good();
}

/*
// String parsing methods
*/

// Read single line from internal file source
int LineParser::readLine()
{
	msg.enter("LineParser::readLine");
	// Returns : 0=ok, 1=error, -1=eof
	sourceFile_.getline(line_, MAXLINELENGTH-1);
	if (sourceFile_.eof())
	{
		closeFile();
		msg.exit("LineParser::readLine");
		return -1;
	}
	if (sourceFile_.fail())
	{
		closeFile();
		msg.exit("LineParser::readLine");
		return 1;
	}
	lineLength_ = strlen(line_);
	linePos_ = 0;
	lastLine_ ++;
	//printf("Line = [%s], length = %i\n",line_,lineLength_);
	msg.exit("LineParser::readLine");
	return 0;
}

bool LineParser::getNextArg(Dnchar *destarg)
{
	// Get the next input chunk from the internal string and put into argument specified.
	msg.enter("LineParser::getNextArg");
	static int arglen, readresult;
	static bool done, hadquotes, expression, failed;
	static char c, quotechar, d;
	failed = FALSE;
	done = FALSE;
	hadquotes = FALSE;
	quotechar = '\0';
	expression = FALSE;
	endOfLine_ = FALSE;
	arglen = 0;
	while (linePos_ < lineLength_)
	{
		c = line_[linePos_];
		switch (c)
		{
			// Backslash - escape next character (read new line if its an EOL marker)
			// If we're inside quotes, keep backslash *and* next character
			case (92):
				d = line_[linePos_ + 1];
				if ((optionMask_&LineParser::NoEscapes) || (quotechar != '\0'))
				{
					tempArg_[arglen] = c;
					arglen ++;
					tempArg_[arglen] = d;
					arglen ++;
					linePos_ ++;
				}
				else if ((d == 10) || (d == 13))
				{
					// Next char is newline, so read another line from file if we have one
					if (sourceFile_ == NULL)
					{
						done = TRUE;
						endOfLine_ = TRUE;
					}
					else
					{
						// Read a new line
						readresult = readLine();
						if (readresult != 0)
						{
							failed = TRUE;
							done = TRUE;
						}
					}
				}
				else
				{
					tempArg_[arglen] = d;
					arglen ++;
					linePos_ ++;
				}
				break;
			// End of line markers
			case (10):	// Line feed (\n)
			case (13):	// Carriage Return
				done = TRUE;
				endOfLine_ = TRUE;
				break;
			// Delimiters
			// If we encounter one and arg length != 0 this signals the end of the argument.
			case (9):	// Horizontal Tab
			case (' '):	// Space
			case (','):	// Comma
				if ((quotechar != '\0') || expression)
				{
					tempArg_[arglen] = c;
					arglen ++;
				}
				else if (arglen != 0) done = TRUE;
				break;
			// Quote marks
			// If LineParser::UseQuotes, keep delimiters and other quote marks inside the quoted text.
			case (34):	// Double quotes
			case (39):	// Single quotes
				if (expression) break;
				if (!(optionMask_&LineParser::UseQuotes)) break;
				if (quotechar == '\0') quotechar = c;
				else if (quotechar == c)
				{
					quotechar = '\0';
					hadquotes = TRUE;
					done = TRUE;
				}
				else
				{
					tempArg_[arglen] = c;
					arglen ++;
				}
				break;
			// Brackets
			case ('('):	// Left parenthesis
			case (')'):	// Right parenthesis
				if ((optionMask_&LineParser::StripBrackets) && (!expression)) break;
				tempArg_[arglen] = c;
				arglen ++;
				break;
			// Comment markers
			case ('#'):	// "#" Rest/all of line is a comment
				endOfLine_ = TRUE;
				done = TRUE;
				break;
			// Normal character
			default: 
				tempArg_[arglen] = c;
				arglen ++;
				break;
		}
		// Increment line position
		linePos_++;
		if (done || failed) break;
	}
	// Finalise argument
	tempArg_[arglen] = '\0';
	if (linePos_ == lineLength_) endOfLine_ = TRUE;
	// Store the result in the desired destination
	if (destarg != NULL) *destarg = tempArg_;
	msg.exit("LineParser::getNextArg");
	if (failed) return FALSE;
	return (arglen == 0 ? (hadquotes ? TRUE : FALSE) : TRUE);
}

// Rip next n characters
bool LineParser::getNextN(int length)
{
	// Put the next 'length' characters from line_ into temparg.
	msg.enter("LineParser::getNextN");
	int arglen = 0;
	char c;
	if (lineLength_ == 0)
	{
		msg.exit("LineParser::getNextN");
		return FALSE;
	}
	int charsleft = lineLength_ - linePos_;
	if (length > charsleft) length = charsleft;
	//if (length > lineLength_) length = lineLength_;
	for (int n=0; n<length; n++)
	{
		c = line_[linePos_];
		switch (c)
		{
			// Brackets
			case ('('):	// Left parenthesis
			case (')'):	// Right parenthesis
				if (optionMask_&LineParser::StripBrackets) break;
				tempArg_[arglen] = c;
				arglen ++;
				break;
		//	case (32):      // Space - ignore to get left-justified data
		//		break;
			default:
				tempArg_[arglen] = c;
				arglen ++;
				break;
		}
		linePos_ ++;
	}
	// Add terminating character to temparg
	tempArg_[arglen] = '\0';
	//printf("getNextN found [%s], length = %i\n", tempArg_, arglen);
	//line_.eraseStart(length);
	msg.exit("LineParser::getNextN");
	return TRUE;
}

// Get all arguments (delimited) from LineParser::line_
void LineParser::getAllArgsDelim()
{
	// Parse the string in 'line_' into arguments in 'args'
	msg.enter("LineParser::getAllArgsDelim");
	arguments_.clear();
	endOfLine_ = FALSE;
	Dnchar *arg;
	while (!endOfLine_)
	{
		// Create new, empty dnchar
		arg = new Dnchar;
		if (getNextArg(arg))
		{
			msg.print(Messenger::Parse,"getAllArgsDelim arg=%i [%s]\n", arguments_.nItems(), arg->get());
			// Add this char to the list
			arguments_.own(arg);
		}
		else delete arg;
	}
	msg.exit("LineParser::getAllArgsDelim");
}

/*
// Delimited Parsing Routines
*/

// Parse delimited (from file)
int LineParser::getArgsDelim(int options)
{
	// Standard file parse routine.
	// Splits the line from the file into delimited arguments via the 'parseline' function
	msg.enter("LineParser::getArgsDelim[ifstream]");
	bool done = FALSE;
	int result;
	// Lines beginning with '#' are ignored as comments
	// Blank lines are skipped if blankskip == TRUE.
	// Returns : 0=ok, 1=error, -1=eof
	optionMask_ = options;
	do
	{
		// Read line from file and parse it
		result = readLine();
		if (result != 0)
		{
			msg.exit("LineParser::getArgsDelim[ifstream]");
			return result;
		}
		// Assume that we will finish after parsing the line we just read in
		done = TRUE;
		// To check for blank lines, do the parsing and then check nargs()
		getAllArgsDelim();
		if ((optionMask_&LineParser::SkipBlanks) && (nArgs() == 0)) done = FALSE;
	} while (!done);
	msg.exit("LineParser::getArgsDelim[ifstream]");
	return 0;
}

// Get next argument (delimited) from file stream
const char *LineParser::getArgDelim(int flags)
{
	msg.enter("LineParser::getArgDelim");
	optionMask_ = flags;
	if (getNextArg(NULL)) msg.print(Messenger::Parse,"getArgDelim [%s]\n", tempArg_);
	msg.exit("LineParser::getArgDelim");
	return tempArg_;
}

// Parse supplied line according to format
int LineParser::getArgsFormatted(const char *s, NuFormat *format, int flags, bool usecurrentline)
{
	if (!usecurrentline) strcpy(line_,s);
	for (FormatChunk *chunk = format->chunks(); chunk != NULL; chunk = chunk->next)
	{
	}
}

// Read line and parse according to format
int LineParser::getArgsFormatted(NuFormat *format, int flags)
{
	
}

// Parse all arguments (delimited) from string
void LineParser::getArgsDelim(const char *s, int options)
{
	strcpy(line_,s);
	lineLength_ = strlen(line_);
	linePos_ = 0;
	optionMask_ = options;
	getAllArgsDelim();
}

// Return a number of characters from the input stream
const char *LineParser::getChars(int nchars)
{
	// Check number of characters requested
	if (nchars == 0) return NULL;
	else if (nchars > MAXLINELENGTH)
	{
		msg.print("Error: The maximum number of characters read at once from a file is currently %i.\n", MAXLINELENGTH);
		return NULL;
	}
	else if (nchars < 0)
	{
		tempArg_[0]   = '\0';
		for (int i=nchars; i<0; i++) sourceFile_.unget();
	}
	else sourceFile_.getline(tempArg_, nchars);
	if (sourceFile_.eof())
	{
		closeFile();
		msg.exit("LineParser::readLine");
		return NULL;
	}
	if (sourceFile_.fail())
	{
		closeFile();
		msg.exit("LineParser::readLine");
		return NULL;
	}
	return tempArg_;
}

// Return an integer value from reading 'n' chars of an (unformatted) input file
int LineParser::getInteger(int nbytes)
{
	// Use default size if none specified
	if (nbytes == 0)
	{
		int readi;
		sourceFile_.read((char*) &readi, sizeof(int));
		return readi;
	}
	// Try and select a suitable type for the read, based on nbytes
	if (sizeof(short int) == nbytes)
	{
		short int readi;
		sourceFile_.read((char*) &readi, nbytes);
		return (int) readi;
	}
	else if (sizeof(int) == nbytes)
	{
		int readi;
		sourceFile_.read((char*) &readi, nbytes);
		return readi;
	}
	else if (sizeof(long int) == nbytes)
	{
		long int readi;
		sourceFile_.read((char*) &readi, nbytes);
		return (int) readi;
	}
	else msg.print("Error: Integer of size %i bytes does not correspond to any internal type.\n", nbytes);
	return 0;
}

// Return a double value from reading 'n' chars of an (unformatted) input file
double LineParser::getReal(int nbytes)
{
	// Use default size if none specified
	if (nbytes == 0)
	{
		double readd;
		sourceFile_.read((char*) &readd, sizeof(double));
		return readd;
	}
	// Try and select a suitable type for the read, based on nbytes
	if (sizeof(double) == nbytes)
	{
		double readd;
		sourceFile_.read((char*) &readd, nbytes);
		return readd;
	}
	else if (sizeof(long double) == nbytes)
	{
		long double readd;
		sourceFile_.read((char*) &readd, nbytes);
		return (double) readd;
	}
	else msg.print("Error: Real of size %i bytes does not correspond to any internal type.\n", nbytes);
	return 0.0;
}

// Skip lines from file
int LineParser::skipLines(int nlines)
{
	msg.enter("LineParser::skipLines");
	int result;
	for (int n=0; n<nlines; n++)
	{
		result = readLine();
		if (result != 0)
		{
			msg.exit("LineParser::skipLines");
			return result;
		}
	}
	msg.exit("LineParser::skipLines");
	return 0;
}

/*
// Atom type definition functions
*/

const char *LineParser::parseAtomtypeString(Dnchar &source)
{
	// Cut the next atomtype command from the supplied string. Put in 'dest', along with any bracketed
	// argument part. Use brackets a bit like quotes are used above, except we don't toggle the flag.
	// Ignore spaces and horizontal tabs. Commas separate commands.
	msg.enter("LineParser::parseAtomtypeString");
	static int n, nchars, bracketlevel;
	static bool done, el_list;
	static Dnchar typecmd;
	nchars = 0;
	bracketlevel = 0;
	el_list = FALSE;
	done = FALSE;
	typecmd.createEmpty(source);
	//source.print();
	for (n=0; n<source.length(); n++)
	{
		switch (source[n])
		{
			case (32):	// Space
				break;
			case (10):	// Line feed (\n) - Signals end of the keyword.
				done = TRUE;
				break;
			case (13):	// Carriage Return - Signals end of the keyword.
				done = TRUE;
				break;
			case (44):	// Comma - end of keyword or separator in element list
				if (bracketlevel == 0 && !el_list) done = TRUE;
				else typecmd += ',';
				break;
			case (9):	// Horizontal Tab
				break;
			case (40):	// "(" - Signals beginning of options list
				bracketlevel ++;
				typecmd += '(';
				break;
			case (41):	// ")" - Signals end of options list
				bracketlevel --;
				typecmd += ')';
				break;
			case (91):	// "[" - Signals beginning of element list
				el_list = TRUE;
				// If we're inside brackets, *don't* remove it
				if (bracketlevel != 0) typecmd += source[n];
				break;
			case (93):	// "]" - Signals end of element list
				el_list = FALSE;
				// If we're inside brackets, *don't* remove it
				if (bracketlevel != 0) typecmd += source[n];
				break;
			default: 
				typecmd += source[n];
				break;
		}
		if (done) break;
	}
	// Strip off the keyword characters from the string.
	//printf("Erasing in parseAtomtypeString...\n");
	// ERASE now takes 'start' and 'end' (not 'count') as parameters.
	source.eraseStart(n+1);
	//printf("Result = ");
	//typecmd.print();
	msg.exit("LineParser::parseAtomtypeString");
	return typecmd.get();
}

const char *LineParser::trimAtomtypeKeyword(Dnchar &source)
{
	// Remove the keyword part of the command and put in 'dest', leaving the options (minus brackets)
	// in the original string. Remove '[' and ']' from keyword since this is only used to keep a list of elements together.
	msg.enter("LineParser::trimAtomtypeKeyword");
	static bool done, equals;
	static Dnchar keywd;
	done = FALSE;
	equals = FALSE;
	//printf("String given to trimAtomKeyword = '%s'\n",source.get());
	keywd.createEmpty(source);
	int n;
	for (n=0; n<source.length(); n++)
	{
		switch (source[n])
		{
			case (40):	// "(" - if present signals start of options
				done = TRUE;
				break;
			case (10):	// Line feed (\n) - Signals end of the keyword.
				done = TRUE;
				break;
			case (13):	// Carriage Return - Signals end of the keyword.
				done = TRUE;
				break;
			case (61):	// "=" - Signals keyword/single option delimiter (unless first character)
				if (n == 0) keywd += source[n];
				else
				{
					done = TRUE;
					equals = TRUE;
				}
				break;
			case (91):	// '['
			case (93):	// ']'
				break;
			default: 
				keywd += source[n]; 
				break;
		}
		if (done) break;
	}
	// Trim off the keyword part
	if (equals) source.eraseStart(n+1);
	else source.eraseStart(n);
	//printf("RESULT in trim atkeywd = '%s'\n",dest.get());
	// Remove brackets if they're there
	if (source.length() != 0) 
		if (source[0] == '(')
		{
			source.eraseStart(1);
			source.eraseEnd(1);
		}
	msg.exit("LineParser::trimAtomtypeKeyword");
	return keywd.get();
}