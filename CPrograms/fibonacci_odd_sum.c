#include <stdio.h>

int main() {

    int n, i;
    int a = 0, b = 1, next;
    int odd_sum = 0;

    printf("Enter number of terms: ");
    scanf("%d", &n);

    printf("Fibonacci Series:\n");

    for(i = 1; i <= n; i++) {

        printf("%d ", a);

        if(a % 2 != 0) {
            odd_sum += a;
        }

        next = a + b;
        a = b;
        b = next;
    }

    printf("\n\nSum of odd Fibonacci numbers = %d\n", odd_sum);

    return 0;
}
