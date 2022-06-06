/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This program demonstrates the use of the merge sort algorithm.  For
 * more information about this and other sorting algorithms, see
 * http://linux.wku.edu/~lamonml/kb.html
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_ITEMS 10

void mergeSort(int numbers[], int temp[], int array_size);
void parallel_merge_sort(int numbers[], int temp[], int array_size);
void m_sort(int numbers[], int temp[], int left, int right);
void merge(int numbers[], int temp[], int left, int mid, int right);

int numbers[NUM_ITEMS];
int temp[NUM_ITEMS];


int main()
{
    int i;
    int p_numbers[NUM_ITEMS], p_temp[NUM_ITEMS];

    //seed random number generator
    srand(getpid());

    //fill array with random integers
    for (i = 0; i < NUM_ITEMS; i++) {
        int buf = rand();
        numbers[i] = buf;
        p_numbers[i] = buf;
    }

    //perform merge sort on array
    mergeSort(numbers, temp, NUM_ITEMS);
    parallel_merge_sort(p_numbers, p_temp, NUM_ITEMS);

    printf("Done with sort.\n");

    printf("merge sort ans");
    for (i = 0; i < NUM_ITEMS; i++) {
        printf("%i\n", numbers[i]);
    }

    printf("parallel merge sort ans");
    for (i = 0; i < NUM_ITEMS; i++) {
        printf("%i\n", p_numbers[i]);
    }

    return 0;
}


void mergeSort(int numbers[], int temp[], int array_size)
{
    m_sort(numbers, temp, 0, array_size - 1);
}

void parallel_merge_sort(int numbers[], int temp[], int array_size) {
    int num1[array_size / 2], num2[array_size - array_size / 2];
    int temp1[array_size / 2], temp2[array_size - array_size / 2];
    for (int i = 0; i < array_size; i++) {
        if (array_size / 2 > i) num1[i] = numbers[i];
        else num2[i - array_size / 2] = numbers[i];
    }

    m_sort(num1, temp1, 0, array_size / 2 - 1);
    m_sort(num2, temp2, 0, array_size - array_size / 2 - 1);

    for (int i = 0; i < array_size; i++) {
        if (array_size / 2 > i) numbers[i] = num1[i];
        else numbers[i] = num2[i - array_size / 2];
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
