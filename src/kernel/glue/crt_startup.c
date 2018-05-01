//
// Kernel Glue
//
void __attribute__((weak)) _init(void)
{
	/*
	* These don't have to do anything since we use init_array/fini_array.
	*/
}

/**
* @brief      Dummy function for __libc_fini_array called
*/
void __attribute__((weak)) _fini(void)
{
	/*
	* These don't have to do anything since we use init_array/fini_array.
	*/
}