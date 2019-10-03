\name{Convert to (sp) SpatialDataFrame, (sf) Simple Feature}
\alias{arc.data2sp}
\alias{arc.data2sf}
\title{
Convert 'arc.data' or 'arc.raster' object to 'sp' - SpatialDataFrame object or 'sf' - Simple Feature object}
\usage{
arc.data2sp(x)
arc.data2sf(x)
}
\arguments{
\item{x}{\code{\link{arc.data}} object, result of \code{\link{arc.select}} or \code{\link{arc.raster}}. }
}
\value{
  sp::Spatial*DataFrame object.

  sf::sfc object.
}

\description{
Convert an ArcGIS \code{arc.data} to the equivalent \code{sp} data frame
type. The output types that can be generated: \code{SpatialPointsDataFrame},
\code{SpatialLinesDataFrame}, or \code{SpatialPolygonsDataFrame}.

Convert an \code{arc.raster} object to a \code{SpatialGridDataFrame} object.

Convert an ArcGIS \code{arc.data} to the equivalent \code{sfc} object
type. The output types that can be generated:
\code{POINT}, \code{MULTIPOINT}, \code{POLYGON}, \code{MULTIPOLYGON}, \code{LINESTRING}, \code{MULTILINESTRING}.
}

\examples{
\dontshow{ if (requireNamespace("sp",quietly = TRUE)) \{ }
d <- arc.select(arc.open(system.file("extdata", "ca_ozone_pts.shp", package="arcgisbinding")), 'ozone')

require("sp")
df.sp <- arc.data2sp(d)
\dontrun{spplot(df.sp)}

\dontshow{ \} }

\dontshow{ if (requireNamespace("sf",quietly = TRUE)) \{ }

require("sf")
df.sf <- arc.data2sf(d)
\dontrun{plot(df.sf)}

\dontshow{ \} }
}

\seealso{
\code{\link{arc.open}},
\code{\link{arc.select}}
\code{\link{arc.raster}}
}

\keyword{method}
\keyword{geometry}
\keyword{feature}
\keyword{spatial}
