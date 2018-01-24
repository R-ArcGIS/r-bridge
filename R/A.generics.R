## mostly forward declaration
## and documentation

#' Open dataset, table, or layer
#'
#' Open ArcGIS datasets, tables, rasters and layers. Returns a new
#' \code{\link{arc.dataset-class}} object which contains details on both the spatial
#' information and attribute information (data frame) contained within the dataset.
#'
#' @section Supported Formats:
#' \itemize{
#'   \item \code{Feature Class}: A collection of geographic features with the
#'   same geometry type (i.e. point, line, polygon) and the same spatial reference,
#'   combined with an attribute table. Feature classes can be stored in a variety
#'   of formats, including: files (e.g. Shapefiles), Geodatabases, components
#'   of feature datasets, and as coverages. All of these types can be accessed
#'   using the full path of the relevant feature class (see note below on how to
#'   specify path names).
#'   \item \code{Layer}: A layer references a feature layer, but also includes
#'   additional information necessary to symbolize and label a dataset appropriately.
#'   \code{arc.open} supports active layers in the current ArcGIS session, which
#'   can be addressed simply by referencing the layer name as it is displayed within
#'   the application. Instead of referencing file layers on disk (i.e.
#'   \code{.lyr} and \code{.lyrx} files), the direct reference to the actual dataset
#'   should be used.
#'   \item \code{Table}: Tables are effectively the same as data frames, containing
#'   a collection of records (or observations) organized in rows, with columns
#'   storing different variables (or fields). Feature classes similarly contain a
#'   table, but include the additional information about geometries lacking in a
#'   standalone table. When a standalone table is queries for its spatial information,
#'   e.g. \code{arc.shape(table)}, it will return \code{NULL}. Table data types include
#'   formats such as text files, Excel spreadsheets, dBASE tables, and INFO tables.
#'   \item \code{rasters}: A raster dataset TODO
#' }
#' @note Paths must be properly quoted for the Windows platform. There are two styles
#' of paths that work within R on Windows:
#' \itemize{
#'   \item Doubled backslashes, such as:
#'         \code{C:\\\\Workspace\\\\archive.gdb\\\\feature_class}.
#'   \item  Forward-slashes such as:
#'         \code{C:/Workspace/archive.gdb/feature_class}.
#' }
#' Network paths can be accessed with a leading
#' \code{\\\\\\\\host\\share} or \code{//host/share} path.
#' To access tables and data within a Feature Dataset, reference the full path to
#' the dataset, which follows the structure:
#'     \code{<directory>/<Geodatabase Name>/<feature dataset name>/<dataset name>}.
#' So for a table called \code{table1} located in a feature dataset {fdataset} within
#' a Geodatabase called \code{data.gdb}, the full path might be:
#'  \code{C:/Workspace/data.gdb/fdataset/table1}
#' @section References:
#' \itemize{
#'   \item \href{http://support.esri.com/es/knowledgebase/techarticles/detail/40057}{What is the difference between a shapefile and a layer file?}
#'   \item \href{https://desktop.arcgis.com/en/desktop/latest/map/working-with-layers/what-is-a-layer-.htm}{ArcGIS Help: What is a layer?}
#'   \item \href{http://desktop.arcgis.com/en/desktop/latest/manage-data/tables/what-are-tables-and-attribute-information.htm}{ArcGIS Help: What are tables and attribute information?}
#' }
#' @param path file path or layer name
#' @return An \code{arc.dataset} object
#' @seealso \code{\link{arc.dataset-class}}
#' @seealso \code{\link{arc.datasetraster-class}}
#' @keywords datasets open table feature raster layer
#' @examples
#'
#' ## open feature
#' filename <- system.file("extdata", "ca_ozone_pts.shp",
#'                           package="arcgisbinding")
#' d <- arc.open(filename)
#' cat('all fields:', names(d@@fields), fill = TRUE) # print all fields
#'
#' ## open raster
#' filename <- system.file("pictures", "logo.jpg", package="rgdal")
#' d <- arc.open(filename)
#' dim(d) # show raster dimension
#'
#' @name arc.open
#' @rdname arc.open
NULL

#' Create RasterLayer or RasterBrick (raster package)
#'
#' Create Rraster* object from arc.raster
#' TODO
#' @param x \link{arc.raster-class} object
#' @param ... TODO
#' @return \code{RasterLayer} or \code{RasterBrick}
#' @examples
#' ## convert arc.raster to Rasterlayer object
#' r.file <- system.file("pictures", "logo.jpg", package="rgdal")
#' r <- arc.raster(arc.open(r.file))
#' rx <- as.raster(r)
#' @name as.raster
#' @rdname as.raster
#' @export
NULL

if (!isGeneric("as.raster"))
{
  setGeneric("as.raster", function(x, ...) standardGeneric("as.raster"))
}

#' Load dataset to data.frame
#'
#' Load dataset to a standard data frame.
#'
#' @param object \link{arc.dataset-class} object
#' @param fields string, or list of strings, containing fields to include (default: all)
#' @param where_clause SQL where clause
#' @param selected use only selected records (if any) when dataset is a layer or
#' standalone table
#' @param sr transform geometry to Spatial Reference (default: object@@sr)
#' @param ... Additional arguements (currently ignored)
#' @return \code{arc.select} returns a \code{data.frame} object (type of
#' \code{arc.data}).
#' @note If dataset includes the \code{arc.feature} attribute, the "shape" of class
#' \code{\link{arc.shape-class}} will be attached to the resulting
#' \code{data.frame} object.
#' @seealso \code{\link{arc.open}}
#' @keywords datasets open table feature select
#' @examples
#'
#' ## read all fields
#' ozone.file <- system.file("extdata", "ca_ozone_pts.shp",
#'                           package="arcgisbinding")
#' d <- arc.open(ozone.file)
#' df <- arc.select(d, names(d@@fields))
#' head(df, n=3)
#'
#' ## read 'name', 'fid' and geometry
#' df <- arc.select(d, c('fid', 'ozone'), where_clause="fid < 5")
#' nrow(df)
#'
#' ## transform points to "+proj=eqc"
#' df <- arc.select(d,"fid", where_clause="fid<5", sr="+proj=eqc")
#' arc.shape(df)
#'
#' @export
#' @name arc.select
#' @rdname arc.select
setGeneric(name = "arc.select", def = function(object, fields, where_clause, selected, sr, ...) standardGeneric("arc.select"))

#' Create arc.raster object
#'
#' @name arc.raster
#' @param object \link{arc.datasetraster-class} object.
#' @param bands integer vector of bands (default: all bands).
#' @param ... optional additional arguments such as \code{nrow, ncol, extent, pixel_type, resample_type} to be passed to the method.
#' @return \code{arc.raster} returns a \code{raster} object (type of \link{arc.raster-class}).
#' @docType methods
#' @examples
#'
#' ## create and resample raster
#' r.file <- system.file("pictures", "cea.tif", package="rgdal")
#' r <- arc.raster(arc.open(r.file), nrow=200, ncol=200, resample_type="CubicConvolution")
#' r
#' stopifnot(r$nrow == 200)
#'
#' @aliases arc.raster,arc.datasetraster-method
#' @export
#' @name arc.raster
setGeneric(name = "arc.raster", def = function(object, bands, ...) standardGeneric("arc.raster"))

#' Get metadata
#'
#' @name arc.metadata
#@aliases arc.metadata arc.dataset-method,arc.container-method
#' @param object \link{arc.dataset-class} object
#' @docType methods
#' @export
#' @name arc.metadata
setGeneric(name = "arc.metadata", def = function(object) standardGeneric("arc.metadata"))

#' Write dataset, raster, feature, table or layer
#'
#' Export a \code{data.frame} object to an ArcGIS dataset. If the data frame
#' includes a spatial attribute, this function writes a feature dataset. If no
#' spatial attribute is found, a table is instead written.\cr
#' Export a \code{arc.raster}, \code{raster::RasterLayer} or \code{raster::RasterBrick} object to an ArcGIS raster dataset.
#'
#' Supports a variety of output formats. Below are pairs of example paths and the resulting data types:
#'  \itemize{
#'   \item \code{C:/place.gdb/fc}: File Geodatabase Feature Class
#'   \item \code{C:/place.gdb/fdataset/fc}: File Geodatabase Feature Dataset
#'   \item \code{in_memory\\logreg}: In-memory workspace (must be run in ArcGIS Session)
#'   \item \code{C:/place.shp}: Esri Shapefile
#'   \item \code{C:/place.dbf}: Table
#'   \item \code{C:/place.gdb/raster}: File Geodatabase Raster when \code{data} parameter is \code{arc.raster} or \code{Raster*} object
#'   \item \code{C:/image.img}: ERDAS Imaging
#'   \item \code{C:/image.tif}: Geo TIFF
#' }
#' @section References:
#' \itemize{
#' \item \href{http://support.esri.com/es/knowledgebase/techarticles/detail/40057}{What is the difference between a shapefile and a layer file?}
#' \item \href{https://desktop.arcgis.com/en/desktop/latest/map/working-with-layers/what-is-a-layer-.htm}{ArcGIS Help: What is a layer?}
#' }
#' @param path full output path
#' @param data input source. Accepts \code{data.frame}, spatial \code{data.frame},
#' \code{SpatialPointsDataFrame}, \code{SpatialLinesDataFrame}, and
#' \code{SpatialPolygonsDataFrame}, \code{arc.raster}, \code{raster::RasterLayer}, \code{raster::RasterBrick} objects.
#' @param \dots Optional parameters
#'  \itemize{
#'    \item \code{coords} list containing geometry. Accepts \code{Spatial} objects. Put field names if \code{data} is data.frame and consists coordinates.
#'    \item \code{shape_info} required argument if \code{data} has no spatial attribute
#'    \item \code{overwrite} overwrite existing dataset. default = FALSE.
#' }
#' @keywords datasets open write
#' @seealso \code{\link{arc.dataset-class}}, \code{\link{arc.open}}, \code{\link{arc.raster}}
#' @examples
#'
#' ## write as a shapefile
#' fc <- arc.open(system.file("extdata", "ca_ozone_pts.shp", package="arcgisbinding"))
#' d <- arc.select(fc, 'ozone')
#' d[1,] <- 0.6
#' arc.write(tempfile("ca_new", fileext=".shp"), d)
#'
#' ## create and write to a new file geodatabase
#' fgdb_path <- file.path(tempdir(), "data.gdb")
#'
#' data(meuse, package="sp")
#' ## create feature dataset 'meuse'
#' arc.write(file.path(fgdb_path, "meuse\\pts"), data=meuse, coords=c("x", "y", "elev"), shape_info=list(type='Point',hasZ=TRUE,WKID=28992))
#
#' data(meuse.riv, package="sp")
#' riv <- sp::SpatialPolygons(list(sp::Polygons(list(sp::Polygon(meuse.riv)),"meuse.riv")))
#' ## write only geometry
#' arc.write(file.path(fgdb_path, "meuse\\riv"), coords=riv)
#'
#' ## write as table
#' arc.write(file.path(fgdb_path, "tlb"), data=list('f1'=c(23,45), 'f2'=c('hello', 'bob')))
#'
#' ## from scratch as feature class
#' arc.write(file.path(fgdb_path, "fc_pts"), data=list('data'=rnorm(100)),
#'           coords=list(x=runif(100,min=0,max=10),y=runif(100,min=0,max=10)),
#'           shape_info=list(type='Point'))
#'
#' ## write Raster
#' # make SpatialPixelsDataFrame
#' data(meuse.grid, package="sp")
#' sp::coordinates(meuse.grid) = c("x", "y")
#' sp::gridded(meuse.grid) <- TRUE
#' meuse.grid@proj4string=sp::CRS(arc.fromWktToP4(28992))
#'
#' arc.write(file.path(fgdb_path, "meuse_grid"), meuse.grid)
#'
#' ## write using a RasterLayer object
#' r <- raster::raster(ncol=10, nrow=10)
#' raster::values(r) <- runif(raster::ncell(r))
#' arc.write(file.path(fgdb_path, "raster"), r)
#'
#' @name arc.write
#' @rdname arc.write
NULL

#' deprecated Convert a sp SpatialDataFrame object to an arc.dataframe object
#'
#' deprecated Convert \code{sp} \code{SpatialPointsDataFrame}, \code{SpatialPolygonsDataFrame},
#' and \code{SpatialLinesDataFrame} objects to an ArcGIS-compatible \code{data.frame}.
#'
#'
#' @param sp.df \code{SpatialPointsDataFrame}, \code{SpatialPolygonsDataFrame},
#'  or \code{SpatialLinesDataFrame}
#' @seealso \code{\link{arc.data2sp}}
#' @keywords convert sp
#' @name arc.sp2data
#' @rdname arc.sp2data
NULL

#' Get arc.shape object
#'
#' Get \code{\link{arc.shape-class}} from \code{arc.data}
#'
#' @param df \code{arc.dataframe}
#' @seealso \code{\link{arc.select}}
#' @examples
#'
#' d <- arc.open(system.file("extdata", "ca_ozone_pts.shp",
#'                           package="arcgisbinding"))
#'
#' df <- arc.select(d, 'ozone')
#' shp <- arc.shape(df)
#' length(shp$x)
#' @name arc.shape
#' @rdname arc.shape
NULL

#' Shape Information
#'
#' \code{arc.shapeinfo} provides details on what type of geometry is stored
#' within the dataset, and the spatial reference of the geometry. The
#' well-known text, \code{WKT}, allows interoperable transfer of the spatial
#' reference system (CRS) between environments. The \code{WKID} is a numeric
#' value that ArcGIS uses to precisely specify a projection.
#'
#' @param object \link{arc.dataset-class} object
#' @slot type geometry type: "Point", "Polyline", or "Polygon"
#' @slot hasZ TRUE if geometry includes Z-values
#' @slot hasM TRUE if geometry includes M-values
#' @slot WKT well-known text representation of the shape's spatial reference
#' @slot WKID well-known ID of the shape's spatial reference
#' @section References:
#' \enumerate{
#'   \item \href{http://resources.arcgis.com/en/help/arcgis-rest-api/index.html#/Using_spatial_references/02r3000000qq000000/}{ArcGIS REST API: Using spatial references}
#'   \item \href{http://spatialreference.org/}{Spatial reference lookup}
#' }
#' @seealso \code{\link{arc.dataset-class}} \code{\link{arc.shape-class}}
#' @keywords shape SpatialReference geometry
#' @examples
#'
#' d <- arc.open(system.file("extdata", "ca_ozone_pts.shp",
#'                           package="arcgisbinding"))
#' # from arc.dataset
#' info <- arc.shapeinfo(d)
#' info$WKT   # print dataset spatial reference
#'
#' # from arc.shape
#' df <- arc.select(d, 'ozone')
#' arc.shapeinfo(arc.shape(df))
#'
#' @name arc.shapeinfo
#' @rdname arc.shapeinfo
NULL

#' Convert PROJ.4 Coordinate Reference System string to Well-known Text.
#'
#' The \code{arc.fromP4ToWkt} command converts a PROJ.4 coordinate
#' reference system (CRS) string to a well-known text (WKT) representation.
#' Well-known text is used by ArcGIS and other applications to robustly
#' describe a coordinate reference system. Converts PROJ.4 stings which
#' include either the '+proj' fully specified projection parameter, or the
#' '+init' form that takes well-known IDs (WKIDs), such as EPSG codes,
#' as input.
#'
#' The produced WKT is equivalent to the ArcPy spatial reference
#' exported string:
#'
#'   \code{arcpy.Describe(layer).SpatialReference.exportToString()}
#' @note The '+init' method currently only works with ArcGIS Pro.
#'
#' @section References:
#'  \enumerate{
#'    \item OGC specification
#'      \href{http://docs.opengeospatial.org/is/12-063r5/12-063r5.html#36}{
#'    12-063r5}
#'    \item \href{http://desktop.arcgis.com/en/desktop/latest/guide-books/map-projections/what-are-map-projections.htm}{ArcGIS Help: What are map projections?}
#' }
#' @param proj4 PROJ.4 projection string
#' @seealso \code{\link{arc.fromWktToP4}}
#' @examples
#'
#' arc.fromP4ToWkt("+proj=eqc") # Equirectangular
#'
#' arc.fromP4ToWkt("+proj=latlong +datum=wgs84") # WGS 1984 geographic
#'
#' arc.fromP4ToWkt("+init=epsg:2806") # initalize based on EPSG code
#' @name arc.fromP4ToWkt
#' @rdname arc.fromP4ToWkt
NULL

#' Convert a Well-known Text Coordinate Reference System into a PROJ.4 string.
#'
#' Convert a well-known text (WKT) coordinate reference system (CRS) string to
#' a PROJ.4 representation. PROJ.4 strings were created as a convenient way to
#' pass CRS information to the command-line PROJ.4 utilities, and have an
#' expressive format. Alternatively, can accept a well-known ID (WKID),
#' a numeric value that ArcGIS uses to specify projections. See the 'Using
#' spatial references' resource for lookup tables which map between WKIDs and
#' given projection names.
#'
#' @section References:
#'  \enumerate{
#'    \item \href{http://resources.arcgis.com/en/help/arcgis-rest-api/index.html#/Using_spatial_references/02r3000000qq000000/}{ArcGIS REST API: Using spatial references}
#'    \item OGC specification
#'      \href{http://docs.opengeospatial.org/is/12-063r5/12-063r5.html#36}{
#'    12-063r5}
#'    \item \href{http://desktop.arcgis.com/en/desktop/latest/guide-books/map-projections/what-are-map-projections.htm}{ArcGIS Help: What are map projections?}
#' }
#' @param wkt WKT projection string, or a WKID integer
#' @seealso \code{\link{arc.fromP4ToWkt}}
#' @examples
#'
#' d <- arc.open(system.file("extdata", "ca_ozone_pts.shp",
#'                           package="arcgisbinding"))
#' arc.fromWktToP4(arc.shapeinfo(d)$WKT)
#'
#' arc.fromWktToP4(4326) # use a WKID for WGS 1984, a widely
#'                       # used standard for geographic coordinates
#' @name arc.fromWktToP4
#' @rdname arc.fromWktToP4
NULL

#' delete dataset
#'
#' delete dataset
#'
#' @param x \code{arc.dataset}
#' @name arc.delete
#' @rdname arc.delete
NULL
