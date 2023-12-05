// Ryan Weisman
// COP 3402 Spring 2023

// All libraries included
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// defines the maximum table size and maximum code size
#define MAX_SYMBOL_TABLE_SIZE 500
#define CODE_SIZE 500

// Data structures for symbol table and assembly code array
typedef struct
{
  int kind;
  char name[12];
  int val;
  int level;
  int addr;
  int mark;
} symbol;

typedef struct
{
  char OP[4];
  int L;
  int M;
} code;

// Initializes arrays for the assembly code (text) and the symbol table
symbol symbolTable[MAX_SYMBOL_TABLE_SIZE];
code text[CODE_SIZE];

// These global variables are used to keep uniform indexes throughout the entire program
int lexLevel = -1;
int lexIndex = 0;
int tableIndex = 0;
int cx = 0;
char token[12] = "";

// An enumerator for all symbols
typedef enum
{
  skipsym = 1,
  identsym,
  numbersym,
  plussym,
  minussym,
  multsym,
  slashsym,
  oddsym,
  eqlsym,
  neqsym,
  lessym,
  leqsym,
  gtrsym,
  geqsym,
  lparentsym,
  rparentsym,
  commasym,
  semicolonsym,
  periodsym,
  becomessym,
  beginsym,
  endsym,
  ifsym,
  thensym,
  whilesym,
  dosym,
  callsym,
  constsym,
  varsym,
  procsym,
  writesym,
  readsym,
  elsesym
} tokens;

// Helper functions declared
int checkValid(char programString[]);
int printLexList(char *programString, char **word, char *lexList);
void printText();

// Symbol Table functions
void addSymbol(int kind, char *name, int val, int level, int addr, int mark);
int checkSymbolTable(char *name, int level);

// Functions for each non-terminal symbol
int programCheck(char *lexList);
int blockCheck(char *lexList);
int constCheck(char *lexList);
int varCheck(char *lexList);
int procedureCheck(char *lexList);
int statementCheck(char *lexList);
int checkCondition(char *lexList);
int checkExpression(char *lexList);
int checkTerm(char *lexList);
int checkFactor(char *lexList);

// Functions for emitting, error, and getting the next token
int emit(char *OP, int L, int M);
int error(int errorCode, char *symbol);
void getToken(char *lexList, char *token);

// Main function
int main(int argc, char *argv[])
{
  // Variable defined for this function
  FILE *ifp = fopen(argv[1], "r");
  int count = 1;
  int i = 0;
  char c;
  char *word[] = {"begin", "const", "do", "end", "if", "odd", "read", "then", "var",
                  "while", "write", "call", "procedure"};

  // Checks for null pointers
  if (ifp == NULL)
  {
    printf("Error: File not found.");
    return -1;
  }

  // Gets first character from input file
  c = fgetc(ifp);

  // Copys content of input file to ouput file
  printf("\nSource Program:\n\n");
  while (!feof(ifp))
  {
    printf("%c", c);
    if (c != ' ' && c != '\t' && c != '\n')
      count++;

    c = fgetc(ifp);
  }

  // Creates a string to hold all valid scanner characters with bars at terminal character
  char programString[count * 2];
  rewind(ifp);
  c = fgetc(ifp);
  while (1)
  {
    if (c == '.' || feof(ifp))
    {
      if (feof(ifp))
      {
        programString[i] = '\0';
        break;
      }
      programString[i] = '|';
      programString[i + 1] = c;
      programString[i + 2] = '\0';
      break;
    }

    if (c == ',' || c == ';' || c == '+' || c == '-' || c == '/' || c == '*' || c == ':' || c == '<' || c == '>' || c == '=')
    {
      if (i >= 1)
      {
        char tempChar = programString[i - 1];
        if ((tempChar >= 'A' && tempChar <= 'Z') || (tempChar >= 'a' && tempChar <= 'z') || (tempChar >= '0' && tempChar <= '9'))
        {
          programString[i] = '|';
          i += 1;
        }
      }
      programString[i] = c;
      i += 1;
      programString[i] = '|';
    }
    else if (c == ' ' || c == '\t' || c == '\n')
    {
      if (programString[i - 1] != '|')
        programString[i] = '|';
      else
      {
        c = fgetc(ifp);
        continue;
      }
    }
    else if (c != ' ' && c != '\t' && c != '\n')
    {
      programString[i] = c;
    }

    c = fgetc(ifp);
    i++;
  }

  fclose(ifp);

  // This calls a function to check to invalid characters and length of numbers and identifiers
  if (checkValid(programString) == -1)
    return -1;

  char lexList[strlen(programString)];
  lexList[0] = '\0';

  // Function to create the lexeme list
  if (printLexList(programString, word, lexList) == 0)
    return -1;

  // Adds main function and JMP call to symbol table and text respectively
  addSymbol(3, "main", 0, 0, 0, 0);

  // This call begins our parser. First with a check that the program is valid
  if (programCheck(lexList) == -1)
    return -1;

  printf("\n\nNo errors, program is syntactically correct.\n");

  // This is where everything prints to the terminal
  printf("\nAssembly Code:\n\n");
  printText();

  printf("\n");
  return 1;
}

// This function is used to check the input string for invalid characters and numbers and
// identifiers that are too long
int checkValid(char programString[])
{
  // All variables for this function
  char c;
  int i = 0;
  int numCounter = 0;
  int charCounter = 0;
  int trueNum = 0;

  // This while loop is used to iterate over the programString
  while (1)
  {
    c = programString[i];

    // If we reach the end the program we break out of the loop
    if (c == '.' || c == '\0')
    {
      if (c == '\0')
        return error(9, "");

      break;
    }

    // This if statement checks for valid characters. The inner while loop skips over comments
    if ((c >= '(' && c <= '>') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '|')
    {
      if (programString[i] == '/' && programString[i + 1] == '|' && programString[i + 2] == '*')
      {
        while (1)
        {
          if (programString[i - 2] == '*' && programString[i - 1] == '|' && programString[i] == '/')
          {
            i++;
            c = programString[i];
            break;
          }
          i++;
        }
      }

      // This if statement checks if each lexeme is within the contraints
      if (c == '|')
      {
        if (numCounter > 5)
          return error(25, "");

        if (charCounter > 11)
          return error(26, "");

        numCounter = 0;
        trueNum = 0;
        charCounter = 0;
      }

      // This is for finding number rather than identifiers
      if ((c >= '0' && c <= '9') && programString[i - 1] == '|')
        trueNum = 1;

      if (trueNum == 1)
        numCounter++;
      else
        charCounter++;
    }
    else
    {
      // This prints any invalid characters
      char invalid[2];
      invalid[0] = c;
      return error(27, invalid);
    }

    i++;
  }

  return 1;
}

// This function prints the Lexeme List
int printLexList(char *programString, char **word, char *lexList)
{
  // Some variable defined for this function
  char c;
  int i = 0, j = 0;

  // This while loop is used to iterate over the programString
  while (1)
  {
    // Here a temporary string is made to hold each lexeme as it is read in
    c = programString[i];
    j = 0;
    char tempString[12] = "";
    char tempIdent[12] = "";

    // If a '.' is found the while loop breaks
    if (c == '.')
    {
      sprintf(tempIdent, "%d", periodsym);
      strcat(lexList, tempIdent);
      break;
    }

    // This if statement iterates over comments because they can be ignored
    if (programString[i] == '/' && programString[i + 1] == '|' && programString[i + 2] == '*')
    {
      while (1)
      {
        if (programString[i - 2] == '*' && programString[i - 1] == '|' && programString[i] == '/')
        {
          i++;
          c = programString[i];
          break;
        }
        i++;
      }
    }

    // This is where keywords and identifiers are detected and put into the lexeme list
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
    {
      // This while loop creates a temporary string to hold the current lexeme
      while (1)
      {
        if (programString[i] != '|')
        {
          tempString[j] = programString[i];
          j++;
          i++;
        }
        else
          break;
      }

      //  These if statements determine what the lexeme is, either a keyword or identifier
      if (strcmp(tempString, word[0]) == 0)
      {
        sprintf(tempIdent, "%d", beginsym);
        strcat(lexList, tempIdent);
        strcat(lexList, "|");
      }
      else if (strcmp(tempString, word[1]) == 0)
      {
        sprintf(tempIdent, "%d", constsym);
        strcat(lexList, tempIdent);
        strcat(lexList, "|");
      }
      else if (strcmp(tempString, word[2]) == 0)
      {
        sprintf(tempIdent, "%d", dosym);
        strcat(lexList, tempIdent);
        strcat(lexList, "|");
      }
      else if (strcmp(tempString, word[3]) == 0)
      {
        sprintf(tempIdent, "%d", endsym);
        strcat(lexList, tempIdent);
        strcat(lexList, "|");
      }
      else if (strcmp(tempString, word[4]) == 0)
      {
        sprintf(tempIdent, "%d", ifsym);
        strcat(lexList, tempIdent);
        strcat(lexList, "|");
      }
      else if (strcmp(tempString, word[5]) == 0)
      {
        sprintf(tempIdent, "%d", oddsym);
        strcat(lexList, tempIdent);
        strcat(lexList, "|");
      }
      else if (strcmp(tempString, word[6]) == 0)
      {
        sprintf(tempIdent, "%d", readsym);
        strcat(lexList, tempIdent);
        strcat(lexList, "|");
      }
      else if (strcmp(tempString, word[7]) == 0)
      {
        sprintf(tempIdent, "%d", thensym);
        strcat(lexList, tempIdent);
        strcat(lexList, "|");
      }
      else if (strcmp(tempString, word[8]) == 0)
      {
        sprintf(tempIdent, "%d", varsym);
        strcat(lexList, tempIdent);
        strcat(lexList, "|");
      }
      else if (strcmp(tempString, word[9]) == 0)
      {
        sprintf(tempIdent, "%d", whilesym);
        strcat(lexList, tempIdent);
        strcat(lexList, "|");
      }
      else if (strcmp(tempString, word[10]) == 0)
      {
        sprintf(tempIdent, "%d", writesym);
        strcat(lexList, tempIdent);
        strcat(lexList, "|");
      }
      else if (strcmp(tempString, word[11]) == 0)
      {
        sprintf(tempIdent, "%d", callsym);
        strcat(lexList, tempIdent);
        strcat(lexList, "|");
      }
      else if (strcmp(tempString, word[12]) == 0)
      {
        sprintf(tempIdent, "%d", procsym);
        strcat(lexList, tempIdent);
        strcat(lexList, "|");
      }
      else
      {
        sprintf(tempIdent, "%d", identsym);
        strcat(lexList, tempIdent);
        strcat(lexList, "|");

        strcat(lexList, tempString);
        strcat(lexList, "|");
      }
    }

    // This else if activates when a operation, terminal character, or number is detected
    else if (c >= '(' && c <= '>')
    {
      if (c >= '0' && c <= '9')
      {
        // This while loop creates a temporary string to hold the current lexeme
        while (1)
        {
          if (programString[i] != '|')
          {
            tempString[j] = programString[i];
            j++;
            i++;
          }
          else
            break;
        }
        sprintf(tempIdent, "%d", numbersym);
        strcat(lexList, tempIdent);
        strcat(lexList, "|");

        sprintf(tempString, "%s", tempString);
        strcat(lexList, tempString);
        strcat(lexList, "|");
      }
      else
      {
        // This switch is used to detect operators and terminal characters
        switch (c)
        {
        case '+':
          sprintf(tempString, "%d", plussym);
          strcat(lexList, tempString);
          strcat(lexList, "|");
          break;
        case '-':
          sprintf(tempString, "%d", minussym);
          strcat(lexList, tempString);
          strcat(lexList, "|");
          break;
        case '*':
          sprintf(tempString, "%d", multsym);
          strcat(lexList, tempString);
          strcat(lexList, "|");
          break;
        case '/':
          sprintf(tempString, "%d", slashsym);
          strcat(lexList, tempString);
          strcat(lexList, "|");
          break;
        case '(':
          sprintf(tempString, "%d", lparentsym);
          strcat(lexList, tempString);
          strcat(lexList, "|");
          break;
        case ')':
          sprintf(tempString, "%d", rparentsym);
          strcat(lexList, tempString);
          strcat(lexList, "|");
          break;
        case ',':
          sprintf(tempString, "%d", commasym);
          strcat(lexList, tempString);
          strcat(lexList, "|");
          break;
        case ';':
          sprintf(tempString, "%d", semicolonsym);
          strcat(lexList, tempString);
          strcat(lexList, "|");
          break;
        case '=':
          sprintf(tempString, "%d", eqlsym);
          strcat(lexList, tempString);
          strcat(lexList, "|");
          break;
        case '>':
          if (programString[i + 2] == '=')
          {
            sprintf(tempString, "%d", geqsym);
            strcat(lexList, tempString);
            strcat(lexList, "|");
            i += 3;
          }
          else
          {
            sprintf(tempString, "%d", gtrsym);
            strcat(lexList, tempString);
            strcat(lexList, "|");
          }
          break;
        case '<':
          if (programString[i + 2] == '=')
          {
            sprintf(tempString, "%d", leqsym);
            strcat(lexList, tempString);
            strcat(lexList, "|");
            i += 3;
          }
          else if (programString[i + 2] == '>')
          {
            sprintf(tempString, "%d", neqsym);
            strcat(lexList, tempString);
            strcat(lexList, "|");
            i += 3;
          }
          else
          {
            sprintf(tempString, "%d", lessym);
            strcat(lexList, tempString);
            strcat(lexList, "|");
          }
          break;
        case ':':
          if (programString[i + 2] == '=')
          {
            sprintf(tempString, "%d", becomessym);
            strcat(lexList, tempString);
            strcat(lexList, "|");
            i += 3;
          }
          else
            return error(27, ":");
          break;
        }
      }
    }
    i++;
  }
  return 1;
}

// This function is used to search the symbol table for a name
int checkSymbolTable(char *identName, int mode)
{
  for (int i = tableIndex - 1; i > 0; i--)
  {
    if (symbolTable[i].mark != 1)
    {
      if (mode == 1)
      {
        if ((strcmp(symbolTable[i].name, identName) == 0) && (symbolTable[i].level == lexLevel))
          return i;
      }
      else
      {
        if (strcmp(symbolTable[i].name, identName) == 0)
          return i;
      }
    }
  }

  return -1;
}

// This function is used to add symbols to the symbol table
void addSymbol(int kind, char *name, int val, int level, int addr, int mark)
{
  symbolTable[tableIndex].kind = kind;
  strcpy(symbolTable[tableIndex].name, name);
  symbolTable[tableIndex].val = val;
  symbolTable[tableIndex].level = level;
  symbolTable[tableIndex].addr = addr;
  symbolTable[tableIndex].mark = mark;
  tableIndex++;
}

// Functions for parser and code generation

// This is the program function. It calls the block function and checks for a
// period at the end of the input file
int programCheck(char *lexList)
{
  // Here we call the Block function, if theres an error we return -1;
  if (blockCheck(lexList) != -1)
  {
    char buffer[3] = "";
    int j = 0;
    for (int i = 2; i > 0; i--)
    {
      buffer[j] = lexList[strlen(lexList) - i];
      j++;
    }

    // This checks for a period at the end
    if (atoi(buffer) != periodsym)
      return error(9, "");
  }
  else
    return -1;

  // This emits the halt OP
  if (emit("SYS", 0, 3) == -1)
    return -1;

  return 1;
}

// This is the block function, it checks for constants, variables, and statements
int blockCheck(char *lexList)
{
  // A jump address is kept for each procedure
  // the lexicographical level is also incremented
  int jmpAddress = cx;
  lexLevel++;
  if (emit("JMP", 0, 0) == -1)
    return -1;

  int numVars = 0;
  getToken(lexList, token);

  // Here are the other function calls
  if (constCheck(lexList) == -1)
    return -1;

  if ((numVars = varCheck(lexList)) == -1)
    return -1;

  if (procedureCheck(lexList) == -1)
    return -1;

  // the procedure JMP is updated with the current code index
  text[jmpAddress].M = cx;

  if (emit("INC", 0, numVars + 3) == -1)
    return -1;

  if (statementCheck(lexList) == -1)
    return -1;

  if (lexLevel != 0)
  {
    if (emit("OPR", 0, 0) == -1)
      return -1;
  }

  // The lexicographical level is decremented
  lexLevel--;

  return 1;
}

// This is the constant function, it checks for a valid constant assignment
int constCheck(char *lexList)
{
  int num;
  if (atoi(token) == constsym)
  {
    while (1)
    {
      num = 0;
      char name[12] = "";
      getToken(lexList, token);

      // Checks for an identifier
      if (atoi(token) != identsym)
        return error(4, "");

      getToken(lexList, token);
      strcpy(name, token);

      // Checks the symbol table for the indentifier passed
      if (checkSymbolTable(name, 1) != -1)
        return error(3, name);

      getToken(lexList, token);

      // Checks for an assignment using the = symbol
      if (atoi(token) != eqlsym)
        return error(1, "");

      getToken(lexList, token);

      // Checks for a number
      if (atoi(token) != numbersym)
        return error(2, "");

      // Here we add the constant to the symbol table
      getToken(lexList, token);

      num = atoi(token);
      addSymbol(1, name, num, lexLevel, 0, 0);

      getToken(lexList, token);

      // This checks for a comma or a semicolon after the assignment,
      // otherwise throws an error

      if (atoi(token) == commasym || atoi(token) == semicolonsym)
      {
        if (atoi(token) == semicolonsym)
          break;
      }
      else
        return error(5, "");
    }
    getToken(lexList, token);
  }
  return 1;
}

// This is the variable function, it checks for a valid constant assignment
int varCheck(char *lexList)
{
  int numVars = 0;
  if (atoi(token) == varsym)
  {
    // This while loop is used to count the varaible in an assignment statement
    while (1)
    {
      numVars++;
      char name[12] = "";
      getToken(lexList, token);

      // Checks for an identifier
      if (atoi(token) != identsym)
        return error(4, "");

      getToken(lexList, token);
      strcpy(name, token);

      // Checks for the identifier in the symbol table
      if (checkSymbolTable(name, 1) != -1)
        return error(3, name);

      // Adds the variable to the symbol table
      addSymbol(2, name, 0, lexLevel, numVars + 2, 0);
      getToken(lexList, token);

      // This checks for a comma or a semicolon after the assignment,
      // otherwise throws an error

      if (atoi(token) == commasym || atoi(token) == semicolonsym)
      {
        if (atoi(token) == semicolonsym)
          break;
      }
      else
        return error(5, "");
    }
    getToken(lexList, token);
  }
  return numVars;
}

// This is the procedure chech function, it makes sure the procedure is impemented correctly
int procedureCheck(char *lexList)
{
  // A mark index is initialized for later use
  int markIndex = 0;
  if (atoi(token) == procsym)
  {
    while (1)
    {
      // Checks for an identifier
      getToken(lexList, token);

      if (atoi(token) != identsym)
        return error(4, "");

      char name[12] = "";
      getToken(lexList, token);
      strcpy(name, token);

      // Checks for the identifier in the symbol table
      if (checkSymbolTable(name, 1) != -1)
        return error(3, "");

      // Adds the procedure name to the symbol table and set the mark index
      addSymbol(3, name, 0, lexLevel, cx, 0);
      getToken(lexList, token);
      markIndex = tableIndex;

      if (atoi(token) != semicolonsym)
        return error(6, "");

      if (blockCheck(lexList) == -1)
        return -1;

      // Marks all the finished symbols
      for (int i = markIndex; i < tableIndex; i++)
        symbolTable[i].mark = 1;

      if (atoi(token) != semicolonsym)
        return error(8, "");

      getToken(lexList, token);

      if (atoi(token) != procsym)
        break;
    }
  }
  return 1;
}

// This is the statement function, it will call other non-terminal functions,
// otherwise it will return -1
int statementCheck(char *lexList)
{
  int symIdx = 0;
  int i = 0;
  int jpcIdx = 0;
  int loopIdx = 0;

  // Checks if token it an identifier
  if (atoi(token) == identsym)
  {
    char name[12] = "";
    getToken(lexList, token);
    strcpy(name, token);
    symIdx = checkSymbolTable(name, 0);

    // This throws an error if the identifier was not declared
    if (symIdx == -1)
      return error(11, name);

    // This throws an error if the identifier is not a variable
    if (symbolTable[symIdx].kind != 2)
      return error(12, "");

    getToken(lexList, token);

    // This throws an error is the next token is not :=
    if (atoi(token) != becomessym)
      return error(13, "");

    getToken(lexList, token);

    // Checks for a valid expression
    if (checkExpression(lexList) == -1)
      return -1;

    // Emits the STO instruction
    if (emit("STO", (lexLevel - symbolTable[symIdx].level), symbolTable[symIdx].addr) == -1)
      return -1;

    return 1;
  }

  // Checks if token is the 'call' keyword
  if (atoi(token) == callsym)
  {
    getToken(lexList, token);

    // Checks for an identifier
    if (atoi(token) != identsym)
      return error(14, "");

    char name[12] = "";
    getToken(lexList, token);
    strcpy(name, token);
    symIdx = checkSymbolTable(name, 0);

    // This throws an error if the identifier was not declared
    if (symIdx == -1)
      return error(11, name);

    // This throws an error if the identifier is not a procedure
    if (symbolTable[symIdx].kind != 3)
      return error(15, "");

    // Emits a CAL OP
    if (emit("CAL", (lexLevel - symbolTable[symIdx].level), symbolTable[symIdx].addr) == -1)
      return -1;

    getToken(lexList, token);
    return 1;
  }

  // Checks if token is the 'begin' keyword
  if (atoi(token) == beginsym)
  {
    // Loops until the 'end' keyword
    while (1)
    {
      getToken(lexList, token);

      // Calls the statement function again
      if (statementCheck(lexList) == -1)
        return -1;

      // Checks for a semicolon or 'end' keyword, otherwise it throws an error
      if (atoi(token) == semicolonsym || atoi(token) == endsym)
      {
        if (atoi(token) == endsym)
          break;
      }
      else
        return error(17, "");
    }
    getToken(lexList, token);
    return 1;
  }

  // Checks for the 'if' keyword
  if (atoi(token) == ifsym)
  {
    getToken(lexList, token);

    // Calls the condition function
    if (checkCondition(lexList) == -1)
      return -1;

    // Emits JPC
    jpcIdx = cx;
    if (emit("JPC", 0, 0) == -1)
      return -1;

    // Checks for the 'then' keyword
    if (atoi(token) != thensym)
      return error(16, "");

    getToken(lexList, token);

    // Calls the statement function
    if (statementCheck(lexList) == -1)
      return -1;

    text[jpcIdx].M = cx;
    return 1;
  }

  // Checks if token is 'while' keyword
  if (atoi(token) == whilesym)
  {
    getToken(lexList, token);
    loopIdx = cx;

    // Calls condition function
    if (checkCondition(lexList) == -1)
      return -1;

    // Checks if token is 'do' keyword
    if (atoi(token) != dosym)
      return error(18, "");

    getToken(lexList, token);

    // Emits JPC
    jpcIdx = cx;
    if (emit("JPC", 0, 0) == -1)
      return -1;

    // Calls statement check
    if (statementCheck(lexList) == -1)
      return -1;

    // Emits JMP
    if (emit("JMP", 0, loopIdx) == -1)
      return -1;

    text[jpcIdx].M = cx;
    return 1;
  }

  // Checks if the token is 'read' symbol table
  if (atoi(token) == readsym)
  {
    getToken(lexList, token);

    // Checks if token is an identifier
    if (atoi(token) != identsym)
      return error(19, "");

    char name[12] = "";
    getToken(lexList, token);

    strcpy(name, token);
    symIdx = checkSymbolTable(name, 0);

    // Checks if the identifier has been declared
    if (symIdx == -1)
      return error(11, name);

    // Checks if identifier is a variable
    if (symbolTable[symIdx].kind != 2)
      return error(19, "");

    getToken(lexList, token);

    // Emits read and STO
    if (emit("SYS", 0, 2) == -1)
      return -1;
    if (emit("STO", (lexLevel - symbolTable[symIdx].level), symbolTable[symIdx].addr) == -1)
      return -1;

    return 1;
  }

  // Check if token is 'write' keyword
  if (atoi(token) == writesym)
  {
    getToken(lexList, token);

    // Call expression function
    if (checkExpression(lexList) == -1)
      return -1;

    // Emits write
    if (emit("SYS", 0, 1) == -1)
      return -1;

    return 1;
  }
  return 1;
}

// This function checks for a valid condition
int checkCondition(char *lexList)
{
  // Checks id token is 'odd' keyword
  if (atoi(token) == oddsym)
  {
    getToken(lexList, token);
    if (checkExpression(lexList) == -1)
      return -1;
    if (emit("OPR", 0, 11) == -1)
      return -1;
  }
  else
  {
    // Calls exoression function
    if (checkExpression(lexList) == -1)
      return -1;

    // Checks if token is = symbol
    if (atoi(token) == eqlsym)
    {
      getToken(lexList, token);
      if (checkExpression(lexList) == -1)
        return -1;

      if (emit("OPR", 0, 5) == -1)
        return -1;
    }
    // Checks if token is <> symbol
    else if (atoi(token) == neqsym)
    {
      getToken(lexList, token);
      if (checkExpression(lexList) == -1)
        return -1;

      if (emit("OPR", 0, 6) == -1)
        return -1;
    }
    // Checks if token is < symbol
    else if (atoi(token) == lessym)
    {
      getToken(lexList, token);
      if (checkExpression(lexList) == -1)
        return -1;

      if (emit("OPR", 0, 7) == -1)
        return -1;
    }
    // Checks if token is <= symbol
    else if (atoi(token) == leqsym)
    {
      getToken(lexList, token);
      if (checkExpression(lexList) == -1)
        return -1;

      if (emit("OPR", 0, 8) == -1)
        return -1;
    }
    // Checks if token is > symbol
    else if (atoi(token) == gtrsym)
    {
      getToken(lexList, token);
      if (checkExpression(lexList) == -1)
        return -1;

      if (emit("OPR", 0, 9) == -1)
        return -1;
    }
    // Checks if token is >= symbol
    else if (atoi(token) == geqsym)
    {
      getToken(lexList, token);
      if (checkExpression(lexList) == -1)
        return -1;

      if (emit("OPR", 0, 10) == -1)
        return -1;
    }
    // Otherwise throws an error
    else
      return error(20, "");
  }
  return 1;
}

// This function is to check for a valid expression
int checkExpression(char *lexList)
{
  // First checks for '-' symbol
  if (atoi(token) == minussym)
  {
    getToken(lexList, token);
    if (checkTerm(lexList) == -1)
      return -1;

    // This while loop runs until a '+' or '-' is not detected
    while (atoi(token) == plussym || atoi(token) == minussym)
    {
      if (atoi(token) == plussym)
      {
        getToken(lexList, token);
        if (checkTerm(lexList) == -1)
          return -1;

        if (emit("OPR", 0, 1) == -1)
          return -1;
      }
      else
      {
        getToken(lexList, token);
        if (checkTerm(lexList) == -1)
          return -1;
        if (emit("OPR", 0, 2) == -1)
          return -1;
      }
    }
  }
  else
  {
    if (atoi(token) == plussym)
      getToken(lexList, token);

    if (checkTerm(lexList) == -1)
      return -1;

    // This while loop runs until a '+' or '-' is not detected
    while (atoi(token) == plussym || atoi(token) == minussym)
    {
      if (atoi(token) == plussym)
      {
        getToken(lexList, token);
        if (checkTerm(lexList) == -1)
          return -1;

        if (emit("OPR", 0, 1) == -1)
          return -1;
      }
      else
      {
        getToken(lexList, token);
        if (checkTerm(lexList) == -1)
          return -1;
        if (emit("OPR", 0, 2) == -1)
          return -1;
      }
    }
  }
  return 1;
}

// This function checks for a valid Term
int checkTerm(char *lexList)
{
  // Calls the factor function
  if (checkFactor(lexList) == -1)
    return -1;

  // This while loop runs until a '*' or '/' is not detected
  while (atoi(token) == multsym || atoi(token) == slashsym)
  {
    if (atoi(token) == multsym)
    {
      getToken(lexList, token);
      if (checkFactor(lexList) == -1)
        return -1;
      if (emit("OPR", 0, 3) == -1)
        return -1;
    }
    else
    {
      getToken(lexList, token);
      if (checkFactor(lexList) == -1)
        return -1;
      if (emit("OPR", 0, 4) == -1)
        return -1;
    }
  }
  return 1;
}

// This function checks for a valid factor
int checkFactor(char *lexList)
{
  int symIdx = 0;

  // Checks if token an identifier
  if (atoi(token) == identsym)
  {
    char name[12] = "";
    getToken(lexList, token);

    strcpy(name, token);
    symIdx = checkSymbolTable(name, 0);

    // Checks if the identifier is declared
    if (symIdx == -1)
      return error(11, name);

    // Checks is the symbol is a literal or not
    if (symbolTable[symIdx].kind == 1)
    {
      if (emit("LIT", 0, symbolTable[symIdx].val) == -1)
        return -1;
    }
    else if (symbolTable[symIdx].kind == 2)
    {
      if (emit("LOD", (lexLevel - symbolTable[symIdx].level), symbolTable[symIdx].addr) == -1)
        return -1;
    }
    else
    {
      return error(21, "");
    }

    getToken(lexList, token);
  }
  // Checks if token is a number
  else if (atoi(token) == numbersym)
  {
    int num = 0;
    getToken(lexList, token);

    num = atoi(token);

    // Emits the LIT
    if (emit("LIT", 0, num) == -1)
      return -1;
    getToken(lexList, token);
  }
  // Checks if token is a '('
  else if (atoi(token) == lparentsym)
  {
    getToken(lexList, token);
    if (checkExpression(lexList) == -1)
      return -1;

    // Checks if token is ')'
    if (atoi(token) != rparentsym)
      return error(22, "");

    getToken(lexList, token);
  }
  else
    return error(23, "");

  return 1;
}

// Function to emit instructs to the text array
int emit(char *OP, int L, int M)
{
  // Checks if the index is within the constraints
  if (cx > CODE_SIZE)
  {
    return -1;
  }
  // Otherwise adds the instruction to the text array
  else
  {
    strcpy(text[cx].OP, OP);
    text[cx].L = L;
    text[cx].M = M;
    cx++;
  }
  return 1;
}

// Function to get the next token in the lexeme list
void getToken(char *lexList, char *token)
{
  char temp[12] = "";
  int i = 0;
  while (lexList[lexIndex] != '|')
  {
    if ((lexList[lexIndex] >= 'A' && lexList[lexIndex] <= 'Z') ||
        (lexList[lexIndex] >= 'a' && lexList[lexIndex] <= 'z') || (lexList[lexIndex] >= '0' && lexList[lexIndex] <= '9'))
    {
      temp[i] = lexList[lexIndex];
      i++;
    }
    lexIndex++;
  }

  strcpy(token, temp);
  lexIndex++;
  // printf("%s\n", token);
}

// Function to print the assembly code
void printText()
{
  int i = 0;
  FILE *ofp = fopen("elf.txt", "w");
  if (ofp == NULL)
  {
    // error
    return;
  }

  // This prints to both the terminal and a file
  printf("Line\tOP\tL\tM\n");
  for (i = 0; i < cx; i++)
  {
    if (strcmp(text[i].OP, "LIT") == 0)
    {
      printf("  %d\t%s\t%d\t%d\n", i, text[i].OP, text[i].L, text[i].M);
      fprintf(ofp, "1 %d %d\n", text[i].L, text[i].M);
    }
    else if (strcmp(text[i].OP, "OPR") == 0)
    {
      printf("  %d\t%s\t%d\t%d\n", i, text[i].OP, text[i].L, text[i].M);
      fprintf(ofp, "2 %d %d\n", text[i].L, text[i].M);
    }
    else if (strcmp(text[i].OP, "LOD") == 0)
    {
      printf("  %d\t%s\t%d\t%d\n", i, text[i].OP, text[i].L, text[i].M);
      fprintf(ofp, "3 %d %d\n", text[i].L, text[i].M);
    }
    else if (strcmp(text[i].OP, "STO") == 0)
    {
      printf("  %d\t%s\t%d\t%d\n", i, text[i].OP, text[i].L, text[i].M);
      fprintf(ofp, "4 %d %d\n", text[i].L, text[i].M);
    }
    else if (strcmp(text[i].OP, "CAL") == 0)
    {
      printf("  %d\t%s\t%d\t%d\n", i, text[i].OP, text[i].L, (text[i].M * 3));
      fprintf(ofp, "5 %d %d\n", text[i].L, (text[i].M * 3));
    }
    else if (strcmp(text[i].OP, "INC") == 0)
    {
      printf("  %d\t%s\t%d\t%d\n", i, text[i].OP, text[i].L, text[i].M);
      fprintf(ofp, "6 %d %d\n", text[i].L, text[i].M);
    }
    else if (strcmp(text[i].OP, "JMP") == 0)
    {
      printf("  %d\t%s\t%d\t%d\n", i, text[i].OP, text[i].L, (text[i].M * 3));
      fprintf(ofp, "7 %d %d\n", text[i].L, (text[i].M * 3));
    }
    else if (strcmp(text[i].OP, "JPC") == 0)
    {
      printf("  %d\t%s\t%d\t%d\n", i, text[i].OP, text[i].L, (text[i].M * 3));
      fprintf(ofp, "8 %d %d\n", text[i].L, (text[i].M * 3));
    }
    else if (strcmp(text[i].OP, "SYS") == 0)
    {
      printf("  %d\t%s\t%d\t%d\n", i, text[i].OP, text[i].L, text[i].M);
      fprintf(ofp, "9 %d %d\n", text[i].L, text[i].M);
    }
  }
  fclose(ofp);
}

// This function prints out specific errors and returns -1 to signal an program interrupt
int error(int errorCode, char *symbol)
{
  printf("\n\n***** Error Number ");
  switch (errorCode)
  {
  case 1:
    printf("1: 'const' identifier must be followed by '='.\n");
    break;
  case 2:
    printf("2: '=' must be followed by a number.\n");
    break;
  case 3:
    printf("3: Symbol has already been declared: %s.\n", symbol);
    break;
  case 4:
    printf("4: 'const', 'var', 'procedure' must be followed by an identifier.\n");
    break;
  case 5:
    printf("5: Semicolon or comma missing.\n");
    break;
  case 6:
    printf("6: Incorrect symbol after procedure declaration.\n");
    break;
  case 7:
    printf("7: Statement expected.\n");
    break;
  case 8:
    printf("8: Incorrect symbol sfter statement part in block.\n");
    break;
  case 9:
    printf("9: Period expected.\n");
    break;
  case 10:
    printf("10: Semicolon between statements missing.\n");
    break;
  case 11:
    printf("11: Undeclared identifier: %s.\n", symbol);
    break;
  case 12:
    printf("12: Assignment to constant or procedure not allowed.\n");
    break;
  case 13:
    printf("13: Assignment operator expected.\n");
    break;
  case 14:
    printf("14: 'call' must be followed by an identifier.\n");
    break;
  case 15:
    printf("15: Call of a constant or variable is meaningless.\n");
    break;
  case 16:
    printf("16: 'then' expected.\n");
    break;
  case 17:
    printf("17: Semicolon or 'end' expected.\n");
    break;
  case 18:
    printf("18: 'do' expected.\n");
    break;
  case 19:
    printf("19: Incorrect symbol following statement.\n");
    break;
  case 20:
    printf("20: Relational operator expected.\n");
    break;
  case 21:
    printf("21: Expression must not contain a procedure identifier.\n");
    break;
  case 22:
    printf("22: Right parenthesis missing.\n");
    break;
  case 23:
    printf("23: The preceding factor cannot begin with this symbol.\n");
    break;
  case 24:
    printf("24: An expression cannot begin with this symbol.\n");
    break;
  case 25:
    printf("25: Number is too large.\n");
    break;
  case 26:
    printf("26: Identifier is too long.\n");
    break;
  case 27:
    printf("27: Invalid symbol: %s.\n", symbol);
    break;
  }

  FILE *ofp = fopen("elf.txt", "w");
  if (ofp == NULL)
    return -1;
  fprintf(ofp, "Error");
  fclose(ofp);
  return -1;
}