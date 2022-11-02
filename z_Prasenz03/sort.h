
#pragma once  //what is this?  防止 include 相互 索引 deadloop？

/// @brief Compares the values pointed to by the given pointers as integers
/// @param a Pointer to frist value
/// @param b Pointer to second value
/// @return 1 if first value is larger, -1 if first value is smaller, 0 if both values are equal.
static int some_sorting_function(const void * a, const void * b);

/// @brief Prints decimal values given as integer array
/// @param arr Array containing values
/// @param arr_size Number of elements in array
inline void print(int * arr, int arr_size);