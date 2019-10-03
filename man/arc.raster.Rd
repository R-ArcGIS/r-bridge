\name{arc.raster}
\docType{methods}
\alias{arc.raster}
\alias{arc.raster,NULL-method}
\alias{arc.raster,arc.datasetraster-method}
\alias{arc.raster,arc.datasetrastermosaic-method}
\concept{raster}

\title{Load or create "arc.raster" object}
\usage{

\S4method{arc.raster}{arc.datasetraster}(object, bands, \dots)

\S4method{arc.raster}{arc.datasetrastermosaic}(object, bands, \dots)

\S4method{arc.raster}{NULL}(object, path, dim, nrow, ncol, nband, extent,
  origin_x, origin_y, cellsize_x, cellsize_y, pixel_type, nodata, sr, \dots)
}
\arguments{
\item{object}{code{\link{arc.datasetraster-class}} object.}
\item{bands}{optional, integer. List of bands to read (default: all bands).}
\item{\dots}{optional additional arguments such as \code{nrow, ncol, extent, pixel_type, resample_type} to be passed to the method.
  Use \code{overwite=TRUE} to overwite existing dataset.
}

\item{path}{file path (character) or layer name (character).}
\item{dim}{optional. List for number of rows and columns of the raster.}
\item{nrow}{optional, integer > 0. Number of rows for the raster or mosaic dataset. The default is \code{object@nrow}.}
\item{ncol}{optional, integer > 0. Number of columns for the raster or mosaic dataset. The default is \code{object@ncol}.}
\item{nband}{integer > 0. Number of bands to create.}
\item{extent}{optional, list. extent of raster to be read. The default is \code{object@extent}.}
\item{origin_x}{optional. Minimum x coordinate.}
\item{origin_y}{optional. Minimum y coordinate.}
\item{cellsize_x}{optional. Size of pixel in x-axis.}
\item{cellsize_y}{optional. Size of pixel in y-axis.}
\item{pixel_type}{optional. Type of raster pixels. For details about different pixel types see \code{\link{pixel_type}}.
                  See also \href{https://desktop.arcgis.com/en/arcmap/10.3/manage-data/raster-and-images/bit-depth-capacity-for-raster-dataset-cells.htm}{ArcGIS Help: Pixel Types}.
                  The default is \code{object@pixel_type}.}
\item{nodata}{numeric, value for no data values.}
\item{sr}{optional transform raster to spatial reference. The default is \code{object@sr}.}

}
\value{
\code{arc.raster} returns a \code{raster} object (type of \link{arc.raster-class}.).
}
\description{
Methods to create a \code{arc.raster} object from scratch, extent, \code{arc.open} object or a raster file (inside or outside of a file geodatabase).
}

\section{References}{
  \enumerate{\item \href{https://desktop.arcgis.com/en/arcmap/latest/manage-data/raster-and-images/what-is-raster-data.htm}{ArcGIS Help: Raster Introduction}
  \item \href{https://desktop.arcgis.com/en/arcmap/10.3/manage-data/raster-and-images/bit-depth-capacity-for-raster-dataset-cells.htm}{ArcGIS Help: Pixel Types}
  \item \href{https://desktop.arcgis.com/en/arcmap/10.3/manage-data/raster-and-images/what-is-a-mosaic-dataset.htm}{ArcGIS Help: Mosaic Introductions}
  \item \href{https://desktop.arcgis.com/en/arcmap/10.3/manage-data/raster-and-images/understanding-the-mosaicking-rules-for-a-mosaic-dataset.htm}{ArcGIS Help: Mosaicking Rules}
  }
}


\examples{
## resample raster
\dontshow{ if (requireNamespace("rgdal", quietly = TRUE)) \{ }
r.file <- system.file("pictures", "cea.tif", package="rgdal")
r <- arc.raster(arc.open(r.file), nrow=200, ncol=200, resample_type="CubicConvolution")
stopifnot(r$nrow == 200 && r$resample_type == "CubicConvolution")

\dontrun{
> r
type            : Raster
pixel_type      : U8 (8bit)
nrow            : 200
ncol            : 200
resample_type   : CubicConvolution
cellsize        : 154.256892046808, 154.557002731725
nodata          : NA
extent          : xmin=-28493.17, ymin=4224973, xmax=2358.212, ymax=4255885
WKT             : PROJCS["North_American_1927_Cylindrical_Equal_Area",GEOGCS["...
band            : Band_1 }
\dontshow{\} }

## create an empty raster

r = arc.raster(NULL, path=tempfile("new_raster", fileext=".img"), extent=c(0, 0, 100, 100), nrow=100, ncol=100, nband=5, pixel_type="F32")
stopifnot(all(dim(r) == c(100, 100, 5)))

\dontrun{
> dim(r)
nrow  ncol nband
 100   100     5 }
}

\seealso{
\code{\link{arc.open}},
\code{\link{arc.write}},
\code{\link{arc.raster-class}}
}

\keyword{method}
\keyword{raster}