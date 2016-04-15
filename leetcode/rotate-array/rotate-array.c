#include <stdio.h>

void move_zeroes(int *nums, int nums_size) {
    int i, j, zeros;
    j = 0;

    zeros = 0;
    for (; j < nums_size; i++) {
        if (nums[i] != 0) {
            j++;
            continue;
        }

        zeros++;
        j = i + 1;
        for (; j < nums_size; j++) {
            if (nums[j] == 0) {
                zeros++;
                continue;
            }
            nums[i] = nums[j];
            i++;
        }
    }

    for (; zeros > 0; zeros--) {
        nums[nums_size - zeros] = 0;
    }
}

void moveZeroes(int *nums, int numsSize) {
    return move_zeroes(nums, numsSize);
}

void dump_nums(int *nums, int nums_size) {
    int i;
    for (i = 0; i < nums_size; i++) {
        printf("%d ", nums[i]);
    }
    printf("\n");
}

#define LEN(a) sizeof(a) / sizeof((a)[0])
int main() {
    //int nums[] = {1};
    int nums[] = {1, 3, 0, 0, 5, 7, 8, 0, 8};
    dump_nums(nums, LEN(nums));
    moveZeroes(nums, LEN(nums));
    dump_nums(nums, LEN(nums));
    return 0;
}
