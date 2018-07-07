\name{as.raster}
\alias{as.raster}
\alias{as.raster,arc.raster-method}

\title{Create RasterLayer or RasterBrick (raster package)}

\description{
Create Raster* object from arc.raster
TODO
}

\usage{
  \S4method{as.raster}{arc.raster}(x, kind ,\dots)
}
\arguments{
  \item{x}{\link{arc.raster-class} object}
  \item{kind}{internal parameter}
  \item{\dots}{.}
}
\value{
  return \code{RasterLayer} for single band source or \code{RasterBrick}
}

\examples{
## convert arc.raster to Rasterlayer object
\dontshow{ if (requireNamespace("rgdal", quietly = TRUE)) \{ }
  r.file <- system.file("pictures", "logo.jpg", package="rgdal")
  r <- arc.raster(arc.open(r.file))
  rx <- as.raster(r)
\dontshow{\}}
}

\keyword{method}
\keyword{raster}

