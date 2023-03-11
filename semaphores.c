#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define maxsize 5
int buffer[maxsize];
int front=-1;
int rear=-1;
int size=0;
// Check if the queue is full
int isFull()
{
    if (size>=maxsize)
        return 1;
    return 0;
}
// Check if the queue is empty
int isEmpty()
{
    if (size==0)
        return 1;
    return 0;
}
// Adding an element
void enqueue(int element)
{
    if (isFull())
        printf("\n Queue is full!! \n");
    else
    {
        if (front == -1)front = 0;
        rear = (rear + 1) % (maxsize-1);
        buffer[rear] = element;
        size++;
    }
}
// Removing an element
void dequeue()
{
    int element;
    if (isEmpty())
    {
        printf("\n Queue is empty !! \n");
    }
    else
    {
        element = buffer[front];
        size--;
        if (front == rear)
        {
            front = -1;
            rear = -1;
        } // Q has only one element, so we reset the
        // queue after dequeing it. ?
        else
        {
            front = (front + 1) % maxsize;
        }
    }
}

sem_t mutex, full, empty,c;
int count = 0;
int incomingMessages = 10;

void* counter(void* args)
{
    while(1)
    {
        sleep(rand() % 3);
        int num = ((int)args);//index(num) of thread
        printf("\nCounter Thread %d: received a message\n", num);
        printf("\nCounter Thread %d: waiting to write\n", num);
        sem_wait(&c);
        count++;//increase count
        printf("\nCounter Thread %d: now adding to counter, counter value = %d\n", num, count);
        sem_post(&c);


    }
}

void* monitor(void* args)
{

    while(1)
    {

        sleep(rand() % 5);
        int tempCount;
        printf("\nMonitor Thread: waiting to read counter\n");
        sem_wait(&c);
        tempCount = count; //store value of counter to be added to buffer later
        count = 0;//reset counter
        printf("\nMonitor Thread: reading a count value of %d\n", tempCount);
        sem_post(&c);
        int valueFull;
        //check if empty slots =0 then no space in buffer and it's full
        sem_getvalue(&empty,&valueFull);
        if(valueFull==0)
        {
            printf("\nMonitor Thread: buffer is full\n");
        }


        //monitor acts as producer
        sem_wait(&empty);//decrease empty slots by 1,if zero block
        sem_wait(&mutex);//decrease mutex semaphore by 1
        printf("\nMonitor Thread: writing to buffer at position %d\n", rear+1);
        enqueue(tempCount); //add to buffer(critical section)
        sem_post(&mutex);// increase mutex semaphore by 1
        sem_post(&full);//increase number of full slots by 1




    }
}


void* collector(void* args)
{

    while(1)
    {
        sleep(rand() % 5);
        //collector acts as consumer
        int value;
        //check if empty slots =maxsize then full slots are equal zero and buffer is empty
        sem_getvalue(&empty,&value);
        if(value==maxsize)
        {
            printf("\nCollector Thread: buffer is empty\n");
        }
        sem_wait(&full);//decrease full slots by 1,if zero block meaning buffer empty
        sem_wait(&mutex);//decrease mutex semaphore by 1
        printf("\nCollector Thread: reading from buffer at position %d, value = %d\n", front, buffer[front]);

        dequeue();//remove from buffer(critical section)
        sem_post(&mutex);// increase mutex semaphore by 1,if zero wake
        sem_post(&empty);//increase empty slots by 1





    }
}


int main()
{
    srand(time(0));
    //initializeSemaphores
    sem_init(&empty, 0, maxsize);
    sem_init(&full, 0, 0);
    sem_init(&mutex, 0, 1);
    sem_init(&c, 0, 1);

    //creating threads
    pthread_t counterThreads[incomingMessages];
    int index[incomingMessages];

    int i = 0;
    while(i < incomingMessages)
    {
        index[i] = i;
        pthread_create(&counterThreads[i], NULL, counter, &index[i]);
        i++;
    }

    pthread_t monitorThread, collectorThread;

    pthread_create(&monitorThread, NULL, monitor, NULL);
    pthread_create(&collectorThread, NULL, collector, NULL);
    //wait of threads to terminate
    i = 0;
    while(i < incomingMessages)
    {
        pthread_join(counterThreads[i++], NULL);
    }

    pthread_join(monitorThread, NULL);
    pthread_join(collectorThread, NULL);

    return 0;
}
