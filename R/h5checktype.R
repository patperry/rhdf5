h5checktype <- function(h5id, type, fctname = deparse(match.call()[1]), allow.character = FALSE) {
  if (!is( h5id, "H5IdComponent" ) ) {
    if (allow.character) {
      stop("Error in ", fctname, ". Argument neither of class H5IdComponent nor a character.", call. = FALSE)
    } else {
      stop("Error in ", fctname, ". Argument not of class H5IdComponent.", call. = FALSE)
    }
  }
  isvalid = H5Iis_valid(h5id)
  if (!isvalid) {
    stop("Error in ", fctname, ". H5Identifier not valid.", call. = FALSE)
  }
  truetype = H5Iget_type(h5id)
  switch(type,
         file = {
           if (truetype != "H5I_FILE") {
             stop("Error in ", fctname, ". H5Identifier not a file identifier.", call. = FALSE)
           }
           0
         },
         group = {
           if (truetype != c("H5I_GROUP")) {
             stop("Error in ", fctname, ". H5Identifier not a group identifier.", call. = FALSE)
           }
           0
         },
         loc = {
           if (!(truetype %in% c("H5I_FILE","H5I_GROUP"))) {
             stop("Error in ", fctname, ". H5Identifier not a location identifier.", call. = FALSE)
           }
           0
         },
         dataset = {
           if (truetype != "H5I_DATASET") {
             stop("Error in ", fctname, ". H5Identifier not a dataset identifier.", call. = FALSE)
           }
           0
         },
         object = {
           if (!(truetype %in% c("H5I_FILE","H5I_GROUP","H5I_DATASET"))) {
             stop("Error in ", fctname, ". H5Identifier not an object identifier.", call. = FALSE)
           }
           0
         },
         dataspace = {
           if (truetype != "H5I_DATASPACE") {
             stop("Error in ", fctname, ". H5Identifier not a dataspace identifier.", call. = FALSE)
           }
           0
         },
         attribute = {
           if (truetype != "H5I_ATTR") {
             stop("Error in ", fctname, ". H5Identifier not an attribute identifier.", call. = FALSE)
           }
           0
         },
         type = {
           if (truetype != "H5I_DATATYPE") {
             stop("Error in ", fctname, ". H5Identifier not a type identifier.", call. = FALSE)
           }
           0
         },
         object = {
           if (!(truetype %in% c("H5I_FILE","H5I_GROUP","H5I_DATASET","H5I_DATATYPE","H5I_DATASPACE","H5I_ATTR"))) {
             stop("Error in ", fctname, ". H5Identifier not an object identifier.", call. = FALSE)
           }
           0
         },
         {
           stop("argument for type unknown")
         }
       )
  return(invisible(NULL))
}

h5checktypeOrNULL <- function(h5id, type, fctname = deparse(match.call()[1])) {
  if (!is.null(h5id)) {
    h5checktype(h5id, type, fctname = fctname)
  }
  invisible(NULL)
}

h5checktypeOrOpenLoc <- function(file, fctname = deparse(match.call()[1]), createnewfile=FALSE, readonly=FALSE) {
  res = list()
  if (is.character(file)) {
    if (file.exists(file)) {
      h5loc <- if (readonly) {
          H5Fopen(file,"H5F_ACC_RDONLY")
        } else {
          H5Fopen(file)
        }
      if (!is(h5loc, "H5IdComponent")) {
        stop("Error in ",fctname,". File '",file,"' is not a valid HDF5 file.")
      } else {
        res$H5Identifier = h5loc
        res$closeit = TRUE
      }
    } else {
      if (createnewfile) {
        h5loc <- H5Fcreate(file)
        if (!is(h5loc, "H5IdComponent")) {
          stop("Error in ",fctname,". Cannot create file.")
        } else {
          res$H5Identifier = h5loc
          res$closeit = FALSE
        }
      } else {
        stop("Error in ",fctname,". Cannot open file. File '",file,"' does not exists.")
      }
    }
  } else {
    h5checktype(file, "loc", fctname = fctname, allow.character = TRUE)
    res$H5Identifier = file
    res$closeit = FALSE
  }
  invisible(res)
}

h5closeitLoc <- function(file, fctname = deparse(match.call()[1])) {
  res = TRUE
  if (file$closeit) {
    if (H5Iis_valid(file$H5Identifier)) {
      res = H5Fclose(file$H5Identifier)
    } else {
      res = FALSE
    }
  } else {
    res = FALSE
  }
  invisible(res)
}

h5checktypeOrOpenObj <- function(obj, file, fctname = deparse(match.call()[1])) {
  res = list()
  if (is.character(obj)) {
    if (missing(file)) {
      stop("Error in ",fctname,". If the object is specified by name, you have to specify the file.")
    }
    loc = h5checktypeOrOpenLoc(file, fctname=fctname)

    if (!H5Lexists(loc$H5Identifier, obj)) {
      stop("Error in ",fctname,". Object '",obj,"' not found in file.")
    } else {
      h5obj = H5Oopen(loc$H5Identifier, obj)
      if (!is(h5obj, "H5IdComponent")) {
        stop("Error in ",fctname,". Cannot open object.")
      } else {
        res$H5Identifier = h5obj
        res$closeit = TRUE
      }
    }
    h5closeitLoc(loc)   
  } else {
    h5checktype(obj, "object", fctname = fctname, allow.character = TRUE)
    res$H5Identifier = obj
    res$closeit = FALSE
  }
  invisible(res)
}

h5closeitObj <- function(obj, fctname = deparse(match.call()[1])) {
  res = TRUE
  if (obj$closeit) {
    if (H5Iis_valid(obj$H5Identifier)) {
      res = H5Oclose(obj$H5Identifier)
    } else {
      res = FALSE
    }
  } else {
    res = FALSE
  }
  invisible(res)
}

