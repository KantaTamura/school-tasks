#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define NUM_ITEMS 10

void mergeSort(int numbers[], int temp[], int array_size);
void m_sort(int numbers[], int temp[], int left, int right);
void merge(int numbers[], int temp[], int left, int mid, int right);

int numbers[NUM_ITEMS];
int temp[NUM_ITEMS];

int main()
{
    int i;

    //seed random number generator
    srand(getpid());

    //fill array with random integers
    for (i = 0; i < NUM_ITEMS; i++) numbers[i] = rand();
    //perform merge sort on array
    clock_t start, end;
    start = clock();
    mergeSort(numbers, temp, NUM_ITEMS);
    end = clock();

    printf("Done with sort.\n");

    for (i = 0; i < NUM_ITEMS; i++) {
        printf("%i\n", numbers[i]);
    }

    printf("sort time : %lf\n", (double)(end - start) / CLOCKS_PER_SEC);

    return 0;
}


void mergeSort(int numbers[], int temp[], int array_size)
{
    int left = 0;
    int right = array_size - 1;
    int mid = (right + left) / 2;

    int buf[NUM_ITEMS];

    int fd[2];
    int pid, status;
    if (pipe(fd) == -1) {
        perror("pipe failed.");
        exit(1);
    }
    if ((pid = fork()) == -1) {
        perror("fork failed.");
        exit(1);
    }
    // Child process
    if (pid == 0) {
        close(fd[0]);
        m_sort(numbers, temp, left, mid);
        if (write(fd[1], numbers, sizeof(buf)) == -1) {
            perror("pepe write.");
            exit(1);
        }
        exit(0);
    }
    // Parent process
    else {
        close(fd[1]);
        m_sort(numbers, temp, mid + 1, right);
        wait(&status);
        if (read(fd[0], buf, sizeof(buf)) == -1) {
            perror("pipe read.");
            exit(1);
        }
        for (int i = left; i <= mid; i++) numbers[i] = buf[i];
    }
    merge(numbers, temp, 0, (array_size - 1) / 2 + 1, array_size - 1);
}

void m_sort(int numbers[], int temp[], int left, int right)
{
    int mid;

    if (right > left)
    {
        mid = (right + left) / 2;
        m_sort(numbers, temp, left, mid);
        m_sort(numbers, temp, mid + 1, right);

        merge(numbers, temp, left, mid + 1, right);
    }
}


void merge(int numbers[], int temp[], int left, int mid, int right)
{
    int i, left_end, num_elements, tmp_pos;

    left_end = mid - 1;
    tmp_pos = left;
    num_elements = right - left + 1;

    while ((left <= left_end) && (mid <= right))
    {
        if (numbers[left] <= numbers[mid])
        {
            temp[tmp_pos] = numbers[left];
            tmp_pos = tmp_pos + 1;
            left = left + 1;
        }
        else
        {
            temp[tmp_pos] = numbers[mid];
            tmp_pos = tmp_pos + 1;
            mid = mid + 1;
        }
    }

    while (left <= left_end)
    {
        temp[tmp_pos] = numbers[left];
        left = left + 1;
        tmp_pos = tmp_pos + 1;
    }
    while (mid <= right)
    {
        temp[tmp_pos] = numbers[mid];
        mid = mid + 1;
        tmp_pos = tmp_pos + 1;
    }

    for (i = 0; i <= num_elements; i++)
    {
        numbers[right] = temp[right];
        right = right - 1;
    }
}
