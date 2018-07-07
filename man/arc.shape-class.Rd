\name{arc.shape-class}
\docType{class}
\alias{arc.shape-class}
\alias{[,arc.shape}
\alias{length,arc.shape-method}
%\alias{show,arc.shape-method}

\title{Class "arc.shape"}
\description{
\code{arc.shape} S4 class. Object \code{arc.shape} is a geometry collection.
}
\details{
\code{arc.shape} is attached to an ArcGIS \code{data.frame} as the
attribute "shape". Each element corresponds to one record in
the input data frame. Points are presented as an array of lists, with
each list containing (\code{x}, \code{y}, \code{Z}, \code{M}), where
}
\section{Extends}{
  Class \code{list}, directly.
}

\section{Slots}{
  \describe{
    \item{\code{.Data}}{internal}
    \item{\code{shapeinfo}}{geometry information, for mode details see \code{\link{arc.shapeinfo}}}
  }
}

\section{Methods}{
  \describe{
    \item{[}{\code{signature(x = "arc.shape", i=numeric)} select geometry subset}
    \item{\link{arc.shapeinfo}}{return geometry information}
%    \item{show}{show object}
    \item{length}{length of collection}
  }
}

\examples{
d <- arc.select(arc.open(system.file("extdata", "ca_ozone_pts.shp", package="arcgisbinding")), "FID")

shape <- arc.shape(d)
shape
\dontrun{
geometry type   : Point
WKT             : PROJCS["USA_Contiguous_Albers_Equal_Area_Conic",GEOGCS["GCS_...
WKID            : 102003
length          : 193
}

# access X and Y values
xy <- list(X=shape$x, Y=shape$y)

}

\seealso{
\code{\link{arc.shape}},
\code{\link{arc.shapeinfo}}
}

\keyword{classes}
\keyword{geometry}
\keyword{shape}

