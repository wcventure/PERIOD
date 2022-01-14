#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
 
int my_account = 0;      //my account balance
int your_account = 100;  //your account balance
 
// transfer money!
int racy_transfer(int m)
{
    if (m <= your_account) {  //result unpredictable
        your_account -= m;      //result unpredictable
        my_account += m;      //result unpredictable
        return 1;
    } else {
        return 0;
    }
}

int main(int argc, char **argv) {
    pthread_t id1, id2;
    pthread_create(&id1, NULL, racy_transfer, 50);
    pthread_create(&id2, NULL, racy_transfer, 80);
    pthread_join(id1, NULL);
    pthread_join(id2, NULL);
    //assert(my_account >= 0 && your_account >= 0 && my_account + your_account ==100);
    printf("%d,%d\n", my_account, your_account);
    return 0;
}
