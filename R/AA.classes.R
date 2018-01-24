# package namespace to hold some global variables
.arc <- new.env(FALSE, parent = emptyenv())

#
# base for all C++ binding classes
#
setClass("arc.object", contains = "VIRTUAL", representation(.ptr = "externalptr"))

#' Class "arc.shape"
#'
#' \code{arc.shape} is geometry collection
#'
#' @name arc.shape-class
#' @aliases arc.shape-class show,arc.shape-method
#' [,arc.shape-method
#' arc.shapeinfo,arc.shape-method
#' @docType class
#' @note \code{arc.shape} is attached to an ArcGIS \code{data.frame} as the
#' attribute "shape". Each element corresponds to one record in
#' the input data frame. Points are presented as an array of lists, with
#' each list containing (\code{x}, \code{y}, \code{Z}, \code{M}), where
#  Z and M are optional dimensions.
#' @keywords classes shape geometry
#' @examples
#'
#' d <- arc.open(system.file("extdata", "ca_ozone_pts.shp",
#'                           package="arcgisbinding"))
#' arc.df <- arc.select(d, "FID")
#'
#' shape <- arc.shape(arc.df)
#' # create data.frame with X and Y columns
#' df <- data.frame(arc.df, X=shape$x, Y=shape$y)
#' # print out row #42
#' df[42,]
#'
#' @export
setClass("arc.shape", contains = "list", representation(shapeinfo = "list"))

#' Class "arc.dataset"
#'
#' \code{arc.dataset} S4 class
#'
#' The \code{dataset_type} slot possible values are described in the
#' referenced "dataset properties -- data type" documentation. For feature datasets,
#' \code{extent} contains four \code{double} values: \code{(xmin, ymin, xmax, ymax)}.
#' The \code{fields} slot includes the details of the ArcGIS data types of the
#' relevant fields, which include data types not directly representable in \code{R}.
#'
#' @section References:
#' \enumerate{
#'   \item \href{http://desktop.arcgis.com/en/desktop/latest/analyze/arcpy-functions/dataset-properties.htm#GUID-35446E5D-31AF-4B41-B795-783409C641A8}{ArcGIS Help: Dataset properties -- dataset type}
#' }
#' @name arc.dataset-class
#' @docType class
#' @slot path file path or layer name
#' @slot dataset_type dataset type
#' @slot extent spatial extent of the dataset
#' @slot fields list of field names
#' @slot shapeinfo geometry information (see \code{\link{arc.shapeinfo}})
#' @aliases arc.dataset-class arc.table-class arc.feature-class arc.container-class
#' show,arc.dataset-method
#' show,arc.table-method show,arc.feature-method
#' names,arc.table-method
#arc.select,arc.table-method
#' arc.metadata,arc.dataset-method arc.metadata,arc.container-method
#arc.shapeinfo,arc.feature-method
#' @section Methods:
#' \describe{
#'   \item{\link{arc.select}}{}
#'   \item{\link{names}}{}
#' }
#' @keywords classes dataset
#' @examples
#'
#' ozone.file <- system.file("extdata", "ca_ozone_pts.shp",
#'                           package="arcgisbinding")
#' d <- arc.open(ozone.file)
#' names(d@@fields) # get all field names
#' arc.shapeinfo(d) # print shape info
#' d                # print dataset info
#'
#' @rdname arc.dataset-class
#' @export
setClass("arc.dataset",
    contains = c("arc.object", "VIRTUAL"),
    representation(.info = "list", path = "character", dataset_type = "character"))

##
## base class arc.table
##
#' @export
setClass("arc.table",
    contains = c("arc.dataset", "VIRTUAL"),
    representation(fields = "list"))

##
## base class arc.feature
##
#' @export
setClass("arc.feature",
    contains = c("arc.table", "VIRTUAL"),
    representation(shapeinfo = "list", extent = "numeric"))

##
## base class arc.container
##
#' @export
setClass("arc.container",
    contains = c("arc.dataset", "VIRTUAL"),
    representation(children = "list"))

#' Class "arc.datasetraster"
#'
#' \code{arc.datasetraster} S4 class
#'
#' TODO
#'
#' @name arc.datasetraster-class
#' @docType class
#' @slot .info internal
#' @slot sr spatial reference
#' @slot extent spatial extent of the dataset
#' @slot pixel_type The pixel type of the referenced raster dataset. (\link[=pixeltypes]{pixel_type})
#' @slot compression_type The compression type
#' @slot nrow return the number of rows
#' @slot ncol return the number of rows
#' @slot bands return rasterdataset bands information
#' @aliases dim,arc.datasetraster-method
#' names,arc.datasetraster-method
#arc.raster,arc.datasetraster-method
#' @section Methods:
#' \describe{
#'   \item{\link{arc.raster}}{}
#'   \item{\link{dim}}{}
#'   \item{\link{names}}{}
#' }
#' @seealso \code{\link{arc.open}}
#' @keywords dataset open raster rasterdataset
#' @rdname arc.datasetraster-class
#' @export
setClass("arc.datasetraster",
    contains = c("arc.dataset", "VIRTUAL"),
    representation(
      sr = "list",
      extent = "numeric",
      pixel_type = "character",
      compression_type = "character",
      nrow = "integer",
      ncol = "integer",
      bands = "list"))

#' @export
setClass("arc.datasetrastermosaic",
    contains = c("arc.datasetraster", "arc.feature", "VIRTUAL"),
    representation())

#' Resample types
#'
#' @section Supported:
#' \itemize{
#'   \item "NearestNeighbor" Nearest neighbor assignment. This is the default
#'   \item "BilinearInterpolation" Bilinear interpolation
#'   \item "CubicConvolution" Cubic convolution
#'   \item "Majority" TODO
#'   \item "BilinearInterpolationPlus" TODO
#'   \item "BilinearGaussBlur" TODO
#'   \item "BilinearGaussBlurPlus" TODO
#'   \item "Average" TODO
#'   \item "Minimum" TODO
#'   \item "Average" TODO
#'   \item "VectorAverage" TODO
#' }
#'
#' @name resampletypes
#' @docType package
#' @rdname resample_type
NULL

#' Pixel types
#'
#' The pixel type of the referenced raster dataset.
#'
#' @section The types are:
#' \itemize{
#'   \item "U1" 1 bit
#'   \item "U2" 2 bits
#'   \item "U4" 4 bits
#'   \item "U8" Unsigned 8 bit integers
#'   \item "S8" 8 bit integers
#'   \item "U16" Unsigned 16 bit integers
#'   \item "S16" 16 bit integers
#'   \item "U32" Unsigned 32 bit integers
#'   \item "S32" 32 bit integers
#'   \item "F32" Single precision floating point
#'   \item "F64" Double precision floating point
#   \item "PT_COMPLEX"
#   \item "PT_DCOMPLEX"
#   \item "PT_CSHORT"
#   \item "PT_CLONG"
#' }
#'
#' @name pixeltypes
#' @docType package
#' @rdname pixel_type
NULL


#' Reference Class "arc.raster"
#'
#' TODO
#'
#' @docType class
#' @aliases arc.raster-class as.raster,arc.raster-method dim,arc.raster-method dim<-,arc.raster-method
#' names,arc.raster-method
#' @field sr Get or set Spacial Reference
#' @field extent Get or set extent. Uset it to read portion of raster.
#' @field nrow Get or set number of row.
#' @field ncol Get or set number of column.
#' @field cellsize Get pixel size.
#' @field \link[=pixeltypes]{pixel_type} Get or set pixel type.
#' @field pixel_depth Get pixel depth
#' @field nodata Get or set nodata value
#' @field \link[=resampletypes]{resample_type} Get or set resampling type.
#' @field colormap Get or set color map table. Return is a vector of 256 colors in the RGB format.
#' @field bands Get raster band information
#'
#' @section Methods:
#' \describe{
#'   \item{\link{names}}{}
#'   \item{\link{dim}}{}
#'   \item{\link{arc.write}}{}
#'   \item{\code{$save_as(path, opt)}}{
#        Save to a file. Fast way to copy an existing raster to a new format.
#'   }
#'
#'   \item{\code{$commit(opt)}}{
#'       End writing. \code{opt} - aditional parameter (default: build-stats)
#'   }
#'
#'   \item{\code{$pixel_block(ul_x, ul_y, nrow, ncol, bands)}}{
#'     Read pixel values.
#'     \code{ul_x, ul_y} - upper left corner in pixels
#'     \code{nrow, ncol} - size in pixels
#'   }
#'
#'   \item{\code{$write_pixel_block(values, ul_x, ul_y, ncol, nrow)}}{
#'       Write pixel values.
#'   }
#'
#'   \item{\code{$attribute_table()}}{
#'       Query raster attribute table
#'   }
#' }
#'
#' @examples
#' ## read 5x5 pixel block with 10,10 offset
#' r.file <- system.file("pictures", "cea.tif", package="rgdal")
#' r <- arc.raster(arc.open(r.file))
#' v <- r$pixel_block(ul_x = 10L, ul_y = 10L, nrow = 5L, ncol= 5L)
#' stopifnot(length(v) == 25)
#'
#' ## process big raster
#' r2 = arc.raster(NULL, path=tempfile("r2", fileext=".img"),
#'                 dim=dim(r), pixel_type=r$pixel_type, nodata=r$nodata,
#'                 extent=r$extent,sr=r$sr)
#' for (i in 1L:r$nrow)
#' {
#'   v <- r$pixel_block(ul_y = i - 1L, nrow = 1L)
#'   r2$write_pixel_block(v * 1.5, ul_y = i - 1L, nrow = 1L, ncol = r$ncol)
#' }
#' r2$commit()
#'
#' ## resample raster
#' r <- arc.raster(arc.open(r.file), nrow=200L, ncol=200L, resample_type="BilinearGaussBlur")
#'
#' ## save to a different format
#' r$save_as(tempfile("new_raster", fileext=".img"))
#'
#' ## get and compare all pixel values
#' r.file <- system.file("pictures", "logo.jpg", package="rgdal")
#' rx <- raster::brick(r.file)
#' r <- arc.raster(arc.open(r.file))
#' stopifnot(all(raster::values(rx) == r$pixel_block()))
#'
#' @rdname arc.raster-class
#' @export
setRefClass("arc.raster",
    contains = "arc.object",
    fields = list(.info = "list",
      sr = "list",
      extent = function(value) .raster_prop(.self, "extent", value),
      nrow = function(value) .raster_prop(.self, "nrow", value),
      ncol = function(value) .raster_prop(.self, "ncol", value),
      cellsize = function(value) .raster_prop(.self, "cellsize", value),
      pixel_type = function(value) .raster_prop(.self, "pixel_type", value),
      pixel_depth = function(value) .raster_prop(.self, "pixel_depth", value),
      nodata = function(value) .raster_prop(.self, "nodata", value),
      resample_type = function(value) .raster_prop(.self, "resample_type", value),
      colormap = function(value) .raster_prop(.self, "colormap", value),
      bands = function(value) .raster_prop(.self, "bands", value)
    ))


#' Get names
#'
#' Return names of columns when \code{x} is \link{arc.table-class}.
#' Return band names when \code{x} is \link{arc.raster-class} or \link{arc.datasetraster-class}.
#'
#' @name names
#' @param x object
#' @return string vector
#' @rdname names
NULL

#' Dimensions of an object
#'
#' Get or set raster dimention
#'
#' @name dim
#' @param x object \link{arc.raster-class} or \link{arc.datasetraster-class} object
#' @return integer vector of \code{c(nrow, ncol, nband)}

#' dim,arc.raster-method
#' @name dim
#' @rdname dim
NULL

# subsetting
#
# subsetting of an object
#
# @name [
# @rdname subsettings
#NULL
