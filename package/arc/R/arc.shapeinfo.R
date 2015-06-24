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
#' @export
arc.shapeinfo <- function (object) UseMethod("arc.shapeinfo")

#' @export
arc.shapeinfo.default <- function(object)
{
  if (inherits(object, "Spatial"))
    return (.get_shape_info_from_sp(object))
  return (NULL)
}

#' @export
arc.shapeinfo.arc.shape <- function(object) object@shapeinfo
#' @export
arc.shapeinfo.arc.feature <- function(object) object@shapeinfo
#arc.shapeinfo.Spatial <- function(x) .get_shape_info_from_sp(x)

.get_shape_info_from_sp <- function (sp.df)
{
  shapeinfo <- NULL
  if (inherits(sp.df, "SpatialPoints"))
    shapeinfo <- list(type = "Point")
  else if (inherits(sp.df, "SpatialPolygons"))
    shapeinfo <- list(type = "Polygon")
  else if (inherits(sp.df, "SpatialLines"))
    shapeinfo <- list(type = "Polyline")
  else
    return (NULL)
  wkt <- arc.fromP4ToWkt(sp.df@proj4string@projargs)
  if (!is.null(wkt))
    shapeinfo["WKT"] <- wkt
  class(shapeinfo) <- append(class(shapeinfo), "arc.shapeinfo")
  return (shapeinfo)
}

#setMethod("show", "arc.shapeinfo", function(object)
#' @export
print.arc.shapeinfo <- function(x, ...)
{
  if (is.null(x))
    return ()
  cat("type         :", x$type)
  if (!is.null(x$hasZ) && x$hasZ)
    cat(", has Z")
  if (!is.null(x$hasM) && x$hasM)
    cat(", has M")
  cat("\nWKT          :", x$WKT)
  if (x$WKID > 0)
    cat("\nWKID         :", x$WKID)
  cat("\n")
  invisible(x)
}
