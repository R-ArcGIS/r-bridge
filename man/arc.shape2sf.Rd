\name{arc.shape2sf}
\alias{arc.shape2sf}
\title{Convert 'arc.shape' geometry to 'sfc' simple feature geometry}
\usage{
arc.shape2sf(shape)
}
\arguments{
\item{shape}{\code{\link{arc.shape-class}}}
}
\description{
Convert \code{\link{arc.shape-class}} to \code{sfc} simple feature geometry:
\code{POINT}, \code{MULTIPOINT}, \code{POLYGON}, \code{MULTIPOLYGON}, \code{LINESTRING}, \code{MULTILINESTRING}.
Similar to \code{\link{arc.data2sf}}.
}
\examples{

\dontshow{ if (requireNamespace("sf", quietly = TRUE)) \{ }
d <- arc.select(arc.open(system.file("extdata", "ca_ozone_pts.shp", package="arcgisbinding")), 'ozone')
x <- arc.shape2sf(arc.shape(d))
\dontrun{plot(x)}
\dontshow{ \} }
}
\seealso{
\code{\link{arc.shape}},
\code{\link{arc.data2sf}}
}

\keyword{method}
\keyword{geometry}
\keyword{feature}
\keyword{spatial}