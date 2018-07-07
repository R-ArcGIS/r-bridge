\name{arc.data2sf}
\alias{arc.data2sf}
\title{Convert an 'arc.data' object to an 'sf' Simple Feature object}
\usage{
arc.data2sf(x)
}
\arguments{
\item{x}{\code{\link{arc.data}} object, result of \code{\link{arc.select}}.}
}
\description{
Convert an ArcGIS \code{arc.data} to the equivalent \code{sf} object
type. The output types that can be generated:
\code{POINT}, \code{MULTIPOINT}, \code{POLYGON}, \code{MULTIPOLYGON}, \code{LINESTRING}, \code{MULTILINESTRING}.
}
\examples{
\dontshow{ if (requireNamespace("sf",quietly = TRUE)) \{ }
d <- arc.select(arc.open(system.file("extdata", "ca_ozone_pts.shp", package="arcgisbinding")), 'ozone')

require("sf")
x <- arc.data2sf(d)
\dontrun{plot(x)}

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
