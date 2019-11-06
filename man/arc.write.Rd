\name{arc.write}
\alias{arc.write}
\title{Write dataset, raster, feature, table or layer}

\usage{
  arc.write(path, data, \dots, overwrite = FALSE)
%  arc.write(path, data, coords, shape_info, \dots, overwrite = FALSE)
}
\arguments{
  \item{path}{full output path}

  \item{data}{Accepts input source objects (see \sQuote{Details} for the types of objects allowed).}
  \item{\dots}{Additional arguments:
    \itemize{
      \item \code{coords} list containing geometry.  Accepts \code{Spatial} objects.
      If \code{data} is \code{data.frame} \code{coords} can be list of field names (see \cite{Example #2}).
      \item \code{shape_info} list.  Required argument if \code{data} has no spatial attribute (see \cite{Example #2}).
      \item \code{validate} logical.  Default \code{FALSE}.  If \code{TRUE} makes the geometries topologically correct.
    }
  }
  \item{overwrite}{overwrite existing dataset. default \code{= FALSE}.}
}
\description{
Export a \code{data} object to an ArcGIS dataset.  If the data frame
includes a spatial attribute, this function writes a feature dataset.  If no
spatial attribute is found, a table is instead written.  If \code{data} is raster-like object,
this function writes a raster dataset.  See \sQuote{Details} section for more information.
}
\note{
  To write Date column type corresponding \code{data} column must have
  \code{\link[base:DateTimeClasses]{POSIXct}} type (see \cite{Example #4}).
}

\details{
Export to a new \bold{table} dataset when \code{data} type is:

  \itemize{
    \item named \code{list} of vectors (see \cite{Example #4})
    \item \code{data.frame}
  }

Export to a new \bold{feature} dataset when \code{data} type is:

  \itemize{
    \item \code{arc.data} result of \code{\link{arc.select}}
    \item named \code{list} of vectors, parameters \code{coords} and \code{shape_info} are required (see \cite{Example #5})
    \item \code{data.frame}, parameters \code{coords} and \code{shape_info} are required (see \cite{Example #2})
    \item \code{\link[sp]{SpatialPointsDataFrame}} in package \pkg{sp}
    \item \code{\link[sp]{SpatialLinesDataFrame}} in package \pkg{sp}
    \item \code{\link[sp]{SpatialPolygonsDataFrame}} in package \pkg{sp}
    \item \code{\link[sf]{sf}}, \code{\link[sf]{sfc}} in package \pkg{sf}
  }

Export to a new \bold{raster} dataset when \code{data} type is:
  \itemize{
    \item \code{arc.raster} result of \code{\link{arc.raster}}
    \item \code{\link[sp]{SpatialPixels}}, \code{\link[sp]{SpatialPixelsDataFrame}} in package \pkg{sp} (see \cite{Example #6})
    \item \code{\link[sp]{SpatialGrid}} in package \pkg{sp}
    \item \code{\link[raster:raster]{RasterLayer}} in package \pkg{raster} (see \cite{Example #7})
    \item \code{\link[raster:brick]{RasterBrick}} in package \pkg{raster}
  }

Below are pairs of example paths and the resulting data types:
 \itemize{
  \item \code{C:/place.gdb/fc}: File Geodatabase Feature Class
  \item \code{C:/place.gdb/fdataset/fc}: File Geodatabase Feature Dataset
  \item \code{in_memory\\logreg}: In-memory workspace (must be run in ArcGIS Session)
  \item \code{C:/place.shp}: Esri Shapefile
  \item \code{C:/place.dbf}: Table
  \item \code{C:/place.gdb/raster}: File Geodatabase Raster when \code{data} parameter is \code{arc.raster} or \code{Raster*} object
  \item \code{C:/image.img}: ERDAS Imaging
  \item \code{C:/image.tif}: Geo TIFF
}
}
\section{References}{

\itemize{
  \item \href{http://support.esri.com/es/knowledgebase/techarticles/detail/40057}{What is the difference between a shapefile and a layer file?}
  \item \href{https://desktop.arcgis.com/en/desktop/latest/map/working-with-layers/what-is-a-layer-.htm}{ArcGIS Help: What is a layer?}
}
}
\examples{

## Example #1. write a shapefile
fc <- arc.open(system.file("extdata", "ca_ozone_pts.shp", package="arcgisbinding"))
d <- arc.select(fc, 'ozone')
d[1,] <- 0.6
arc.write(tempfile("ca_new", fileext=".shp"), d)

## create and write to a new file geodatabase
fgdb_path <- file.path(tempdir(), "data.gdb")

\dontshow{ if (require("sp")) \{ }
data(meuse, package="sp")
## Example #2. create feature dataset 'meuse'
arc.write(file.path(fgdb_path, "meuse\\\\pts"), data=meuse, coords=c("x", "y", "elev"), shape_info=list(type='Point',hasZ=TRUE,WKID=28992))
data(meuse.riv, package="sp")
riv <- sp::SpatialPolygons(list(sp::Polygons(list(sp::Polygon(meuse.riv)),"meuse.riv")))

## Example #3. write only geometry
arc.write(file.path(fgdb_path, "meuse\\\\riv"), coords=riv)
\dontshow{ \} }

## Example #4. write a table
t <- Sys.time() # now
arc.write(file.path(fgdb_path, "tlb"), data=list(
  'f_double'=c(23,45),
  'f_string'=c('hello', 'bob'),
  'f_datetime'=as.POSIXct(c(t, t - 3600)) # now and an hour ago
  ))

## Example #5. from scratch as feature class
arc.write(file.path(fgdb_path, "fc_pts"), data=list('data'=rnorm(100)),
          coords=list(x=runif(100,min=0,max=10),y=runif(100,min=0,max=10)),
          shape_info=list(type='Point'))

\dontshow{if (require("sp")) \{ }
## Example #6. write Raster
# make SpatialPixelsDataFrame
data(meuse.grid, package="sp")
sp::coordinates(meuse.grid) = c("x", "y")
sp::gridded(meuse.grid) <- TRUE
meuse.grid@proj4string=sp::CRS(arc.fromWktToP4(28992))

arc.write(file.path(fgdb_path, "meuse_grid"), meuse.grid)
\dontshow{ \} }

\dontshow{ if (require("raster")) \{ }
## Example #7. write using a RasterLayer object
r <- raster::raster(ncol=10, nrow=10)
raster::values(r) <- runif(raster::ncell(r))

arc.write(file.path(fgdb_path, "raster"), r)
\dontshow{ \} }

\dontshow{ arc.delete(fgdb_path) }
}
\seealso{
  \code{\link{arc.open}},
  \code{\link{arc.select}},
  \code{\link{arc.raster}}
}

\keyword{method}
\keyword{datasets}
\keyword{open}
\keyword{write}
\keyword{feature}
\keyword{raster}
