.arc <- new.env(FALSE, parent = emptyenv())

#' @export
.call_proxy <- function (n, ...)
{
  ret <- .Call(n, PACKAGE=.arc$dll, ...)
  if (exists(".arc_err", mode = "character", envir=.GlobalEnv))
  {
    tmp <- .GlobalEnv$.arc_err;
    rm(.arc_err, envir=.GlobalEnv)
    stop(tmp)
  }
  return(ret)
}

##
## internals for C++ binding
##
setClass("arc.object",
    contains = "VIRTUAL",
    slots = list(.ptr = "externalptr", .ptr_class = "character"))

#' Class "arc.shape"
#'
#' \code{arc.shape} is geometry collection
#'
#' @name arc.shape-class
#' @aliases arc.shape-class show,arc.shape-method
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
setClass("arc.shape", contains = c("list"), slots = c(shapeinfo = "list"))

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
#' @aliases arc.dataset-class arc.table-class arc.feature-class
#' show,arc.dataset-method show,arc.table-method show,arc.feature-method
#' arc.shapeinfo,arc.feature-method
#' @docType class
#' @slot path file path or layer name
#' @slot dataset_type dataset type
#' @slot extent spatial extent of the dataset
#' @slot fields list of field names
#' @slot shapeinfo geometry information (see \code{\link{arc.shapeinfo}})
#' @keywords classes dataset
#' @examples
#'
#' ozone.file <- system.file("extdata", "ca_ozone_pts.shp",
#'                           package="arcgisbinding")
#' d <- arc.open(ozone.file)
#' names(d@@fields) # get all field names
#' d@@shapeinfo     # print shape info
#' d               # print dataset info
#'
#' @export
setClass("arc.dataset",
    contains = c("arc.object", "VIRTUAL"),
    slots = c(path = "character", dataset_type = "character"))

##
## base class arc.table
##
#' @export
setClass("arc.table",
    contains = c("arc.dataset", "VIRTUAL"),
    slots = c(fields = "list"))

##
## base class arc.feature
##
#setClass("arc.feature", contains = c("arc.table", "VIRTUAL"),
#    slots = list(shapeinfo = "arc.shapeinfo", extent = "numeric"))
#' @export
setClass("arc.feature",
    contains = c("arc.table", "VIRTUAL"),
    slots = c(shapeinfo = "list", extent = "numeric"))

##
## base class arc.container
##
#setClass("arc.container", contains = c("arc.dataset", "VIRTUAL"),
#    slots = list(children = "list"))
#' @export
setClass("arc.container",
    contains = c("arc.dataset", "VIRTUAL"),
    slots = c(children = "list"))

#' Load dataset to data.frame
#'
#' Load dataset to a standard data frame.
#'
#' @aliases arc.select arc.select,arc.dataset-method
#' arc.select,arc.table-method arc.select,arc.feature-method
#' @param object \link{arc.dataset-class} object
#' @param fields string, or list of strings, containing fields to include (default: all)
#' @param where_clause SQL where clause
#' @param selected use only selected records (if any) when dataset is a layer or
#' standalone table
#' @param sr transform geometry to Spatial Reference
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
setGeneric(name="arc.select",
    def=function(object, fields = "*", where_clause = "",
                 selected = TRUE, sr = NULL) standardGeneric("arc.select"))
