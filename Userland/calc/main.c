#include <stdio.h>
 
//Copied from https://www.tutorialgateway.org/c-program-to-create-simple-calculator/
int main()
{
	char Operator;
	int num1, num2, result = 0;
	
    for(;;)
    {
        printf("\n Please Enter an Operator (+, -, *, /)  :  ");
        scanf("%c", &Operator);
        getchar();
        printf("\n Please Enter the Values for two Operands: num1 and num2  :  ");
        scanf("%d%d", &num1, &num2);
        getchar();
        
        switch(Operator)
        {
            case '+':
                result = num1 + num2;
                break;
            case '-':
                result = num1 - num2;
                break;  			
            case '*':
                result = num1 * num2;
                break;
            case '/':
                result = num1 / num2;
                break;
            default:
                printf("\n You have enetered an Invalid Operator ");				    			
        }
    
        printf("\n The result of %d %c %d  = %d", num1, Operator, num2, result);
    }
	
  	return 0;
}