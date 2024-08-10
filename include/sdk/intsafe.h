#pragma once
#define UINT_ERROR 0xffffffff
#define INTSAFE_E_ARITHMETIC_OVERFLOW ((HRESULT)0x80070216L) // 0x216 = 534 = ERROR_ARITHMETIC_OVERFLOW

_Must_inspect_result_ __inline HRESULT UIntAdd(_In_ UINT uAugend, _In_ UINT uAddend,
                                               _Out_ _Deref_out_range_(==, uAugend + uAddend) UINT *puResult) {
    HRESULT hr;

    if ((uAugend + uAddend) >= uAugend) {
        *puResult = (uAugend + uAddend);
        hr = S_OK;
    } else {
        *puResult = UINT_ERROR;
        hr = INTSAFE_E_ARITHMETIC_OVERFLOW;
    }

    return hr;
}

inline int wcscmp(const wchar_t *s1, const wchar_t *s2) {

    while (*s1 == *s2++)
        if (*s1++ == 0)
            return (0);
    return (*s1 < *--s2 ? -1 : 1);
}
