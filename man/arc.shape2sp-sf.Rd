\name{Convert to sp::Spatial* - Spatial geometry, sf::sfc - Simple Feature geometry}
\alias{arc.shape2sp}
\alias{arc.shape2sf}
\title{Convert 'arc.shape' geometry object to sp::Spatial* - Spatial geometry or sf::sfc - simple feature geometry}
\usage{
arc.shape2sp(shape, wkt)
arc.shape2sf(shape)
}
\arguments{
\item{shape}{\code{\link{arc.shape-class}}}
\item{wkt}{optional, WKT spatial reference}
}

\value{
 an object of class sp::Spatial*.

 an object of class sf::sfc, which is a classed list-column with simple feature geometries.
}

\description{
Convert \code{\link{arc.shape-class}} to \code{sp} spatial geometry:
\code{SpatialPoints}, \code{SpatialLines}, or \code{SpatialPolygons}.
Similar to \code{\link{arc.data2sp}}.

Convert \code{\link{arc.shape-class}} to \code{sfc} simple feature geometry:
\code{POINT}, \code{MULTIPOINT}, \code{POLYGON}, \code{MULTIPOLYGON}, \code{LINESTRING}, \code{MULTILINESTRING}.
Similar to \code{\link{arc.data2sf}}.
}

\examples{
d <- arc.select(arc.open(system.file("extdata", "ca_ozone_pts.shp", package="arcgisbinding")), 'ozone')
x <- arc.shape(d)

\dontshow{ if (requireNamespace("sp", quietly = TRUE)) \{ }
geom <- arc.shape2sp(x)
\dontrun{plot(geom)}
\dontshow{ \} }
\dontshow{ if (requireNamespace("sf", quietly = TRUE)) \{ }
geom <- arc.shape2sf(x)
\dontrun{plot(geom)}
\dontshow{ \} }
}

\seealso{
\code{\link{arc.shape}},
\code{\link{arc.data2sp}}
\code{\link{arc.data2sf}}
}

\keyword{method}
\keyword{geometry}
\keyword{feature}
\keyword{spatial}
\keyword{shape}