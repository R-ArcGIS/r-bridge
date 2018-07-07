\name{arc.data2sp}
\alias{arc.data2sp}
\title{Convert an 'arc.data' or 'arc.raster' object to an sp 'SpatialDataFrame' object}
\usage{
arc.data2sp(x)
}
\arguments{
  \item{x}{\code{\link{arc.data}} object, result of \code{\link{arc.select}} or \code{\link{arc.raster}}. }
}
\description{
Convert an ArcGIS \code{arc.data} to the equivalent \code{sp} data frame
type. The output types that can be generated: \code{SpatialPointsDataFrame},
\code{SpatialLinesDataFrame}, or \code{SpatialPolygonsDataFrame}.
Convert an \code{arc.raster} object to a \code{SpatialGridDataFrame} object.
}
\examples{
\dontshow{ if (requireNamespace("sp",quietly = TRUE)) \{ }
d <- arc.select(arc.open(system.file("extdata", "ca_ozone_pts.shp", package="arcgisbinding")), 'ozone')

require("sp")
sp.df <- arc.data2sp(d)
\dontrun{spplot(sp.df)}

\dontshow{ \} }
}

\seealso{
\code{\link{arc.open}},
\code{\link{arc.select}}
}

\keyword{method}
\keyword{geometry}
\keyword{feature}
\keyword{spatial}