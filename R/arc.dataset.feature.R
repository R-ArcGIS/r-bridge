
setMethod("initialize", "arc.feature", def = function(.Object, path)
{
  .Object <- callNextMethod(.Object, path)
  shapeinfo <- .call_proxy("feature_class.shape_info", .Object)
  #class(shapeinfo) <- c(class(shapeinfo), "arc.shapeinfo")
  class(shapeinfo) <- c("arc.shapeinfo", class(shapeinfo))
  .Object@shapeinfo <- shapeinfo
  .Object@extent <- .call_proxy("dataset.extent", .Object)
  return(.Object)
})

setMethod("show", "arc.feature", function(object)
{
  callNextMethod(object)
  x <- c("extent"=.format_extent(object@extent), format(object@shapeinfo))
  cat(paste(paste0(format(names(x), width=16), ": ", x), collapse="\n"), "\n")
  invisible(object)
})

setClass("arc.feature_impl", contains = "arc.feature")
setMethod("initialize", "arc.feature_impl", def = function(.Object, base)
{
  stopifnot(!is.null(base))
  .call_proxy("feature_class.create_from", .Object, base)
  callNextMethod(.Object, base@path)
})

setIs("arc.dataset", "arc.feature",
  test = function(from) .call_proxy("dataset.is_feature_class", from),
  coerce = function(from) new("arc.feature_impl", from),
  replace = function(obj, value) value)

.write_feature <- function(path, data, coords, shape_info, overwrite, validate=FALSE)
{
  if(missing(data) && missing(coords))
    stop("arc.write() - 'coords' and 'data' are missing", call. = FALSE)

  if (missing(data))
    data <- NULL
  if (missing(coords))
    coords <- NULL
  if (missing(shape_info))
    shape_info <- NULL

  # inherits(data, "arc.data")
  if (inherits(data, "Spatial"))
  {
    if (is.null(coords) && is.null(attr(data, "data"))) #not Spatial*DataFrame
      coords <- data
    else
      data <- .sp2data(data)
    shape_info <- .get_shapeinfo_any(coords)
  }
  else if (inherits(data, "sf"))
  {
    if (is.null(coords))
      coords <- .coords_from_sfc(sf::st_geometry(data))
    if (is.null(shape_info))
      shape_info <- .get_shapeinfo_any(data)#arc.shapeinfo(data)
    data <- sf::st_set_geometry(data, NULL)
  }
  else if (inherits(data, "sfc"))
  {
    coords <- data
    data <- NULL
  }

  if (!is.null(data) && is.null(coords))
    coords <- arc.shape(data)
  if (!is.null(coords) && is.null(shape_info))
    shape_info <- .get_shapeinfo_any(coords)#arc.shapeinfo(coords)

  if (is.null(coords) && is.null(data))
    stop("arc.write() - 'coords' and 'data' are NULL", call. = FALSE)

  if (!is.null(coords))
  {
    if (inherits(coords, "Spatial"))
    {
      coords <- .sp2shape(coords)
      if (is.null(shape_info))
        shape_info <- .get_shapeinfo_any(coords)#arc.shapeinfo(coords)
    }
    else if (inherits(coords, "sfc"))
    {
      if (is.null(shape_info))
        shape_info <- .get_shapeinfo_any(coords)#arc.shapeinfo(coords)
      coords <- .coords_from_sfc(coords)
    }
    if (is.null(shape_info))
      stop("arc.write() - shape is missing 'shape_info' attribute", call. = FALSE)
    if (!any(names(shape_info) == "type"))
      stop("arc.write() - geometry 'type' is missing 'shape_info' attribute", call. = FALSE)
  }

  if (!is.null(data))
  {
    if (is.list(data)) #data.frame ok
    {
      stopifnot(ncol(data) > 0)
      if (!is.data.frame(data))
      {
       clen <- sapply(data, length)
       if (min(clen) != max(clen))
         stop("arc.write() - differing number of rows: 'data'", call. = FALSE)
      }
    }
    else if (is.vector(data))
    {
      stopifnot(length(data) > 0)
      data <- list("data"=data);
    }
    else stop(paste("unsupported 'data' type:", class(data)) , call. = FALSE)
  }
  .call_proxy("arc_write", path, pairlist(data=data, coords=coords, shape_info=shape_info, overwrite=overwrite, simplify=validate))
  return (invisible(TRUE))
}
