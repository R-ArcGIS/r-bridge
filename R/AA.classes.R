# package namespace to hold some global variables
.arc <- new.env(FALSE, parent = emptyenv())

#
# base for all C++ binding classes
#
setClass("arc.object", contains = "VIRTUAL", representation(.ptr = "externalptr"))

setOldClass("arc.shapeinfo")

# Class "arc.shape"
#' @export
setClass("arc.shape",
    contains = "list",
    representation(shapeinfo = "arc.shapeinfo"))

# Class "arc.dataset"
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
    representation(shapeinfo = "arc.shapeinfo", extent = "numeric"))

##
## base class arc.container
##
#' @export
setClass("arc.container",
    contains = c("arc.dataset", "VIRTUAL"),
    representation(children = "list"))

# Class "arc.datasetraster"
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


# Reference Class "arc.raster"
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
