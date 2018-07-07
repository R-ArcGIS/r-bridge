\name{arc.shape2sp}
\alias{arc.shape2sp}
\title{Convert 'arc.shape' geometry sp spatial geometry}
\usage{
arc.shape2sp(shape, wkt)
}
\arguments{
\item{shape}{\code{\link{arc.shape-class}}}

\item{wkt}{optional, WKT spatial reference}
}
\description{
Convert \code{\link{arc.shape-class}} to \code{sp} spatial geometry:
\code{SpatialPoints}, \code{SpatialLines}, or \code{SpatialPolygons}.
Similar to \code{\link{arc.data2sp}}.
}
\examples{

if (require(sp))
{
  d <- arc.select(arc.open(system.file("extdata", "ca_ozone_pts.shp", package="arcgisbinding")), 'ozone')
  x <- arc.shape2sp(arc.shape(d))
  \dontrun{plot(x)}
}
}
\seealso{
\code{\link{arc.shape}},
\code{\link{arc.data2sp}}
}

\keyword{method}
\keyword{geometry}
\keyword{feature}
\keyword{spatial}
