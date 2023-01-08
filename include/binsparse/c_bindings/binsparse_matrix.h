//------------------------------------------------------------------------------
// binsparse_matrix.h: a matrix data structure for the C binding of binsparse
//------------------------------------------------------------------------------

// draft by Tim Davis

// I propose that we create a single object, the binsparse_c_matrix, or
// bc_matrix for short, to hold any matrix for the C bindings of the binsparse
// package.

// Name space:  binsparse_c_*?  bc_*?  I'm open to suggestions.  For brevity
// I'm assuming bc_* for now.

#ifndef BINSPARSE_MATRIX_H
#define BINSPARSE_MATRIX_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// If compiling in C++, use `extern "C"` to declare C linkage.
#ifdef __cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
// type codes
//------------------------------------------------------------------------------

// The bc_matrix can hold data types that correspond to the primary C built-in
// types, with a placeholder for future user-defined types.

typedef enum
{
    bc_type_none    = 0,        // no type; values array is NULL (maybe?)
    // bit types
    bc_type_uint1   = 1,        // a single bit
    bc_type_uint2   = 2,        // 2 bits
    bc_type_uint4   = 3,        // 4 bits
    // unsigned types
    bc_type_bool    = 4,        // bool, assume sizeof(bool) == 1, as uint8_t
    bc_type_uint8   = 5,        // uint8_t
    bc_type_uint16  = 6,        // uint16_t
    bc_type_uint32  = 7,        // uint32_t
    bc_type_uint64  = 8,        // uint64_t
    // signed types
    bc_type_int8    = 9,        // int8_t
    bc_type_int16   = 10,       // int16_t
    bc_type_int32   = 11,       // int32_t
    bc_type_int64   = 12,       // int64_t
    bc_type_fp32    = 13,       // float
    bc_type_fp64    = 14,       // double
    bc_type_fc32    = 15,       // float complex
    bc_type_fc64    = 16,       // double complex
    // user-defined type
    bc_type_user    = 17,       // user-defined type
}
bc_type_code ;

//------------------------------------------------------------------------------
// the bc_matrix: a sparse matrix or vector of any type
//------------------------------------------------------------------------------

// Each axis k of a given n-D matrix can be in one of four formats,
// listed in increasing order of sparsity:
//
// pointer[k]   index[k]    Name and description
// ----------   --------    --------------------
//
// NULL         non-NULL    "Index": some entries present.
//                          indices need not be in order, nor unique.
//                          size of index [k] array is nindex [k].
//                          in_order [k] can be true or false.
//
// non-NULL     non-NULL    "Hyper": some entries present.
//                          indices must be in order and unique.
//                          index [k] has size nindex [k]
//                          pointer [k] has size nindex [k]+1 and must be
//                          monotonically non-decreasing.
//                          in_order [k] must be true.
//
// non-NULL     NULL        "Sparse": all entries present.
//                          pointer [k] has size dim [k]+1.
//                          nindex [k] not used (or can be set to
//                          dim [k] for consistency).
//                          in_order [k] must be true.
//
// NULL         NULL        "Full": all entries present,
//                          nindex [k] not used (or can be set to
//                          dim [k] for consistency).
//                          in_order [k] must be true.

// The matrix format is determined by the presence of pointer [0:rank-1]
// and index [0:rank-1] (NULL or non-NULL).  There need not be any format
// enum.

// Common formats:

// rank = 0:    a scalar, no arrays present.  nvals = 0 or 1

// rank = 1:    a 1-D vector of dimension n
//
//      axis[0].order = { 0 }
//      axis[0].dim  = { n }

//      sparse vector (COO-style):  Format is (Index)
//          axis[0].pointer  = NULL
//          axis[0].index    = [list of nvals indices] of size axis [0].nindex
//          axis[0].in_order = true if indices in order, false otherwise
//          axis[0].nindex   = nvals
//          values = [list of nvals values], or size 1 if iso

//      full vector: Format is (Full)
//          axis[0].pointer  = NULL
//          axis[0].index    = NULL
//          axis[0].in_order = true
//          axis[0].nindex   = n
//          values = size n, or size 1 if iso

// rank = 2:    a 2-D matrix of dimension m-by-n

//      axis[0:1].order = { 0, 1 } if stored by-row
//      axis[0:1].order = { 1, 0 } if stored by-column
//      axis[0:1].dim   = { m, n } if by row
//      axis[0:1].dim   = { n, m } if by col

//      COO: Format is (Index, Index)
//
//          axis[0].pointer  = NULL
//          axis[0].index    = row indices if by-row, col indices if by-col
//          axis[0].in_order = true if index [0] in order, false otherwise
//          axis[0].nindex   = nvals
//
//          axis[1].pointer  = NULL
//          axis[1].index    = col indices if by-row, row indices if by-col
//          axis[1].in_order = true if index [1] in order, false otherwise
//          axis[0].nindex   = nvals
//
//          values: size nvals, or 1 if iso

//      CSR: Format is (Sparse, Index)
//
//          axis[0:1].order = { 0, 1 }, stored by-row
//          axis[0:1].dim   = { m, n }
//
//          axis[0].pointer  = non-NULL, of size m+1
//          axis[0].index    = NULL
//          axis[0].in_order = true
//          axis[0].nindex   = axis [0].dim = m
//
//          axis[1].pointer  = NULL
//          axis[1].index    = col indices, size nvals
//          axis[1].in_order = true if index [1] in order, false otherwise
//          axis[1].nindex   = nvals
//
//          values: size nvals, or 1 if iso

//      CSC: Format is (Sparse, Index)
//
//          axis[0:1].order = { 1, 0 }, stored by-col
//          axis[0:1].dim   = { n, m }
//
//          axis[0].pointer  = non-NULL, of size n+1
//          axis[0].index    = NULL
//          axis[0].in_order = true
//          axis[0].nindex   = axis [0].dim = n
//
//          axis[1].pointer  = NULL
//          axis[1].index    = row indices, size nvals
//          axis[1].in_order = true if index [1] in order, false otherwise
//          axis[1].nindex   = nvals
//
//          values: size nvals, or 1 if iso

//      DCSR: Format is (Hyper, Index): hypersparse-by-row
//
//          axis[0:1].order = { 0, 1 }, stored by-row
//          axis[0:1].dim   = { m, n }
//
//          axis[0].pointer  = non-NULL, of size axis[0].ninidex[0]+1
//          axis[0].index    = non-NULL, of size axis[0].ninidex[0]
//          axis[0].in_order = true
//          axis[0].nindex   = # of non-empty rows
//
//          axis[1].pointer  = NULL
//          axis[1].index    = col indices, size nvals
//          axis[1].in_order = true if index [1] in order, false otherwise
//          axis[1].nindex   = nvals
//
//          values: size nvals, or 1 if iso

//      DCSC: Format is (Hyper, Index): hypersparse-by-col
//
//          axis[0:1].order = { 1, 0 }, stored by-col
//          axis[0:1].dim   = { n, m }
//
//          axis[0].pointer  = non-NULL, of size axis[0].ninidex[0]+1
//          axis[0].index    = non-NULL, of size axis[0].ninidex[0]
//          axis[0].in_order = true
//          axis[0].nindex   = # of non-empty cols
//
//          axis[1].pointer  = NULL
//          axis[1].index    = row indices, size nvals
//          axis[1].in_order = true if index [1] in order, false otherwise
//          axis[1].nindex   = nvals
//
//          values: size nvals, or 1 if iso

//      full: Format is (Full, Full): full-by-row
//
//          axis[0:1].order = { 0, 1 }, stored by-row
//          axis[0:1].dim   = { m, n }
//
//          axis[0].pointer  = NULL
//          axis[0].index    = NULL
//          axis[0].in_order = true
//          axis[0].nindex   = axis [0].dim = m
//
//          axis[1].pointer  = NULL
//          axis[1].index    = NULL
//          axis[1].in_order = true
//          axis[1].nindex   = axis [0].dim = n
//
//          values: size nvals = m*n, or 1 if iso

//      full: Format is (Full, Full): full-by-col
//
//          axis[0:1].order = { 1, 0 }, stored by-col
//          axis[0:1].dim   = { n, m }
//
//          axis[0].pointer  = NULL
//          axis[0].index    = NULL
//          axis[0].in_order = true
//          axis[0].nindex   = axis [0].dim = n
//
//          axis[1].pointer  = NULL
//          axis[1].index    = NULL
//          axis[1].in_order = true
//          axis[1].nindex   = axis [0].dim = m
//
//          values: size nvals = m*n, or 1 if iso

//      Index-Full format (held by row: each row is either full or all empty)
//          Format is (Index, Full)
//
//          axis[0:1].order = { 0, 1 }, stored by-row
//          axis[0:1].dim   = { m, n }
//
//          axis[0].pointer  = NULL
//          axis[0].index    = list of rows, of size axis [0].nindex
//          axis[0].in_order = true
//          axis[0].nindex   = # of non-empty rows
//
//          axis[1].pointer  = NULL
//          axis[1].index    = NULL
//          axis[1].in_order = true
//          axis[1].nindex   = axis [0].dim = n
//
//          values: size nvals = axis[0].index*n, or 1 if iso

//      Index-Full format (held by col: each col is either full or all empty)
//          Format is (Index, Full)
//
//          axis[0:1].order = { 1, 0 }, stored by-col
//          axis[0:1].dim   = { n, m }
//
//          axis[0].pointer  = NULL
//          axis[0].index    = list of cols, of size axis [0].nindex
//          axis[0].in_order = true
//          axis[0].nindex   = # of non-empty cols
//
//          axis[1].pointer  = NULL
//          axis[1].index    = NULL
//          axis[1].in_order = true
//          axis[1].nindex   = axis [0].dim = m
//
//          values: size nvals = axis[0].index*m, or 1 if iso

//      Are all 16 formats possible?  NO.  I think there are only 5.

//          (Full, Sparse, Hyper, Index) x (Full, Sparse, Hyper, Index) ?
//          I think the last axis must be Full or Index, which leads to
//          8 formats: (Full, Sparse, Hyper, Index) x (Full, Index).
//          5 Listed above are:
//
//              (Index, Index)      COO
//              (Sparse, Index)     CSR and CSC
//              (Hyper, Index)      hypersparse by row or col
//              (Full, Full)        full
//              (Index, Full)       can be defined, looks useful.  An unordered
//                                  or ordered set of full vectors.
//
//          not described above:  either not useful or not valid
//
//              (Hyper, Full)       not useful (see rule 4 below)
//              (Sparse, Full)      can be defined but not useful?
//                                  see rule 3 below
//              (Full, Index)       huh?  See Rule (1) below.
//
//      bitmap format: held as two full bc_matrices with same dimension and
//          axis_order.  The first matrix ('bitmap' pattern) is always bool.
//          The second full matrix holds the values.
//          Format of both bc_matrices is (Full, Full)

// I think a few rules might work:
//
// (1) from left to right, if a "Full" format appears, the remaining
//      formats must all be Full.
//
// (2) the last format must be "Index" or "Full".
//
// (3) (... ,Sparse, Full, ...) can be defined but is not useful. It would be
//      the less storage cost and same representation as ( ... Full, Full, ...).
//      So do not support this format.
//
// (4) (... , Hyper, Full, ...) can be defined but is not useful.  The
//      same thing can be done with (... , Index, Full, ...) where the Index
//      axis is sorted.  There's no need for the pointer for the Hyper
//      axis, since all objects to the right have the same size.
//
// (5) Like rule 1, once "Index" appears, the remaining formats to the right
//      must be "Index" or "Full".  This is because "Index" has no pointer so
//      all formats to the right must have a known size, or be a list like
//      (Index, Index, Full) where the total size is given nindex [...].

/*
LANGUAGE OF VALID FORMATS

These 5 rules lead to a simple finite-state machine that descibes the language
of valid formats.  The starting state (0th rank) can be any of the four
formats.  Each state has a self-loop (not shown).  The end state of the
language must be Index or Full.

    POINTER PRESENT                     |   NO POINTER
    the objects in the (k+1)st          |   the kth axis and all axes to the
    axis are of arbitary size           |   right must be a list of objects of
                                        |   fixed size

    "Sparse"                                "Index"
    (pointer present  ------------------->  no pointer
    no index.                               index present
    size is                                 size is
    dim [k]   <---\                  /--->  nindex[k]
           \       \                /                 \
            \       \              /                   \
             \       \            /                     \
              \       \          /                       \
               \        "Hyper" /                         --->  "Full"
                \-----> (both pointer                           no pointer
                        and index.                              no index
                        size is                                 size is
                        nindex [k]                              dim [k]

                    |                                       |
                    |                                       |
    NO INDEX        |           INDEX IS PRESENT            |   NO INDEX
    must be         |       in order if axis[k].in_order    |   must be
    in order        |       is true, unordered if false     |   in order


That is, the format can start with any mix of Sparse and/or Hyper (or none of
them), in any order.  These formats have pointers so the size of the objects to
the right of them can vary in size.

The Sparse and Hyper formats have a pointer, so the objects they describe to
the right of them in axis k+1 have variable sizes.

The Index and Full formats have no pointer, so the objects they describe
in their axes and the axes to the right of them must have a fixed size.

The Sparse and Full formats have no index, so their own size must be dim [k]
if they describe the kth axis.  "Sparse" is short-hand for a dense list of
objects, each of variable size.  "Full" is short-hand for a dense list of
objects of fixed size.

*/

// rank = 3
//
//      describe some for future extensions.  12 possible formats:

//      (Index , Index , Index)     all COO

//      (Hyper , Index , Index)     1D hyperlist of 2D COO matrices
//      (Hyper , Hyper , Index)     1D hyperlist of 2D hypersparse mtx
//      (Hyper , Sparse, Index)     1D hyperlist of 2D CSR/CSC matrices

//      (Sparse, Index , Index)     1D dense array of 2D COO matrices
//      (Sparse, Hyper , Index)     1D dense array of 2D hypersparse
//      (Sparse, Sparse, Index)     1D dense array of 2D CSR/CSR matrices

//      (Index , Index , Full )     like 2D COO, but each "entry" is an entire
//                                  dense vector
//      (Index , Full  , Full )     a 1D list of 2D full matrices

//      (Hyper , Index , Full )     2D hypersparse, each entry a full vec

//      (Sparse, Index , Full )     1D dense array of 2D (Index,Full) matrices

//      (Full  , Full  , Full )     a 3D full matrix

typedef struct
{
    uint8_t order ;             // axis ordering, in the range 0 to rank-1.
                                // axis [k].order is a permutation of 0:rank-1

    uint64_t dimension ;        // size of this dimension of the matrix

    bool in_order ;             // axis.in_order [k] is true if the kth axis
                                // appears in strictly ascending order


    void *pointer ;             // set of pointers, of type pointer_type
    size_t pointer_size ;       // allocated size of each pointer[k] array
                                // pointer [k] has nindex [k]+1 entries.

    void *index ;               // array of indices, of type index_type
    uint64_t nindex ;           // index [k] has nindex [k] entries
    size_t index_size ;         // allocated size of each index[k] array

}
bc_axis_struct ;


typedef struct
{

    // basic information:  dimensions, type, and format

    int64_t magic ;             // for detecting uninitialized objects

    size_t header_size ;        // allocated size of this bc_matrix_struct,
                                // in bytes

    int rank ;                  // 0: scalar, 1:vector, 2:matrix,
                                // 3: 3D tensor, etc.

    bc_type_code pointer_type ; // matrix 'pointer' type (any integer type)

    bc_type_code index_type ;   // matrix index type (any integer type)

    bc_type_code value_type ;   // matrix value type (bool, int8, ...).

    bool iso_valued ;           // if true, all entries have the same value,
                                // and thus only values [0] is used.

    size_t type_size ;          // sizeof (value type), that is, sizeof (bool),
                                // sizeof (int8), ...  Allows extension to
                                // user-defined types.

    char *json_string ;         // metadata (may be NULL).  Allows future
                                // extension to user-defined types, with a json
                                // string.
    size_t json_string_size ;   // allocated size of json_string, in bytes

    // matrix content: an array of structs

    bc_axis_struct *axis ;      // an array axis [0..rank-1] where axis [k]
                                // is the kth axis of the matrix/tensor

    // matrix content: values

    void *values ;              // array of values, of type value_type
                                // size 1 if iso, at least size nvals otherwise
    size_t values_size ;        // allocated size of values array, in bytes
    uint64_t nvals ;            // # of values present

}
bc_matrix_struct ;

// a bc_matrix is a pointer to the bc_matrix_struct shown above
typedef bc_matrix_struct *bc_matrix ;

#ifdef __cplusplus
}
#endif

#endif
