\name{arc.shapeinfo}
\alias{arc.shapeinfo}
\alias{arc.shapeinfo,arc.feature-method}
\alias{arc.shapeinfo,arc.shape-method}
\concept{geomery}
\concept{shape}

\title{Get geometry information}
\usage{
  \S4method{arc.shapeinfo}{arc.shape}(object)
  \S4method{arc.shapeinfo}{arc.feature}(object)
}
\arguments{
\item{object}{\link{arc.feature-class} or \link{arc.shape-class} object}
}

\value{
  returns named list of :
    \item{\code{type}}{geometry type: "Point", "Polyline", or "Polygon"}
    \item{\code{hasZ}}{TRUE if geometry includes Z-values}
    \item{\code{hasM}}{TRUE if geometry includes M-values}
    \item{\code{WKT}}{well-known text representation of the shape's spatial reference}
    \item{\code{WKID}}{well-known ID of the shape's spatial reference}
}
\description{
\code{arc.shapeinfo} provides details on what type of geometry is stored
within the dataset, and the spatial reference of the geometry. The
well-known text, \code{WKT}, allows interoperable transfer of the spatial
reference system (CRS) between environments. The \code{WKID} is a numeric
value that ArcGIS uses to precisely specify a projection.
}

\section{References}{

\enumerate{
  \item \href{http://resources.arcgis.com/en/help/arcgis-rest-api/index.html#/Using_spatial_references/02r3000000qq000000/}{ArcGIS REST API: Using spatial references}
  \item \href{http://spatialreference.org/}{Spatial reference lookup}
}
}
\examples{
d <- arc.open(system.file("extdata", "ca_ozone_pts.shp", package="arcgisbinding"))
# from arc.feature
info <- arc.shapeinfo(d)
info$WKT   # print dataset spatial reference

# from arc.shape
df <- arc.select(d, 'ozone')
info <- arc.shapeinfo(arc.shape(df))

}

\seealso{
  \code{\link{arc.open}},
  \code{\link{arc.shape}}
}

\keyword{SpatialReference}
\keyword{geometry}
\keyword{shape}

