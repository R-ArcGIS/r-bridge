\name{arc.shape}
\alias{arc.shape}
%\alias{arc.shape.arc.data}
\title{Get "arc.shape" geometry object}

\usage{
 % \method{arc.shape}{arc.data}(x)
 arc.shape(x)
}
\arguments{
  \item{x}{ a \code{data.frame} object of type \code{arc.data}}
}
\value{
  returns \code{\link{arc.shape-class}}
}
\description{
Get geometry object of \code{\link{arc.shape-class}} from \code{\link{arc.data}} object.
}

\examples{

d <- arc.open(system.file("extdata", "ca_ozone_pts.shp", package="arcgisbinding"))
df <- arc.select(d, 'ozone')

shp <- arc.shape(df)
stopifnot(length(shp) == nrow(df))

shp
\dontrun{
geometry type   : Point
WKT             : PROJCS["USA_Contiguous_Albers_Equal_Area_Conic",GEOGCS["GCS_...
WKID            : 102003
length          : 193
}
}

\seealso{
\code{\link{arc.shapeinfo}},
\code{\link{arc.select}},
\code{\link{arc.data}}
}

\keyword{method}
\keyword{geometry}

