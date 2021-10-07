test cases



## Lexic and Syntatic tests

test1.c

```C
const int a  = 2;  // global const variable declaration

int aaa; // global variable declaration
int bbb = 100; // global variable declaration with initial value


// global void function declaration
int abc() {
    return 123;
}

// global int functino declaration
void gcd() {
    int b;        // local variable declaration 
    int a = 1;    // local variable declaration with initial value
    const int c = 1;  // const local variable declaration with initial value 
    
    int arr[4] = {1,1,2,5};
    // const brr[4] = {1,2,3,4};  //? zsh: illegal hardware instruction
}

//? zsh: illegal hardware instruction
// void avg(int a, int b)
// {
//     //int c = a + b;
// }
int main() {
    int a = 1;
    // blocks
    {
        int a = 1;  // nested variable
        int b = 1;
        // while expression. logical and relational expressions allowed
        while(a == b)
        {
            a = a+1;
            // if expression
            if( a == 100) 
            {
                break;
            }
        }
    }    
    int c = a + 1;  // variable expression assignment
    int x3 = 0x10;  // hexadecimal prefix
    int y3 = 0777;  // octal digit
} 
```

error in declaring: const  brr[4] = {1,2,3,4};

error in declaring: 

void avg(int a, int b)
{

​    int c = a + b;
}

// problems to be fixed



## Advanced Lexic and Syntatic tests



## Semantic test 

test3.c

```C
// some semantic test cases 
int main()
{
    int min = 0, max = -1, mid = -1;
    if(min == 0) {
        if (pos) pos = 0;  // undeclared variable
        return 0;
    }
    else {
        if(max > 5)
        {
            return 0;
        }
    }
    
    while(max >= min) {
        mid = (max + min) / 2;
        max = max - 1;
    }
    return  50;

}
```

semantic error: undeclared variable

// to be continue.



## Advanced Semantic Test





## IR 





