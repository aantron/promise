// We need extra C helpers to access many libuv data structures. This is for two
// reasons:
// - libuv leaves their exact layouts undefined, so we can't use cstruct or
//   ctypes to generate code for manipulating these structures.
// - libuv lists fields that these structures are guaranteed to have, but does
//   not provide accessor functions for many of the fields. As a result, we
//   can't easily call pre-existing functions using ctypes.
//
// Also, because the layout is unspecified, this file has to define allocation
// functions. They must be defined in terms of the C type names.
//
// This file basically defines the missing accessors for the rest of the library
// to then call using ctypes.
//
// The helpers are grouped by type, and the types are listed alphabetically.



#include <stdlib.h>
#include <string.h>
#include <uv.h>



// Utility: uv_buf_t.

uv_buf_t* ocaml_luv_allocate_uv_buf_ts(int count)
{
    return malloc(sizeof(uv_buf_t) * count);
}

void ocaml_luv_set_uv_buf_t(
    uv_buf_t *buffers, int index, char *base, unsigned int length)
{
    buffers[index] = uv_buf_init(base, length);
}

char* ocaml_luv_duplicate_buffer(char *buffer, int length)
{
    char *copy = malloc(length);
    if (copy == NULL)
        return NULL;

    memcpy(copy, buffer, length);

    return copy;
}



// Filesystem: uv_fs_t.

uv_fs_t* ocaml_luv_allocate_uv_fs_t()
{
    return malloc(sizeof(uv_fs_t));
}

// TODO This should return a ssize_t really, but we are going to use C to
// truncate it for now.
int ocaml_luv_uv_fs_t_get_result(uv_fs_t *request)
{
    return request->result;
}



// Jbuilder+Ctypes workaround. See
//   https://github.com/janestreet/jbuilder/issues/135
void ocaml_luv_jbuilder_workaround()
{
}
