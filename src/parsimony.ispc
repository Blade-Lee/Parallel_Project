export void array_copy_ispc(uniform int arr_len, uniform int input[], uniform int output[]) {
    foreach (i = 0 ... arr_len) {
        output[i] = input[i];
    }
}