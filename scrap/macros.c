#include <stdio.h>

int main(){
#ifdef METHOD_A
  printf("method a");
#else
#ifdef METHOD_B
    printf("method b");
#else
    printf("default method");
#endif
#endif
}
