\name{arc.open}
\docType{methods}
\alias{arc.open}
\concept{open datset}
\concept{open table}
\concept{open feature}
\concept{open layer}
\concept{open shape}
\concept{open raster}
\concept{table}
\concept{feature}
\concept{layer}
\concept{shape}
\concept{raster}

\title{Open dataset, table, or layer}
\usage{
  arc.open(path)
}
\arguments{
\item{path}{file path (character) or layer name (character) }
}
\value{
An \code{\link{arc.dataset-class}} object
}
\description{
Open ArcGIS datasets, tables, rasters and layers. Returns a new
\code{\link{arc.dataset-class}} object which contains details on both the spatial
information and attribute information (data frame) contained within the dataset.
}
\note{
Paths must be properly quoted for the Windows platform. There are two styles
of paths that work within R on Windows:
\itemize{
  \item Doubled backslashes, such as:
        \code{C:\\\\Workspace\\\\archive.gdb\\\\feature_class}.
  \item  Forward-slashes such as:
        \code{C:/Workspace/archive.gdb/feature_class}.
}
Network paths can be accessed with a leading
\code{\\\\\\\\host\\share} or \code{//host/share} path.
To access tables and data within a Feature Dataset, reference the full path to
the dataset, which follows the structure:
    \code{<directory>/<Geodatabase Name>/<feature dataset name>/<dataset name>}.
So for a table called \code{table1} located in a feature dataset {fdataset} within
a Geodatabase called \code{data.gdb}, the full path might be:
 \code{C:/Workspace/data.gdb/fdataset/table1}
}
\section{Supported Formats}{

\itemize{
  \item \code{Feature Class}: A collection of geographic features with the
  same geometry type (i.e. point, line, polygon) and the same spatial reference,
  combined with an attribute table. Feature classes can be stored in a variety
  of formats, including: files (e.g. Shapefiles), Geodatabases, components
  of feature datasets, and as coverages. All of these types can be accessed
  using the full path of the relevant feature class (see note below on how to
  specify path names).
  \item \code{Layer}: A layer references a feature layer, but also includes
  additional information necessary to symbolize and label a dataset appropriately.
  \code{arc.open} supports active layers in the current ArcGIS session, which
  can be addressed simply by referencing the layer name as it is displayed within
  the application. Instead of referencing file layers on disk (i.e.
  \code{.lyr} and \code{.lyrx} files), the direct reference to the actual dataset
  should be used.
  \item \code{Table}: Tables are effectively the same as data frames, containing
  a collection of records (or observations) organized in rows, with columns
  storing different variables (or fields). Feature classes similarly contain a
  table, but include the additional information about geometries lacking in a
  standalone table. When a standalone table is queries for its spatial information,
  e.g. \code{arc.shape(table)}, it will return \code{NULL}. Table data types include
  formats such as text files, Excel spreadsheets, dBASE tables, and INFO tables.
  \item \code{rasters}: Rasters represent continuous geographic data in cells, or pixels, of equal size (square or rectangular). Spatial data represented on this rasters are also known as grided data. In contrast to spatial data structures represented in feature classes, rasters contain information on spatially continuous data. 


}
}

\section{References}{

\itemize{
  \item \href{http://support.esri.com/es/knowledgebase/techarticles/detail/40057}{What is the difference between a shapefile and a layer file?}
  \item \href{https://desktop.arcgis.com/en/desktop/latest/map/working-with-layers/what-is-a-layer-.htm}{ArcGIS Help: What is a layer?}
  \item \href{http://desktop.arcgis.com/en/desktop/latest/manage-data/tables/what-are-tables-and-attribute-information.htm}{ArcGIS Help: What are tables and attribute information?}
}
}
\examples{

## open feature
filename <- system.file("extdata", "ca_ozone_pts.shp",
                          package="arcgisbinding")
d <- arc.open(filename)
cat('all fields:', names(d@fields), fill = TRUE) # print all fields

## open raster
\dontshow{ if (requireNamespace("rgdal", quietly = TRUE)) \{ }
filename <- system.file("pictures", "logo.jpg", package="rgdal")
d <- arc.open(filename)
dim(d) # show raster dimension
\dontshow{\}}
}
\seealso{
\code{\link{arc.select}},
\code{\link{arc.raster}},
\code{\link{arc.write}}
}

\keyword{method}
\keyword{open}
