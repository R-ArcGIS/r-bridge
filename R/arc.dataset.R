setMethod("arc.metadata", "arc.dataset", function(object) .call_proxy("dataset.metadata", object))

setMethod("initialize", "arc.dataset", def = function(.Object, path)
{
  stopifnot(!is.null(.Object@.ptr))
  .Object <- callNextMethod(.Object)
  .Object@path <- path
  .Object@dataset_type <- .call_proxy("dataset.type", .Object)
  .Object@.info <- .call_proxy("dataset.props", .Object)

  return(.Object)
})

setMethod("show", "arc.dataset", function(object)
{
  x <- c("dataset_type" = object@dataset_type, "path" = object@path)
  cat(paste(paste0(format(names(x), width=16), ": ", x), collapse="\n"), "\n")
  invisible(object)
})

##
##  implementation
##
setClass("arc.dataset_impl", contains = "arc.dataset")
setMethod("initialize", "arc.dataset_impl", def = function(.Object, path)
{
  ptr <- .call_proxy("dataset.create", .Object)
  stopifnot(!is.null(ptr))
  .call_proxy("dataset.open", .Object, path)
  callNextMethod(.Object, path)
})
