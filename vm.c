// Ryan Weisman
// COP3402 Spring 2023

// All libraries needed
#include <stdio.h>
#include <stdlib.h>

// Defines an array size of 500
#define ARRAY_SIZE 500
int pas[ARRAY_SIZE];

// Uses a struct to store the current instruction
struct IR
{
  int OP;
  int L;
  int M;
};
typedef struct IR IR;

// Two functions were needed, one to find the base pointer given an L value
// and another to help rint the stack and seperate the AR's
int base(int BP, int L);
void printStack(int SP, int ARbases[], int count);

// Main function has all instruction code
int main(int argc, char *argv[])
{
  // All variable needed throughout the main function
  int BP = 499;
  int SP = BP + 1;
  int PC = 0;
  int i = 0;
  int j = 0;
  int eop = 1;
  int buffer1, buffer2, buffer3;
  int ARbases[ARRAY_SIZE];
  FILE *ifp = fopen(argv[1], "r");

  // Dynamically allocating an 500 size int array and initializing all values to 0
  for (i = 0; i < ARRAY_SIZE; i++)
  {
    pas[i] = 0;
    ARbases[i] = 0;
  }

  // Creating the Instruction register and initializing all values to 0
  IR Ir;
  Ir.OP = 0;
  Ir.L = 0;
  Ir.M = 0;

  // This while loop takes all instuctions from a text file and inserts them in
  // the 'text section' of the pas. (Starting at index 0)
  i = 0;
  while (!feof(ifp))
  {
    fscanf(ifp, "%d %d %d", &buffer1, &buffer2, &buffer3);
    pas[i] = buffer1;
    pas[i + 1] = buffer2;
    pas[i + 2] = buffer3;
    i += 3;
  }

  // An initial print statement giving inital values
  printf("\t\t\tPC\tBP\tSP\tStack\n");
  printf("Initial Values:\t\t0\t499\t500\n\n");
  while (eop)
  {
    // These lines represent the fetch cycle of the program
    Ir.OP = pas[PC];
    Ir.L = pas[PC + 1];
    Ir.M = pas[PC + 2];
    PC = PC + 3;

    // This switch statement determines which operation is exectuted base on the OP
    switch (Ir.OP)
    {
    case 1:
      // If OP is 1, a literal is pushed onto the stack and then all values are printed
      SP = SP - 1;
      pas[SP] = Ir.M;
      printf("\tLIT %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
      printStack(SP, ARbases, j);
      break;
    case 2:
      // If OP is 2, then OPR is determined by the value of M
      switch (Ir.M)
      {
      case 0:
        // If M is 0, the current AR is returned and then all values are printed
        SP = BP + 1;
        BP = pas[SP - 2];
        PC = pas[SP - 3];
        ARbases[j] = 0;
        j--;
        printf("\tRTN %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
        printStack(SP, ARbases, j);
        break;
      case 1:
        // If M is 1, the two top values are added and then all values are printed
        pas[SP + 1] = pas[SP + 1] + pas[SP];
        SP = SP + 1;
        printf("\tADD %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
        printStack(SP, ARbases, j);
        break;
      case 2:
        // If M is 2, the two top values are subtracted and then all values are printed
        pas[SP + 1] = pas[SP + 1] - pas[SP];
        SP = SP + 1;
        printf("\tSUB %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
        printStack(SP, ARbases, j);
        break;
      case 3:
        // If M is 3, the two top values are mulitplied and then all values are printed
        pas[SP + 1] = pas[SP + 1] * pas[SP];
        SP = SP + 1;
        printf("\tMUL %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
        printStack(SP, ARbases, j);
        break;
      case 4:
        // If M is 4, the two top values are divided and then all values are printed
        pas[SP + 1] = pas[SP + 1] / pas[SP];
        SP = SP + 1;
        printf("\tDIV %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
        printStack(SP, ARbases, j);
        break;
      case 5:
        // If M is 5, the two top to values are checked for equality and
        // then all values are printed
        if (pas[SP + 1] == pas[SP])
          pas[SP + 1] = 1;
        else
          pas[SP + 1] = 0;
        SP = SP + 1;
        printf("\tEQL %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
        printStack(SP, ARbases, j);
        break;
      case 6:
        // If M is 6, the two top values are check for inequality and
        // then all values are printed
        if (pas[SP + 1] != pas[SP])
          pas[SP + 1] = 1;
        else
          pas[SP + 1] = 0;
        SP = SP + 1;
        printf("\tNEQ %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
        printStack(SP, ARbases, j);
        break;
      case 7:
        // If M is 7, checks if ine of the top two values are less than the other
        // and then all values are printed
        if (pas[SP + 1] < pas[SP])
          pas[SP + 1] = 1;
        else
          pas[SP + 1] = 0;
        SP = SP + 1;
        printf("\tLSS %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
        printStack(SP, ARbases, j);
        break;
      case 8:
        // If M is 8, checks if one of the top two values are less than or equal to
        // the other and then all values are printed
        if (pas[SP + 1] <= pas[SP])
          pas[SP + 1] = 1;
        else
          pas[SP + 1] = 0;
        SP = SP + 1;
        printf("\tLEQ %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
        printStack(SP, ARbases, j);
        break;
      case 9:
        // If M is 9, checks if one of the top two values are greater than
        // the other and then all values are printed
        if (pas[SP + 1] > pas[SP])
          pas[SP + 1] = 1;
        else
          pas[SP + 1] = 0;
        SP = SP + 1;
        printf("\tGTR %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
        printStack(SP, ARbases, j);
        break;
      case 10:
        // If M is 10, checks if one of the top two values are greater than or equal
        // the other and then all values are printed
        if (pas[SP + 1] >= pas[SP])
          pas[SP + 1] = 1;
        else
          pas[SP + 1] = 0;
        SP = SP + 1;
        printf("\tGEQ %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
        printStack(SP, ARbases, j);
        break;
      }
      break;
    case 3:
      // If OP is 3, the value are lexicographical level L and offset M are loaded
      // to current AR and then all values are printed
      SP = SP - 1;
      pas[SP] = pas[base(BP, Ir.L) - Ir.M];
      printf("\tLOD %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
      printStack(SP, ARbases, j);
      break;
    case 4:
      // If OP is 4, the value at SP is stored at lexicographical level L and offset M
      // and then all values are printed
      pas[base(BP, Ir.L) - Ir.M] = pas[SP];
      SP = SP + 1;
      printf("\tSTO %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
      printStack(SP, ARbases, j);
      break;
    case 5:
      // If OP is 5, a new AR is created and then all values are printed
      pas[SP - 1] = base(BP, Ir.L);
      pas[SP - 2] = BP;
      pas[SP - 3] = PC;
      BP = SP - 1;
      PC = Ir.M;
      printf("\tCAL %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
      printStack(SP, ARbases, j);
      ARbases[j] = BP;
      j++;
      break;
    case 6:
      // If OP is 6, SP is incremented by M and then all values are printed
      SP = SP - Ir.M;
      printf("\tINC %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
      printStack(SP, ARbases, j);
      break;
    case 7:
      // If OP is 7, PC jumps to index M and then all values are printed
      PC = Ir.M;
      printf("\tJMP %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
      printStack(SP, ARbases, j);
      break;
    case 8:
      // If OP is 8, if the value in pas at index SP is 0 then PC jumps to index M
      // and then all values are printed
      if (pas[SP] == 0)
        PC = Ir.M;
      SP = SP + 1;
      printf("\tJPC %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
      printStack(SP, ARbases, j);
      break;
    case 9:
      // If OP is 9, then M determines which action is taken and then all values are printed
      switch (Ir.M)
      {
      case 1:
        // If M is 1, then the top of the current stack is printed out and then all
        // values are printed
        printf("Output result is: %d\n", pas[SP]);
        SP = SP + 1;
        printf("\tSYS %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
        printStack(SP, ARbases, j);
        break;
      case 2:
        // If M is 2, then the user is prompted to enter an integer which is
        // stored on top of the stack and then all values are printed
        SP = SP - 1;
        printf("Please enter an integer: \n");
        scanf("%d", &buffer1);
        pas[SP] = buffer1;
        printf("\tSYS %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
        printStack(SP, ARbases, j);
        break;
      case 3:
        // If M is 3, then the eop flag is set to 0 and the program is halted
        eop = 0;
        printf("\tSYS %d\t%d\t%d\t%d\t%d\t", Ir.L, Ir.M, PC, BP, SP);
        printStack(SP, ARbases, j);
      }
      break;
    }
  }

  // All allocated memory is freed
  fclose(ifp);
  return 1;
}

// This function is used to find the base pointer from a different
// lexicographical level
int base(int BP, int L)
{
  int arb = BP;
  while (L > 0)
  {
    arb = pas[arb];
    L--;
  }
  return arb;
}

// This function prints the values in the stack and it seperates AR's
// with a '|' value.
void printStack(int SP, int ARbases[], int count)
{
  int i = 0;
  int j = 0;
  int k = 0;

  k = count;
  while (j < (ARRAY_SIZE - SP))
  {
    if (k > 0 && ((ARRAY_SIZE - j - 1) == ARbases[count - k]))
    {
      printf("| ");
      k--;
    }
    printf("%d ", pas[ARRAY_SIZE - j - 1]);
    j++;
  }
  printf("\n");
}
