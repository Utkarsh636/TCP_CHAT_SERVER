#include <iostream>
// #include <vector>
#include <cmath>

using namespace std;

int main()
{
    char op;
    int num1, num2;
    cout << "welcome to the calculator" << endl;
    cout << "Enter the first number: ";
    cin >> num1;
    cout << "Enter the second number: ";
    cin >> num2;
    cout << "please select the operator to proceed" << endl;
    cout << "+ for addition" << endl;
    cout << "- for subtraction" << endl;
    cout << "* for multiplication" << endl;
    cout << "/ for division" << endl;
    cout << "^ for power" << endl;
    cout << "% for modulus" << endl;
    cin >> op;

    switch (op)
    {
    case '+':
        cout << num1 + num2;
        break;
    case '-':
        cout << num1 - num2;
        break;
    case '*':
        cout << num1 * num2;
        break;
    case '/':
        if (num2 == 0)
        {
            cout << "division by zero is not possible";
            break;
        }
        cout << num1 / num2;
        break;
    case '^':
        cout << pow(num1, num2);
        break;
    case '%':
        if (num2 == 0)
        {
            cout << "modulus by zero is not possible";
            break;
        }
        cout << num1 % num2;
        break;
    default:
        cout << "invalid operator";
    }
}