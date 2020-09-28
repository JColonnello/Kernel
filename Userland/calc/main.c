#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

int doOperation(double a, double b, char op, double *result)
{
    switch(op)
    {
        case '+':
            *result = a + b;
            break;
        case '-':
            *result = a - b;
            break;
        case '*':
            *result = a * b;
            break;
        case '/':
            if(b == 0.0)
            {
                printf("Divition by zero\n");
                return -5;
            }
            *result = a / b;
            break;
        default:
            printf("Invalid operand\n");
            return -2;
    }
    return 0;
}

int compute(const char *expr, int len, double *result)
{
    int i = 0;

    while(i < len && isspace(expr[i])) i++;
    if(i >= len)
    {
        printf("Missing expression\n");
        return -1;
    }
    if(isdigit(expr[i]) || expr[i] == '-')
    {
        int j = 0;
        char num[18];
        bool neg = false;
        if(expr[i] == '-')
        {
            neg = true;
            i++;
        }
        do
        {
            num[j++] = expr[i++];
        }
        while(isdigit(expr[i]) && j < sizeof(num) - 1);
        num[j] = 0;
        *result = atol(num);
        if(expr[i] == '.')
        {
            i++;
            j = 0;
            while(isdigit(expr[i]) && j < sizeof(num) - 1)
            {
                num[j++] = expr[i++];
            }
            num[j] = 0;
            double frac = atoi(num);
            while(j--) frac /= 10.0;
            *result += frac;
        }
        if(neg)
            *result = -*result;
        while(i < len && isspace(expr[i])) i++;
        return i;
    }
    else if(expr[i] == '(')
    {
        double a, b;
        i++;
        int rd = compute(&expr[i], len - i, &a);
        if(rd < 0)
            return rd;
        i += rd;
        char op = expr[i++];
        rd = compute(&expr[i], len - i, &b);
        if(rd < 0)
            return rd;
        i += rd;
        rd = doOperation(a, b, op, result);
        if(rd < 0)
            return rd;
        if(expr[i++] != ')')
        {
            printf("Invalid expresion\n");
            return -3;
        }
        while(i < len && isspace(expr[i])) i++;
        return i;
    }
    printf("Invalid symbol: %c\n", expr[i]);
    return -4;
}

int main()
{
    for(;;)
    {
        char line[512];
        printf(": ");
        scanf("%511[^=]=", line);
        while(getchar() != '\n');
        double result;
        size_t len = strlen(line);
        int rd = compute(line, len, &result);
        if(result > (double)LONG_MAX || result < (double)LONG_MIN)
            printf("Too big\n");
        else if(rd == len)
        {
            long integer = result;
            long frac = (result-integer)*10000;
            frac *= (result < 0) ? -1 : 1;
            printf("= %ld.%04ld\n", integer, frac);
        }
        else if(rd > 0)
            printf("Invalid symbols at end\n");
    }
	
  	return 0;
}