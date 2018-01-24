#' @export
arc.shapeinfo <- function (object) UseMethod("arc.shapeinfo")

#' @export
arc.shapeinfo.default <- function(object)
{
  if (inherits(object, "Spatial"))
    return (.get_shape_info_from_sp(object))
  if (inherits(object, "sf"))
    return (.get_shape_info_from_sf(object))
  if (inherits(object, "sfc"))
    return (.get_shape_info_from_sf(object))
  return (NULL)
}

.shapeinfo_dim <- function(x)
{
  stopifnot(inherits(x, "arc.shapeinfo"))
  paste0("XY",
    ifelse(!is.null(x$hasZ) && x$hasZ,'Z', ''),
    ifelse(!is.null(x$hasM) && x$hasM, 'M', ''))
}

#' @aliases arc.shapeinfo arc.shape-method
#' @export
arc.shapeinfo.arc.shape <- function(object) object@shapeinfo

#' @aliases arc.shapeinfo arcarc.feature-method
#' @export
arc.shapeinfo.arc.feature <- function(object) object@shapeinfo
#arc.shapeinfo.Spatial <- function(x) .get_shape_info_from_sp(x)

#setMethod("show", "arc.shapeinfo", function(object)
#' @export
format.arc.shapeinfo <- function(x, ...)
{
  if (is.null(x)) return (invisible(x))
  zm<-substr(.shapeinfo_dim(x),3,4)
  gt <- if (nchar(zm) > 0) paste0(x$type, ", has ", zm) else x$type
  c("geometry type"=gt, .format_sr(x))
}
#' @export
print.arc.shapeinfo <- function(x, ...)
{
  x<-format(x, ...)
  cat(paste(paste0(format(names(x), width=16), ": ", x), collapse="\n"), "\n")
 }
