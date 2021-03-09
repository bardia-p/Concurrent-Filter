# Concurrent Filtering Program

by Bardia Parmoun

Released on: 9/03/2021

## DESCRIPTION
- This program is used to filter an unordered array of characters consisting of letters and numbers
- It uses Linux semaphores and shared memory to achieve concurrency
- The unordered list is sorted in the way that all the letters are on the left and all the numbers are on the right side

## RUNNING THE PROGRAM
- This programs needs to be tested in a Linux environment. 
- Requires a gcc version of 9.3.1 or higher
1. To run the program first run the makefile to compile the executable file by typing:
```shell
$ make
```
2. After that an executable with the name of MMULT has been generated which  can be run using:
```shell
$ ./FILTER
```

## USING THE PROGRAM
1. After running the program you will be prompted by the program to enter the mode that you want to run the program in:
Enter 1 for debug mode and 0 for regular mode:

```
Do you want to run the program in debug mode? Enter 1 for yes
1
```

2. Then you can enter your string of characters and integers separated by space like the following:
```
1 2 3 C D E F
```

The program can be customized to work for arrays of different sizes. This can be achieved by navigating to the FILTER.h file and 
changing the value of ARRAY_SIZE to your desired value


## Testing the program in non debug mode:
### Testing the first given testcase
```
Do you want to run the program in debug mode? Enter 1 for yes
0
Initialize the array: separate values by space
5 A 9 M W 6 Z
[ A, M, W, Z, 5, 9, 6,]
```
As you can see all the letters were moved to the left side and the numbers to the right side

### Testing the second given testcase (the array is filtered in the reverse order)
```
Do you want to run the program in debug mode? Enter 1 for yes
0
Initialize the array: separate values by space
1 2 3 C D E F
[ C, D, E, F, 1, 2, 3,]
```
As you can see the program was able to properly filter the array in the other direction

### Testing with values characters and numbers being one apart
```
Do you want to run the program in debug mode? Enter 1 for yes
0
Initialize the array: separate values by space
A 1 B 2 C 3 D
[ A, B, C, D, 1, 2, 3,]
```
The program was once again able to properly filter the program

### Testing with all all letters only
```
Do you want to run the program in debug mode? Enter 1 for yes
0
Initialize the array: separate values by space
A B C D E F G
[ A, B, C, D, E, F, G,]
```
The array was left unchanged since it is already filtered 

### Testing with numbers only
```
Do you want to run the program in debug mode? Enter 1 for yes
0
Initialize the array: separate values by space
1 2 3 4 5 6 7
[ 1, 2, 3, 4, 5, 6, 7,]
```
The array was left unchanged since it is already filered

## Testing the program with an already filtered array
```
Do you want to run the program in debug mode? Enter 1 for yes
0
Initialize the array: separate values by space
C D E F 1 2 3
[ C, D, E, F, 1, 2, 3,]
```
The array was left unchanged since it was already filtered

## Testing the program in debug mode
### Testing the first testcase
```
Do you want to run the program in debug mode? Enter 1 for yes
1
Initialize the array: separate values by space
5 A 9 M W 6 Z
Process: 1
Process: 2
P1 swapped A and 5
P1 Did not swap
P2 swapped M and 9
Process: 3
P1 swapped M and 5
P3 swapped Z and 6
P2 swapped W and 9
P1 Did not swap
P2 swapped W and 5
P3 swapped Z and 9
P3 swapped Z and 5
P1 Did not swap
P3 Did not swap
P1 Did not swap
P2 Did not swap
Final Result:
[ A, M, W, Z, 5, 9, 6,]
```
The swapping actions were properly divider between all the processes and the program only stops executing when all the processes have stopped executing

### Testing the second testcase
```
Do you want to run the program in debug mode? Enter 1 for yes
1
Initialize the array: separate values by space
1 2 3 C D E F
Process: 1
Process: 2
P1 Did not swap
P1 Did not swap
Process: 3
P1 Did not swap
P2 swapped C and 3
P1 swapped C and 2
P3 swapped D and 3
P1 swapped C and 1
P3 swapped E and 3
P3 swapped F and 3
P1 Did not swap
P2 swapped D and 2
P3 swapped E and 2
P1 swapped D and 1
P3 swapped F and 2
P2 swapped E and 1
P1 Did not swap
P3 swapped F and 1
P1 Did not swap
P3 Did not swap
P1 Did not swap
P2 Did not swap
Final Result:
[ C, D, E, F, 1, 2, 3,]
```

## Error handling
Note that in the default case for this program the length of the array is defined as 7 so any input that has less than 7 elements will be disregarded and for inputs with more than 7 values only the first 7 inputs are considered. The input is only allowed to have UPPERCASE letters and digits so and anything other than these values will be result in the program exiting and considering it as an incorrect input. Here are some examples of incorrect inputs:

Having less than 7 elements:
```
Do you want to run the program in debug mode? Enter 1 for yes
0
Initialize the array: separate values by space
1 2 3 C D G
There was an error with the input
```

Not using uppercase letters:
```
Do you want to run the program in debug mode? Enter 1 for yes
0
Initialize the array: separate values by space
1 a v 3 e 5 t
There was an error with the input
```

Using numbers are than digits/special characters:
```
Do you want to run the program in debug mode? Enter 1 for yes
0
Initialize the array: separate values by space
1 2 D ! S D 3
There was an error with the input
```

## CREDITS
Author: Bardia Parmoun

Copyright © 2021 Bardia Parmoun. All rights reserved
