\name{arc.data}
\alias{arc.data}
\alias{[.arc.data}
%\alias{arc.shape.arc.data}
\alias{arrange.arc.data}
\alias{filter.arc.data}
\alias{group_by.arc.data}
\alias{mutate.arc.data}
\alias{ungroup.arc.data}
\title{Class "arc.data"}
\description{
  \code{arc.data} class and methods
}
\section{Extends}{
  Class \code{data.frame}, directly.
}

\details{
  TODO
  \code{arc.data} object is \code{data.frame} with geometry attribute.
  To access geometry use \code{\link{arc.shape}}.
}
\note{
  You can display the \code{arc.data} object. Geometry information, first 5 and last 3 row will be showed.
}
\usage{
  %\method{arc.shape}{arc.data}(x)
  \method{[}{arc.data}(x, i, j, drop)

### dplyr methods:
  \method{filter}{arc.data}(.data, \dots, .dots)
  \method{arrange}{arc.data}(.data, \dots, .dots)
  \method{mutate}{arc.data}(.data, \dots, .dots)
  \method{group_by}{arc.data}(.data, \dots, add)
  \method{ungroup}{arc.data}(x, \dots)
}

\arguments{
  \item{i, j, \dots}{indices specifying elements to subset}
  \item{drop}{if \code{TRUE} coerce the result to the lowest possible dimension and
   remove the geometry attribute }
  \item{x}{A \code{arc.data} object}
  \item{.data}{A \code{arc.data} object}
  \item{.dots}{other arguments (see package dplyr)}
  \item{add}{To add to the existing groups, use \code{add = TRUE}}
}

\section{dplyr methods}{
  \itemize{
    \item \code{filter}: Return rows with matching conditions
    \item \code{arrange}: Arrange rows by variables
    \item \code{mutate}, \code{transmute}: Add new variables
    \item \code{select}: Select/rename variables by name
    \item \code{group_by}: Group by one or more variables
    \item \code{slice}: Select rows by position
    \item \code{distinct}: Select distinct/unique rows
  }
}

\examples{
  d <- arc.select(arc.open(system.file("extdata", "ca_ozone_pts.shp", package="arcgisbinding")))
  d
\dontrun{
geometry type   : Point
WKT             : PROJCS["USA_Contiguous_Albers_Equal_Area_Conic",GEOGCS["GCS_...
WKID            : 102003
    FID LATITUDE LONGITUDE ELEVATION   OZONE        X         Y    text
1     0  39.1447 -123.2065       194 0.04650 -2298092  515557.4 Value_0
2     1  39.4030 -123.3491       420 0.04969 -2301588  546772.7 Value_1
3     2  37.7661 -122.3978         5 0.05000 -2273948  347691.4 Value_2
4     3  37.9508 -122.3569        23 0.05799 -2264847  366623.2 Value_3
5     4  36.6986 -121.6354        36 0.05860 -2241776  214412.1 Value_0
... ...      ...       ...       ...     ...      ...       ...     ...
191 190  34.0598 -117.1462         0 0.16449 -1921585 -170440.0 Value_2
192 191  34.2412 -117.2756      1384 0.16470 -1928645 -148045.5 Value_3
193 192  34.1065 -117.2732         0 0.17360 -1931774 -162775.2 Value_0
}

# subset rows 1,3 and 5 with corresponding features
d135 <- d[c(1,3,5),]

\dontshow{ if (requireNamespace("dplyr", quietly = TRUE)) \{ }

# dplyr support
require("dplyr")
filter(d, ELEVATION > 1800)

#add new elevation column in meters
mutate(d, elevm = ELEVATION * 0.3048)
\dontshow{ \} }
}

\seealso{
  \code{\link{arc.shape}},
  \code{\link{arc.open}},
  \code{\link{arc.select}}
}


\keyword{classes}
\keyword{methods}
\keyword{dataset}
\keyword{features}
\keyword{subset}
