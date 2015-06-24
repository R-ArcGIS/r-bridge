setMethod("initialize", "arc.dataset", def = function(.Object, path)
{
  .Object <- callNextMethod(.Object)
  .Object@path <- path
  .Object@dataset_type <- .call_proxy("dataset.type", .Object)
  return(.Object)
})

#' @export
setMethod("show", "arc.dataset", function(object)
{
  cat("dataset_type :", object@dataset_type, "\n")
  cat("path         :", object@path, "\n")
  invisible(object)
})

setMethod("initialize", "arc.table", def = function(.Object, path)
{
  .Object <- callNextMethod(.Object, path)
  .Object@fields <- as.list(.call_proxy("table.fields", .Object))
  return(.Object)
})

#' @export
setMethod("show", "arc.table", function(object)
{
  callNextMethod(object)
  cat("fields       :", paste0(names(object@fields), collapce=",") , "\n")
  invisible(object)
})

setMethod("initialize", "arc.feature", def = function(.Object, path)
{
  .Object <- callNextMethod(.Object, path)
  #shapeinfo <- new("arc.shapeinfo",
  #    .call_proxy("feature_class.shape_info", .Object))
  shapeinfo <- .call_proxy("feature_class.shape_info", .Object)
  class(shapeinfo) <- append(class(shapeinfo), "arc.shapeinfo")
  .Object@shapeinfo <- shapeinfo
  .Object@extent <- .call_proxy("feature_class.extent", .Object)
  return(.Object)
})

#' @export
setMethod("show", "arc.feature", function(object)
{
  callNextMethod(object)
  cat("extent       :\n")
  print(object@extent)
  cat("*** shapeinfo ***\n")
  print(object@shapeinfo)
  invisible(object)
})

##
##  implementation
##
setClass("arc.dataset_impl", contains = "arc.dataset")
setMethod("initialize", "arc.dataset_impl", def = function(.Object, path)
{
  .Object <- .call_proxy("dataset.create", .Object)
  .call_proxy("dataset.open", .Object, path)
  callNextMethod(.Object, path)
})

setClass("arc.table_impl", contains = "arc.table")
setMethod("initialize", "arc.table_impl",
    def = function(.Object, path = NULL, base = NULL)
    {
      if (is.null(path) && is.null(base))
        stop("no parameters")
      if (is.null(base))
      {
        .Object <- .call_proxy("table.create", .Object)
        .call_proxy("dataset.open", .Object, path)
      }
      else
      {
        path <- base@path
        .Object <- .call_proxy("table.create_from", .Object, base)
      }
      callNextMethod(.Object, path)
    }
)

setClass("arc.feature_impl", contains = "arc.feature")
setMethod("initialize", "arc.feature_impl",
    def = function(.Object, path = NULL, base = NULL)
    {
      if (is.null(path) && is.null(base))
        stop("no parameters")
      if (is.null(base))
      {
        .Object <- .call_proxy("feature_class.create", .Object)
        .call_proxy("dataset.open", .Object, path)
      }
      else
      {
        path <- base@path
        .Object <- .call_proxy("feature_class.create_from", .Object, base)
      }
      callNextMethod(.Object, path)
    }
)

setIs("arc.dataset_impl", "arc.table",
    test = function(from) .call_proxy("dataset.is_table", from),
    coerce = function(from) new("arc.table_impl", base = from),
    replace = function(obj, value) value)

setIs("arc.dataset_impl", "arc.feature",
    test = function(from) .call_proxy("dataset.is_feature_class", from),
    coerce = function(from) new("arc.feature_impl", base = from),
    replace = function(obj, value) value)
