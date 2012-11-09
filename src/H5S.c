#include "H5S.h"

/* hid_t H5Screate( H5S_class_t type ) */
SEXP _H5Screate( SEXP _type ) {
  H5S_class_t type = INTEGER(_type)[0];
  hid_t hid = H5Screate( type );
  addHandle(hid);

  SEXP Rval;
  PROTECT(Rval = allocVector(INTSXP, 1));
  INTEGER(Rval)[0] = hid;
  UNPROTECT(1);
  return Rval;
}

/* hid_t H5Scopy( hid_t space_id ) */
SEXP _H5Scopy( SEXP _space_id ) {
  hid_t space_id =  INTEGER(_space_id)[0];
  hid_t hid = H5Scopy( space_id );
  addHandle(hid);

  SEXP Rval;
  PROTECT(Rval = allocVector(INTSXP, 1));
  INTEGER(Rval)[0] = hid;
  UNPROTECT(1);
  return Rval;
}

/* herr_t H5Sclose( hid_t space_id ) */
SEXP _H5Sclose( SEXP _space_id ) {
  hid_t space_id =  INTEGER(_space_id)[0];
  herr_t herr = H5Sclose( space_id );
  if (herr == 0) {
    removeHandle(space_id);
  }

  SEXP Rval;
  PROTECT(Rval = allocVector(INTSXP, 1));
  INTEGER(Rval)[0] = herr;
  UNPROTECT(1);
  return Rval;
}

/* hid_t H5Screate_simple( int rank, const hsize_t * dims, const hsize_t * maxdims ) */
SEXP _H5Screate_simple( SEXP _dims, SEXP _maxdims ) {
  hid_t hid;
  int rank = length(_dims);
  hsize_t dims[rank];
  for (int i=0; i<rank; i++) {
    dims[i] = INTEGER(_dims)[i];
  }
  if (length(_maxdims) == 0) {
    hid = H5Screate_simple( rank, dims, NULL);
    addHandle(hid);
  } else {
    if (length(_maxdims) != length(_dims)) {
      printf("dims and maxdims must have the same length.\n");
      hid = -1;
    } else {
      hsize_t maxdims[rank];
      for (int i=0; i<rank; i++) {
	maxdims[i] = INTEGER(_maxdims)[i];
      }
      hid = H5Screate_simple( rank, dims, maxdims);
      addHandle(hid);
    }
  }

  SEXP Rval;
  PROTECT(Rval = allocVector(INTSXP, 1));
  INTEGER(Rval)[0] = hid;
  UNPROTECT(1);
  return Rval;
}

/* htri_t H5Sis_simple( hid_t space_id ) */
SEXP _H5Sis_simple( SEXP _space_id ) {
  hid_t space_id =  INTEGER(_space_id)[0];
  htri_t htri = H5Sis_simple( space_id );

  SEXP Rval;
  PROTECT(Rval = allocVector(INTSXP, 1));
  INTEGER(Rval)[0] = htri;
  UNPROTECT(1);
  return Rval;
}

/* int H5Sget_simple_extent_dims(hid_t space_id, hsize_t *dims, hsize_t *maxdims ) */
SEXP _H5Sget_simple_extent_dims( SEXP _space_id ) {
  hid_t space_id =  INTEGER(_space_id)[0];
  hsize_t   size[H5S_MAX_RANK];
  hsize_t   maxsize[H5S_MAX_RANK];
  int rank = H5Sget_simple_extent_dims(space_id, size, maxsize);

  /* printf("%d ( %d %d )\n", rank, size[0], maxsize[0] ); */
  SEXP Rval = PROTECT(allocVector(VECSXP, 3));
  SET_VECTOR_ELT(Rval,0,ScalarInteger(rank));

  SEXP Rsize;
  SEXP Rmaxsize;
  if (rank < 0) {
    Rsize = PROTECT(allocVector(INTSXP, 0));
    Rmaxsize = PROTECT(allocVector(INTSXP, 0));
    SET_VECTOR_ELT(Rval,1,Rsize);
    SET_VECTOR_ELT(Rval,2,Rmaxsize);
    UNPROTECT(2);
  } else {
    Rsize = PROTECT(allocVector(INTSXP, rank));
    Rmaxsize = PROTECT(allocVector(INTSXP, rank));
    for (int i=0; i < rank; i++) {
      INTEGER(Rsize)[i] = size[i];
      INTEGER(Rmaxsize)[i] = maxsize[i];
    }
    SET_VECTOR_ELT(Rval,1,Rsize);
    SET_VECTOR_ELT(Rval,2,Rmaxsize);
    UNPROTECT(2);
  }

  SEXP names = PROTECT(allocVector(STRSXP, 3));
  SET_STRING_ELT(names, 0, mkChar("rank"));
  SET_STRING_ELT(names, 1, mkChar("size"));
  SET_STRING_ELT(names, 2, mkChar("maxsize"));
  SET_NAMES(Rval, names);
  UNPROTECT(1);
  UNPROTECT(1);
  return(Rval);
}

/* herr_t H5Sselect_hyperslab(hid_t space_id, H5S_seloper_t op, const hsize_t *start, const hsize_t *stride, const hsize_t *count, const hsize_t *block ) */
SEXP _H5Sselect_hyperslab( SEXP _space_id, SEXP _op, SEXP _start, SEXP _stride, SEXP _count, SEXP _block ) {
  hid_t space_id =  INTEGER(_space_id)[0];  
  H5S_seloper_t op =  INTEGER(_op)[0];
  hsize_t start[LENGTH(_start)];
  hsize_t stride[LENGTH(_stride)];
  hsize_t count[LENGTH(_count)];
  hsize_t block[LENGTH(_block)];
  int i;
  for (i=0; i < LENGTH(_start); i++) {
    start[i] = INTEGER(_start)[i];
  }
  for (i=0; i < LENGTH(_stride); i++) {
    stride[i] = INTEGER(_stride)[i];
  }
  for (i=0; i < LENGTH(_count); i++) {
    count[i] = INTEGER(_count)[i];
  }
  for (i=0; i < LENGTH(_block); i++) {
    block[i] = INTEGER(_block)[i];
  }

  herr_t herr = H5Sselect_hyperslab( space_id, op, start, stride, count, block );

  SEXP Rval;
  PROTECT(Rval = allocVector(INTSXP, 1));
  INTEGER(Rval)[0] = herr;
  UNPROTECT(1);
  return Rval;

}

/* H5Sselect_index is not part of the standart H5S interfaces. It is a iteratie call to H5Sselect_point. */
SEXP _H5Sselect_index( SEXP _space_id, SEXP _start, SEXP _count) {
  hid_t space_id =  INTEGER(_space_id)[0];
  int l = LENGTH(_start);

  herr_t herr = H5Sselect_none(space_id);
  hsize_t start[l];
  hsize_t stride[l];
  hsize_t count[l];
  hsize_t block[l];
  int index[l];
  for (int i=0; i<l; i++) {
    stride[i] = 1;
    index[i] = 0;
    block[i] = 1;
  }
  int cont = 1;
  if (herr < 0) {
    cont = 0;
  }
  int k = l-1;
  while(cont > 0) {
    for (int i=0; i<l; i++) {
      start[i] = INTEGER(VECTOR_ELT(_start,i))[index[i]];
      count[i] = INTEGER(VECTOR_ELT(_count,i))[index[i]];
    }
    herr = H5Sselect_hyperslab(space_id, H5S_SELECT_OR, start, stride, count, block);
    if (herr < 0) {
      cont = 0;
    } else {
      k = l-1;
      index[k]++;
      int carry = 0;
      if (index[k] >= LENGTH(VECTOR_ELT(_count,k))) {
	carry = 1;
      }
      while ((k >= 0) & (carry > 0)) {
	index[k] = 0;
	k--;
	if (k >= 0) {
	  index[k]++;
	  if (index[k] >= LENGTH(VECTOR_ELT(_count,k))) {
	    carry = 1;
	  } else {
	    carry = 0;
	  }
	}
      }
      if (k < 0) {
	cont = 0;
      }
    }
  }
  SEXP Rval = ScalarInteger(herr);
  return Rval;
}

