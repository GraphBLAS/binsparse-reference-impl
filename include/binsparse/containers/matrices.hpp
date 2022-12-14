#pragma once

namespace binsparse {

struct row_major {
  constexpr bool operator==(row_major) { return true; }
  template <typename T>
  constexpr bool operator==(T&&) { return false; }
};

struct column_major {
  constexpr bool operator==(column_major) { return true; }
  template <typename T>
  constexpr bool operator==(T&&) { return false; }
};

template <typename T, typename I>
struct csr_matrix {
  T* values;
  I* colind;
  I* row_ptr;

  I m, n, nnz;
};

template <typename T, typename I>
struct coo_matrix {
  T* values;
  I* rowind;
  I* colind;

  I m, n, nnz;
};

template <typename T, typename I, typename Order>
struct dense_matrix {
  T* values;
  
  I m, n;

  using order = Order;
};

} // end binsparse