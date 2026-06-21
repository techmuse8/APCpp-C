# APCpp-C
A C wrapper for [APCpp](https://github.com/N00byKing/APCpp).

By design, APCpp-C is intended to mirror the original C++ public API via thin wrappers, including function names, types, and general behavior. These are the following things you should keep in mind when using this library:

## Ownership

- Returned pointers should not be freed, as they are views into internal static storage used by the library.
- Data returned is only valid until the next call of the same function. Any important data should be copied beforehand if it must persist.
