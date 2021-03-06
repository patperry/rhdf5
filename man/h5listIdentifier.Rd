\name{h5listIdentifier}
\alias{h5listIdentifier}
\alias{h5validObjects}
\title{list all valid H5 identifier.}
\description{A list of all valid H5 identifier. H5 objects should be closed after usage to release resources.}
\usage{
h5listIdentifier()
h5validObjects()
}
\value{
\code{h5validObjects} returns a list of \code{\link{H5IdComponent}} objects. \code{h5listIdentifier} prints the valid identifiers on screen and returns NULL.
}
\references{\url{http://www.hdfgroup.org/HDF5}}
\author{Bernd Fischer}
\seealso{
\link{rhdf5}
}
\examples{
h5createFile("ex_list_identifier.h5")

# create groups
h5createGroup("ex_list_identifier.h5","foo")

h5listIdentifier()
h5validObjects()
}
\keyword{ programming }
\keyword{ interface }
\keyword{ IO }
\keyword{ file }
