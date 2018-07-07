\name{arc.raster-class}
\docType{class}
\alias{arc.raster-class}
\alias{dim,arc.raster-method}
\alias{dim<-,arc.raster-method}
\alias{names,arc.raster-method}
%\alias{show,arc.raster-method}
\title{Reference Class "arc.raster"}
\description{A raster dataset is any valid raster format organized into one or more bands. Each band consists of an array of pixels (cells), and each pixel has a value. A raster dataset has at least one band. Raster data is a discrete data representation in which space is divided into uniform cells, or pixels.}

\section{Fields}{

\describe{
\item{\code{sr}}{Get or set spacial reference}

\item{\code{extent}}{Get or set extent. Use it to read a portion of the raster.}

\item{\code{nrow}}{Get or set number of rows.}

\item{\code{ncol}}{Get or set number of columns.}

\item{\code{cellsize}}{Get pixel size.}

\item{\code{pixel_type}}{Get or set \link[=pixel_type]{pixel type}. For details see \href{https://desktop.arcgis.com/en/arcmap/10.3/manage-data/raster-and-images/bit-depth-capacity-for-raster-dataset-cells.htm}{ArcGIS help on pixel types}.}

\item{\code{pixel_depth}}{Get pixel depth. Pixel depth/Bit depth (1, 2, 4, 8, 16, 32, 64). For details see \href{https://desktop.arcgis.com/en/arcmap/10.3/manage-data/raster-and-images/bit-depth-capacity-for-raster-dataset-cells.htm}{ArcGIS help on pixel types}.}

\item{\code{nodata}}{Get or set nodata value}

\item{\code{resample_type}}{Get or set \link[=resample_type]{resampling type}. For details see \href{https://desktop.arcgis.com/en/arcmap/latest/extensions/spatial-analyst/performing-analysis/cell-size-and-resampling-in-analysis.htm}{ArcGIS help on rasampling}.}

\item{\code{colormap}}{Get or set color map table. Return is a vector of 256 colors in the RGB format.}

\item{\code{bands}}{Get list of raster bands}

\item{\code{band}}{Get a single raster band}
}}

\section{Methods}{

  \describe{
    \item{\code{names}}{return bands names}
    \item{\code{dim}}{retrieves dimensions}
    \item{\code{$show()}}{show object}

    \item{\code{$pixel_block(ul_x, ul_y, nrow, ncol, bands)}}{
      Read pixel values.

      \code{ul_x, ul_y} - optional, upper left corner in pixels
      \code{nrow, ncol} - optional, size in pixels
      \code{bands} - optional, select band(s).

      The values returned are always a \code{matrix}, with the rows representing cells,
      and the columns representing band(s), \code{c(nrow*ncol, length(bands))}
      (see \cite{Example #1})
    }

    \item{\code{$write_pixel_block(values, ul_x, ul_y, ncol, nrow)}}{
        Write pixel values. (see \cite{Example #2})

        \code{ul_x, ul_y} - optional, upper left corner in pixels
        \code{nrow, ncol} - optional, size in pixels
    }

    \item{\code{$has_colormap()}}{
      logical, return \code{TRUE} if raster has colormap
    }
    \item{\code{$attribute_table()}}{
        Query raster attribute table.
        Return \code{data.frame} object.

        Raster datasets that contain attribute tables typically have cell values that represent or define a class, group, category, or membership.
    }

    \item{\code{$save_as(path, opt)}}{ TODO (see \cite{Example #3}) }

    \item{\code{$commit(opt)}}{
        End writing. (see \cite{Example #2.3})

        \code{opt} - additional parameter(s):
        (default: \code{"build-stats"}), (\code{"build-pyramid"})

    }

    \item{\link{arc.write}}{
      Write to an ArcGIS raster dataset
    }
  }
}
\examples{
\dontshow{ if (requireNamespace("rgdal", quietly = TRUE) && requireNamespace("raster", quietly = TRUE)) \{ }
## Example #1. read 5x5 pixel block with 10,10 offset
r.file <- system.file("pictures", "cea.tif", package="rgdal")
r <- arc.raster(arc.open(r.file))
v <- r$pixel_block(ul_x = 10L, ul_y = 10L, nrow = 5L, ncol= 5L)
dim(v) == c(25, 1)
#[1] TRUE TRUE

stopifnot(length(v) == 25)

## Example #2. process big raster
## 2.1 create new arc.raster
r2 = arc.raster(NULL, path=tempfile("r2", fileext=".img"),
                dim=dim(r), pixel_type=r$pixel_type, nodata=r$nodata,
                extent=r$extent,sr=r$sr)
## 2.2 loop by rows, process pixels
for (i in 1L:r$nrow)
{
  v <- r$pixel_block(ul_y = i - 1L, nrow = 1L)
  r2$write_pixel_block(v * 1.5, ul_y = i - 1L, nrow = 1L, ncol = r$ncol)
}
## 2.3 stop all writings and crete raster file
r2$commit()

## Example #3. resample raster
r <- arc.raster(arc.open(r.file), nrow=200L, ncol=200L, resample_type="BilinearGaussBlur")
## save to a different format
r$save_as(tempfile("new_raster", fileext=".img"))

## Example #4. get and compare all pixel values
r.file <- system.file("pictures", "logo.jpg", package="rgdal")
rx <- raster::brick(r.file)
r <- arc.raster(arc.open(r.file))
stopifnot(all(raster::values(rx) == r$pixel_block()))

\dontshow{ \} }
}

\seealso{
\code{\link{arc.raster}},
\code{\link{arc.write}},
\code{\link{arc.datasetraster-class}}
}

\keyword{classes}
\keyword{raster}
\keyword{spatial}
