#include<stdio.h>

short * pa ;

char c[2] ;

int main(){
    c[0] = 2;
    c[1] = 1;
    
    pa = (short *)c;

    printf("%d ",(int)*pa);
    return 0;
}
