#ifndef _H5F_H
#define _H5F_H

#include <R.h>
#include <Rdefines.h>
#include <R_ext/Rdynload.h>
#include <R_ext/Error.h>
#include "myhdf5.h"
#include "HandleList.h"

SEXP _H5Fcreate( SEXP _name, SEXP _flags );
SEXP _H5Fopen( SEXP _name, SEXP _flags );
SEXP _H5Freopen( SEXP _file_id );
SEXP _H5Fclose( SEXP _file_id );
SEXP _H5Fflush( SEXP _object_id, SEXP _scope );
/* _H5Fis_hdf5 */
/* _H5Fmount */
/* _H5Funmount */

/* _H5Fget_vfd_handle */
/* _H5Fget_filesize */
/* _H5Fget_create_plist */
/* _H5Fget_access_plist */
/* _H5Fget_info */
/* _H5Fget_intent */
/* _H5Fget_name */
/* _H5Fget_obj_count */
      	
/* _H5Fget_obj_ids */
/* _H5Fget_freespace */
/* _H5Fget_mdc_config */
/* _H5Fget_mdc_hit_rate */
/* _H5Fget_mdc_size */
/* _H5Freset_mdc_hit_rate_stats */
/* _H5Fset_mdc_config */

#endif
