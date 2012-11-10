#include "H5D.h"
#include <stdlib.h>
#include <time.h>

/* hid_t H5Dcreate( hid_t loc_id, const char *name, hid_t dtype_id, hid_t space_id, hid_t lcpl_id, hid_t dcpl_id, hid_t dapl_id ) */
/* TODO more parameters: hid_t lcpl_id, hid_t dcpl_id, hid_t dapl_id */
SEXP _H5Dcreate( SEXP _loc_id, SEXP _name, SEXP _dtype_id, SEXP _space_id, SEXP _cdim, SEXP _level ) {
  hid_t loc_id = INTEGER(_loc_id)[0];
  const char *name = CHAR(STRING_ELT(_name, 0));
  hid_t dtype_id = INTEGER(_dtype_id)[0];
  hid_t space_id =  INTEGER(_space_id)[0];

  hid_t plist = H5P_DEFAULT;
  int rank = length(_cdim);
  if (rank > 0) {
    hsize_t cdim[rank];
    for (int i=0; i < rank; i++) {
      cdim[i] = INTEGER(_cdim)[i];
    }
    unsigned int level = 6;
    if (length(_level) > 0) {
      level = (unsigned int)INTEGER(_level)[0];
    }
    plist = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_fill_time( plist, H5D_FILL_TIME_ALLOC );
    H5Pset_chunk(plist, rank, cdim);
    if (level > 0) {
      H5Pset_deflate( plist, level );
    }
  }

  hid_t hid = H5Dcreate( loc_id, name, dtype_id, space_id, 
			 H5P_DEFAULT, plist, H5P_DEFAULT );
  addHandle(hid);
 
  SEXP Rval;
  PROTECT(Rval = allocVector(INTSXP, 1));
  INTEGER(Rval)[0] = hid;
  UNPROTECT(1);
  return Rval;
}

/* hid_t H5Dopen( hid_t loc_id, const char *name, hid_t dapl_id ) */
/* TODO more parameters: hid_t dapl_id */
SEXP _H5Dopen( SEXP _loc_id, SEXP _name ) {
  hid_t loc_id = INTEGER(_loc_id)[0];
  const char *name = CHAR(STRING_ELT(_name, 0));
  hid_t hid = H5Dopen( loc_id, name, H5P_DEFAULT );
  addHandle(hid);

  SEXP Rval;
  PROTECT(Rval = allocVector(INTSXP, 1));
  INTEGER(Rval)[0] = hid;
  UNPROTECT(1);
  return Rval;
}

/* herr_t H5Dclose(hid_t dataset_id ) */
SEXP _H5Dclose( SEXP _dataset_id ) {
  hid_t dataset_id = INTEGER(_dataset_id)[0];
  herr_t herr = H5Dclose( dataset_id );
  if (herr == 0) {
    removeHandle(dataset_id);
  }

  SEXP Rval;
  PROTECT(Rval = allocVector(INTSXP, 1));
  INTEGER(Rval)[0] = herr;
  UNPROTECT(1);
  return Rval;
}

void getMemSpaceDim( hid_t file_space_id, hsize_t *size) {
  hssize_t sel_hyper_nblocks = H5Sget_select_hyper_nblocks( file_space_id );
  int rank = H5Sget_simple_extent_ndims( file_space_id );
  hsize_t sizebuf[2*rank*sel_hyper_nblocks];
  H5Sget_select_hyper_blocklist(file_space_id, 0, sel_hyper_nblocks, sizebuf );

  int isnew;
  for (int i=0; i < rank; i++) {
    size[i] = 0;
    for (int j=0; j < sel_hyper_nblocks; j++) {
      isnew = 1;
      for (int k=0; k < j; k++) {
	if ((sizebuf[j*2*rank+i] == sizebuf[k*2*rank+i]) 
	    & (sizebuf[j*2*rank+i+rank] == sizebuf[k*2*rank+i+rank])) {
	  isnew=0;
	}
      }
      if (isnew != 0) {
	size[i] += (sizebuf[j*2*rank+i+rank] - sizebuf[j*2*rank+i] + 1);
      }
    }
  }
}

SEXP H5Dread_helper_INTEGER(hid_t dataset_id, hid_t file_space_id, hid_t mem_space_id, hsize_t n, SEXP Rdim, SEXP _buf, 
			    hid_t dtype_id, hid_t cpdType, int cpdNField, char ** cpdField, int compoundAsDataFrame ) {
  hid_t mem_type_id = -1;

  SEXP Rval;
  if (cpdType < 0) {
    mem_type_id = H5T_NATIVE_INT;
  } else {
    mem_type_id = H5Tcreate(H5T_COMPOUND, H5Tget_size(H5T_NATIVE_INT));
    herr_t status = H5Tinsert(mem_type_id, cpdField[0], 0, H5T_NATIVE_INT);
    for (int i=1; i<cpdNField; i++) {
      hid_t mem_type_id2 = H5Tcreate(H5T_COMPOUND, H5Tget_size(H5T_NATIVE_INT));
      herr_t status = H5Tinsert(mem_type_id2, cpdField[i], 0, mem_type_id);
      mem_type_id = mem_type_id2;
    }
  }
  void * buf;
  if (length(_buf) == 0) {
    Rval = PROTECT(allocVector(INTSXP, n));
    buf = INTEGER(Rval);
  } else {
    buf = INTEGER(_buf);
    Rval = _buf;
  }
  herr_t herr = H5Dread(dataset_id, mem_type_id, mem_space_id, file_space_id, H5P_DEFAULT, buf );
  if (length(_buf) == 0) {
    setAttrib(Rval, R_DimSymbol, Rdim);
    UNPROTECT(1);
  }

  return(Rval);
}


SEXP H5Dread_helper_FLOAT(hid_t dataset_id, hid_t file_space_id, hid_t mem_space_id, hsize_t n, SEXP Rdim, SEXP _buf, 
			    hid_t dtype_id, hid_t cpdType, int cpdNField, char ** cpdField, int compoundAsDataFrame ) {
  hid_t mem_type_id = -1;

  SEXP Rval;
  if (cpdType < 0) {
    mem_type_id = H5T_NATIVE_DOUBLE;
  } else {
    mem_type_id = H5Tcreate(H5T_COMPOUND, H5Tget_size(H5T_NATIVE_DOUBLE));
    herr_t status = H5Tinsert(mem_type_id, cpdField[0], 0, H5T_NATIVE_DOUBLE);
    for (int i=1; i<cpdNField; i++) {
      hid_t mem_type_id2 = H5Tcreate(H5T_COMPOUND, H5Tget_size(H5T_NATIVE_DOUBLE));
      herr_t status = H5Tinsert(mem_type_id2, cpdField[i], 0, mem_type_id);
      mem_type_id = mem_type_id2;
    }
  }
  void * buf;
  if (length(_buf) == 0) {
    Rval = PROTECT(allocVector(REALSXP, n));
    buf = REAL(Rval);
  } else {
    buf = REAL(_buf);
    Rval = _buf;
  }
  herr_t herr = H5Dread(dataset_id, mem_type_id, mem_space_id, file_space_id, H5P_DEFAULT, buf );
  if (length(_buf) == 0) {
    setAttrib(Rval, R_DimSymbol, Rdim);
    UNPROTECT(1);
  }
  return(Rval);
}

SEXP H5Dread_helper_STRING(hid_t dataset_id, hid_t file_space_id, hid_t mem_space_id, hsize_t n, SEXP Rdim, SEXP _buf, 
			    hid_t dtype_id, hid_t cpdType, int cpdNField, char ** cpdField, int compoundAsDataFrame ) {
  hid_t mem_type_id = -1;

  SEXP Rval;
  size_t size = H5Tget_size(dtype_id);
  if (cpdType < 0) {
    mem_type_id = dtype_id;
  } else {
    mem_type_id = H5Tcreate(H5T_COMPOUND, size);
    herr_t status = H5Tinsert(mem_type_id, cpdField[0], 0, dtype_id);
    for (int i=1; i<cpdNField; i++) {
      hid_t mem_type_id2 = H5Tcreate(H5T_COMPOUND, size);
      herr_t status = H5Tinsert(mem_type_id2, cpdField[i], 0, mem_type_id);
      mem_type_id = mem_type_id2;
    }
  }
  Rval = PROTECT(allocVector(STRSXP, n));

  if (H5Tis_variable_str(dtype_id)) {
    char *bufSTR[n];
    herr_t herr = H5Dread(dataset_id, mem_type_id, mem_space_id, file_space_id, H5P_DEFAULT, bufSTR );

    for (int i=0; i<n; i++) {
      SET_STRING_ELT(Rval, i, mkChar(bufSTR[i]));
      free(bufSTR[i]);
    }
  } else {
    char bufSTR[n][size];
    herr_t herr = H5Dread(dataset_id, mem_type_id, mem_space_id, file_space_id, H5P_DEFAULT, bufSTR );
    char bufSTR2[n][size+1];
    for (int i=0; i<n; i++) {
      for (int j=0; j<size; j++) {
	bufSTR2[i][j] = bufSTR[i][j];
      }
      bufSTR2[i][size] = '\0';
    }

    for (int i=0; i<n; i++) {
      SET_STRING_ELT(Rval, i, mkChar(bufSTR2[i]));
    }
  }
  setAttrib(Rval, R_DimSymbol, Rdim);
  UNPROTECT(1);
  return(Rval);
}

SEXP H5Dread_helper_ENUM(hid_t dataset_id, hid_t file_space_id, hid_t mem_space_id, hsize_t n, SEXP Rdim, SEXP _buf, 
			 hid_t dtype_id, hid_t cpdType, int cpdNField, char ** cpdField, int compoundAsDataFrame ) {
  hid_t mem_type_id = -1;

  SEXP Rval;

  hid_t superclass =  H5Tget_class(H5Tget_super( dtype_id ));
  if (superclass == H5T_INTEGER) {
    hid_t enumtype = H5Tenum_create(H5T_NATIVE_INT);
    int nmembers = H5Tget_nmembers( dtype_id );
    SEXP levels = PROTECT(allocVector(STRSXP, nmembers));
    for (int i=0; i<nmembers; i++) {
      char * st = H5Tget_member_name( dtype_id, i );
      SET_STRING_ELT(levels, i, mkChar(st));
      herr_t status = H5Tenum_insert (enumtype, st, &i);
    }
    UNPROTECT(1);

    if (cpdType < 0) {
      mem_type_id = enumtype;
    } else {
      mem_type_id = H5Tcreate(H5T_COMPOUND, H5Tget_size(enumtype));
      herr_t status = H5Tinsert(mem_type_id, cpdField[0], 0, enumtype);
      for (int i=1; i<cpdNField; i++) {
	hid_t mem_type_id2 = H5Tcreate(H5T_COMPOUND, H5Tget_size(enumtype));
	herr_t status = H5Tinsert(mem_type_id2, cpdField[i], 0, mem_type_id);
	mem_type_id = mem_type_id2;
      }
    }
    
    void * buf;
    if (length(_buf) == 0) {
      Rval = PROTECT(allocVector(INTSXP, n));
      buf = INTEGER(Rval);
    } else {
      buf = INTEGER(_buf);
      Rval = _buf;
    }

    herr_t herr = H5Dread(dataset_id, mem_type_id, mem_space_id, file_space_id, H5P_DEFAULT, buf );
    if (length(_buf) == 0) {
      for (int i=0; i < n; i++) {
	((int *)buf)[i] += 1;
      }
      setAttrib(Rval, R_DimSymbol, Rdim);
      setAttrib(Rval, mkString("levels"), levels);
      setAttrib(Rval, R_ClassSymbol, mkString("factor"));
      UNPROTECT(1);
    }
  } else {
    double na = R_NaReal;
    Rval = PROTECT(allocVector(REALSXP, n));
    for (int i=0; i<n; i++) { REAL(Rval)[i] = na; }
    setAttrib(Rval, R_DimSymbol, Rdim);
    UNPROTECT(1);
    printf("Warning: h5read for type ENUM [%s] not yet implemented. Values replaced by NA's\n", getDatatypeClass(H5Tget_super( dtype_id )));
  }

  return(Rval);

}

SEXP H5Dread_helper_ARRAY(hid_t dataset_id, hid_t file_space_id, hid_t mem_space_id, hsize_t n, SEXP Rdim, SEXP _buf, 
			  hid_t dtype_id, hid_t cpdType, int cpdNField, char ** cpdField, int compoundAsDataFrame ) {
  hid_t mem_type_id = -1;

  SEXP Rval;

  hid_t superclass =  H5Tget_class(H5Tget_super( dtype_id ));
  if (((superclass == H5T_INTEGER) | (superclass == H5T_FLOAT)) & (!((cpdNField > 0) & (compoundAsDataFrame > 0)))) {
    int ndims = H5Tget_array_ndims (dtype_id);
    hsize_t na = 1;
    hsize_t adims[ndims];
    H5Tget_array_dims( dtype_id, adims );
    for (int i=0; i < ndims; i++) {
      na = na * adims[i];
    }
    hid_t arraytype;
    if (superclass == H5T_INTEGER) {
      arraytype = H5Tarray_create (H5T_NATIVE_INT, ndims, adims);
    } else if (superclass == H5T_FLOAT) {
      arraytype = H5Tarray_create (H5T_NATIVE_DOUBLE, ndims, adims);
    }
    if (cpdType < 0) {
      mem_type_id =  arraytype;
    } else {
      mem_type_id = H5Tcreate(H5T_COMPOUND, H5Tget_size(arraytype));
      herr_t status = H5Tinsert(mem_type_id, cpdField[0], 0, arraytype);
      for (int i=1; i<cpdNField; i++) {
    	hid_t mem_type_id2 = H5Tcreate(H5T_COMPOUND, H5Tget_size(arraytype));
    	herr_t status = H5Tinsert(mem_type_id2, cpdField[i], 0, mem_type_id);
    	mem_type_id = mem_type_id2;
      }
    }
    void * buf;
    if (length(_buf) == 0) {
      if (superclass == H5T_INTEGER) {
	Rval = PROTECT(allocVector(INTSXP, n*na));
	buf = INTEGER(Rval);
      } else if (superclass == H5T_FLOAT) {
	Rval = PROTECT(allocVector(REALSXP, n*na));
	buf = REAL(Rval);
      }
    } else {
      if (superclass == H5T_INTEGER) {
	buf = INTEGER(_buf);
      } else if (superclass == H5T_FLOAT) {
	buf = REAL(_buf);
      }
      Rval = _buf;
    }

    herr_t herr = H5Dread(dataset_id, mem_type_id, mem_space_id, file_space_id, H5P_DEFAULT, buf );
    if (length(_buf) == 0) {
      SEXP Rdima = PROTECT(allocVector(INTSXP, LENGTH(Rdim)+ndims));
      int i=0,j=0;
      for (j=ndims-1; j>=0; j--,i++) {
        INTEGER(Rdima)[i] = adims[j];
      }
      for (j=0; j<LENGTH(Rdim); j++,i++) {
        INTEGER(Rdima)[i] = INTEGER(Rdim)[j];
      }
      setAttrib(Rval, R_DimSymbol, Rdima);
      UNPROTECT(2);
    }
  } else {
    double na = R_NaReal;
    Rval = PROTECT(allocVector(REALSXP, n));
    for (int i=0; i<n; i++) { REAL(Rval)[i] = na; }
    setAttrib(Rval, R_DimSymbol, Rdim);
    UNPROTECT(1);
    if ((cpdNField > 0) & (compoundAsDataFrame > 0)) {
      printf("Warning: h5read cannot coerce COMPOUND dataset with element of type ARRAY to data.frame. Values replaced by NA's. Try h5read with argument compoundAsDataFrame=FALSE to read element of type ARRAY\n");
    } else {
      printf("Warning: h5read for type ARRAY [%s] not implemented. Values replaced by NA's\n", getDatatypeClass(H5Tget_super( dtype_id )));
    }
  }

  return(Rval);

}

SEXP H5Dread_helper_COMPOUND(hid_t dataset_id, hid_t file_space_id, hid_t mem_space_id, hsize_t n, SEXP Rdim, SEXP _buf, 
			    hid_t dtype_id, hid_t cpdType, int cpdNField, char ** cpdField, int compoundAsDataFrame ) {
  hid_t mem_type_id = -1;

  if ((LENGTH(Rdim) > 1) && compoundAsDataFrame) {
    compoundAsDataFrame = 0;
    printf("Warning: Cannot coerce multi-dimensional data to data.frame. Data returned it as list.\n");
  }

  SEXP Rval;
  if (cpdType < 0) {
    int N = H5Tget_nmembers(dtype_id);
    PROTECT(Rval = allocVector(VECSXP, N));
    SEXP names = PROTECT(allocVector(STRSXP, N));
    for (int i=0; i<N; i++) {
      SET_STRING_ELT(names, i, mkChar(H5Tget_member_name(dtype_id,i)));
      char* name[1];
      name[0] = H5Tget_member_name(dtype_id,i);
      SEXP col;
      if (compoundAsDataFrame && (H5Tget_member_class(dtype_id,i) == H5T_COMPOUND)) {
	printf("Warning: Cannot read hierarchical compound data types as data.frame. Use 'compoundAsDataFrame=FALSE' instead. Values replaced by NA's.\n");
	double na = R_NaReal;
	col = PROTECT(allocVector(REALSXP, n));
	for (int i=0; i<n; i++) { REAL(col)[i] = na; }
	setAttrib(col, R_DimSymbol, Rdim);
	UNPROTECT(1);
      } else {
	col = H5Dread_helper(dataset_id, file_space_id, mem_space_id, n, Rdim, _buf,
			     H5Tget_member_type(dtype_id,i), 1, name, compoundAsDataFrame);
      }
      SET_VECTOR_ELT(Rval, i, col);
    }
    SET_NAMES(Rval, names);
    if (compoundAsDataFrame) {
      SEXP rn = PROTECT(allocVector(INTSXP, INTEGER(Rdim)[0]));
      for (int i=0; i<INTEGER(Rdim)[0]; i++) { INTEGER(rn)[i] = i+1; }
      UNPROTECT(1);
      setAttrib(Rval, mkString("row.names"), rn);
      setAttrib(Rval, R_ClassSymbol, mkString("data.frame"));
    }
    UNPROTECT(2);
  } else {
    int N = H5Tget_nmembers(dtype_id);
    PROTECT(Rval = allocVector(VECSXP, N));
    SEXP names = PROTECT(allocVector(STRSXP, N));
    for (int i=0; i<N; i++) {
      SET_STRING_ELT(names, i, mkChar(H5Tget_member_name(dtype_id,i)));
      char* name[cpdNField+1];
      name[0] = H5Tget_member_name(dtype_id,i);
      for (int j=0; j<cpdNField; j++) {
	name[j+1] = cpdField[j];
      }
      SEXP col = H5Dread_helper(dataset_id, file_space_id, mem_space_id, n, Rdim, _buf,
				H5Tget_member_type(dtype_id,i), cpdNField+1, name, compoundAsDataFrame);
      
      SET_VECTOR_ELT(Rval, i, col);
    }
    SET_NAMES(Rval, names);
    UNPROTECT(2);
  }
  return(Rval);
}

SEXP H5Dread_helper(hid_t dataset_id, hid_t file_space_id, hid_t mem_space_id, hsize_t n, SEXP Rdim, SEXP _buf, 
		    hid_t cpdType, int cpdNField, char ** cpdField, int compoundAsDataFrame ) {

  hid_t dtype_id;
  if (cpdType >= 0) {
    dtype_id = cpdType;
  } else {
    dtype_id = H5Dget_type(dataset_id);
  }
  hid_t dtypeclass_id = H5Tget_class(dtype_id);

  SEXP Rval;
  switch(dtypeclass_id) {
  case H5T_INTEGER: {
    Rval = H5Dread_helper_INTEGER(dataset_id, file_space_id, mem_space_id, n, Rdim, _buf, 
				  dtype_id, cpdType, cpdNField, cpdField, compoundAsDataFrame );
  } break;
  case H5T_FLOAT: {
    Rval = H5Dread_helper_FLOAT(dataset_id, file_space_id, mem_space_id, n, Rdim, _buf, 
				dtype_id, cpdType, cpdNField, cpdField, compoundAsDataFrame );
  } break;
  case H5T_STRING: {
    Rval = H5Dread_helper_STRING(dataset_id, file_space_id, mem_space_id, n, Rdim, _buf, 
				 dtype_id, cpdType, cpdNField, cpdField, compoundAsDataFrame );
  } break;
  case H5T_COMPOUND: {
    Rval = H5Dread_helper_COMPOUND(dataset_id, file_space_id, mem_space_id, n, Rdim, _buf, 
				   dtype_id, cpdType, cpdNField, cpdField, compoundAsDataFrame );
  } break;
  case H5T_ENUM: {
    Rval = H5Dread_helper_ENUM(dataset_id, file_space_id, mem_space_id, n, Rdim, _buf,
  			       dtype_id, cpdType, cpdNField, cpdField, compoundAsDataFrame );
  } break;
  case H5T_ARRAY: {
    Rval = H5Dread_helper_ARRAY(dataset_id, file_space_id, mem_space_id, n, Rdim, _buf,
  			        dtype_id, cpdType, cpdNField, cpdField, compoundAsDataFrame );
  } break;
  case H5T_TIME:
  case H5T_BITFIELD:
  case H5T_OPAQUE:
  case H5T_REFERENCE:
  case H5T_VLEN:
  default: {
    double na = R_NaReal;
    Rval = PROTECT(allocVector(REALSXP, n));
    for (int i=0; i<n; i++) { REAL(Rval)[i] = na; }
    setAttrib(Rval, R_DimSymbol, Rdim);
    UNPROTECT(1);
    printf("Warning: h5read for type '%s' not yet implemented. Values replaced by NA's\n", getDatatypeClass(dtype_id)); 
  } break;
  }

  return(Rval);
}

/* herr_t H5Dread(hid_t dataset_id, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id, void * buf ) */
/* TODO: accept mem_type_id as parameter */
SEXP _H5Dread( SEXP _dataset_id, SEXP _file_space_id, SEXP _mem_space_id, SEXP _buf, SEXP _compoundAsDataFrame ) {

  int compoundAsDataFrame = LOGICAL(_compoundAsDataFrame)[0];

  /***********************************************************************/
  /* dataset_id                                                          */
  /***********************************************************************/
  hid_t dataset_id = INTEGER(_dataset_id)[0];

  /***********************************************************************/
  /* file_space_id and get dimensionality of output file_space and buf   */
  /***********************************************************************/
  hid_t file_space_id;

  if (length(_file_space_id) == 0) {
    file_space_id = H5Dget_space( dataset_id );
  } else {
    file_space_id = INTEGER(_file_space_id)[0];
  }

  /***********************************************************************/
  /* create mem_space_id                                                 */
  /***********************************************************************/

  hid_t mem_space_id;
  if (length(_mem_space_id) == 0) {
    H5S_sel_type sel_type = H5Sget_select_type(file_space_id);
    if (sel_type != H5S_SEL_ALL) {
      printf("file dataspace is set up for selective reading (e.g. hyperslabs). You have to provide a memory space for selective reading.\n");
      SEXP Rval = R_NilValue;
      return Rval;
    }
    int rank = H5Sget_simple_extent_ndims( file_space_id );
    hsize_t size[rank];
    hsize_t maxsize[rank];
    H5Sget_simple_extent_dims(file_space_id, size, maxsize);
    hsize_t dims[rank];
    for (int i=0; i<rank; i++) {
      dims[i] = size[rank-i-1];
    }
    mem_space_id = H5Screate_simple( rank, dims, dims);
  } else {
    mem_space_id = INTEGER(_mem_space_id)[0];
  }

  /***********************************************************************/
  /* calculate buffer size and set dim-attribute                         */
  /***********************************************************************/

  int rank = H5Sget_simple_extent_ndims( mem_space_id );
  hsize_t size[rank];
  hsize_t maxsize[rank];
  H5Sget_simple_extent_dims(mem_space_id, size, maxsize);
  hsize_t n = 1;
  for (int i=0; i < rank; i++) {
    n = n * size[i];
  }
  SEXP Rdim;
  if (rank > 0) {
    Rdim = PROTECT(allocVector(INTSXP, rank));
    for (int i=0; i<rank; i++) {
      INTEGER(Rdim)[i] = size[i];
    }
  } else {
    Rdim = NULL_USER_OBJECT;
  }

  /***********************************************************************/
  /* read file space data type                                           */
  /***********************************************************************/

  SEXP Rval = H5Dread_helper(dataset_id, file_space_id, mem_space_id, n, Rdim, _buf, -1, -1, NULL, compoundAsDataFrame);

  // close mem space
  if (length(_mem_space_id) == 0) {
    H5Sclose(mem_space_id);
  }
  if (rank > 0) {
    UNPROTECT(1);
  }

  // close file space 
  if (length(_file_space_id) == 0) {
    H5Sclose(file_space_id);
  }
  return Rval;
}

/* herr_t H5Dwrite(hid_t dataset_id, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id, const void * buf ) */
/* TODO more parameters: hid_t xfer_plist_id */
SEXP _H5Dwrite( SEXP _dataset_id, SEXP _buf, SEXP _file_space_id, SEXP _mem_space_id ) {
  hid_t dataset_id = INTEGER(_dataset_id)[0];
  hid_t mem_type_id;
  hid_t mem_space_id;
  if (length(_mem_space_id) == 0) {
    mem_space_id = H5S_ALL;
  } else {
    mem_space_id = INTEGER(_mem_space_id)[0];
  }
  hid_t file_space_id;
  if (length(_file_space_id) == 0) {
    file_space_id = H5S_ALL;
  } else {
    file_space_id = INTEGER(_file_space_id)[0];
  }

  const void * buf;
  if (TYPEOF(_buf) == INTSXP) {
    mem_type_id = H5T_NATIVE_INT;
    buf = INTEGER(_buf);
  } else {
    if (TYPEOF(_buf) == REALSXP) {
      mem_type_id = H5T_NATIVE_DOUBLE;
      buf = REAL(_buf);
    } else {
      if (TYPEOF(_buf) == STRSXP) {
	mem_type_id = H5Dget_type(dataset_id);
	size_t stsize = H5Tget_size( mem_type_id );
	char * strbuf = (char *)R_alloc(LENGTH(_buf),stsize);
	int z=0;
	int j;
	for (int i=0; i < LENGTH(_buf); i++) {
	  for (j=0; (j < LENGTH(STRING_ELT(_buf,i))) & (j < (stsize-1)); j++) {
	    strbuf[z++] = CHAR(STRING_ELT(_buf,i))[j];
	  }
	  for (; j < stsize; j++) {
	    strbuf[z++] = '\0';
	  }
	}
	buf = strbuf;
      } else {
	mem_type_id = -1;
	printf("Writing of this type of data not supported.\n");
	SEXP Rval = R_NilValue;
	return Rval;
      }
    }
  }
  herr_t herr = 3;
  herr = H5Dwrite(dataset_id, mem_type_id, mem_space_id, file_space_id, H5P_DEFAULT, buf );
  SEXP Rval;
  PROTECT(Rval = allocVector(INTSXP, 1));
  INTEGER(Rval)[0] = herr;
  UNPROTECT(1);
  return Rval;
}

/* hid_t H5Dget_space(hid_t dataset_id ) */
SEXP _H5Dget_space(SEXP _dataset_id ) {
  hid_t dataset_id = INTEGER(_dataset_id)[0];
  hid_t sid = H5Dget_space( dataset_id );
  addHandle(sid);
  SEXP Rval = ScalarInteger( sid );
  return Rval;
}

/* hid_t H5Dget_type(hid_t dataset_id) */
SEXP _H5Dget_type( SEXP _dataset_id ) {
  hid_t dataset_id = INTEGER(_dataset_id)[0];
  hid_t hid = H5Dget_type( dataset_id );
  SEXP Rval = ScalarInteger(hid);
  return Rval;
}

