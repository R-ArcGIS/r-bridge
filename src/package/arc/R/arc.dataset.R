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
  cat(  "type         :", object@dataset_type)
  cat("\npath         :", object@path, "\n")
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
  cat(names(object@fields), sep = ", ", labels = "fields       :", fill = TRUE)
  #cat("\n")
  invisible(object)
})

#' @export
setMethod("show", "arc.container", function(object) {
  callNextMethod(object)
  if (.hasSlot(object, "sr"))
    .show_sr(object@sr)
  cat("\nchildren       :", paste0(names(object@children), " = ", object@children, "\n"))
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


setMethod("initialize", "arc.container", def = function(.Object, path) {
  .Object <- callNextMethod(.Object, path)
  .Object@children <- .call_proxy("container.children", .Object)
  return(.Object)
})

##' @export
##setMethod("arc.ls", "arc.workspace", def = function(object) .call_proxy("workspace.ls", object))

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

setClass("arc.workspace_impl", contains = "arc.container")
setMethod("initialize", "arc.workspace_impl",
    def = function(.Object, path = NULL, base = NULL) {
        if (is.null(path) && is.null(base))
           stop("no parameters")
        if (is.null(base)) {
           .Object <- .call_proxy("container.create", .Object)
           .call_proxy("dataset.open", .Object, path)
       } else {
           path <- base@path
           .Object <- .call_proxy("container.create_from", .Object, base)
       }
       .Object@.ptr <- 0
       callNextMethod(.Object, path)
    })

setClass("arc.featuredataset_impl", contains = "arc.container", slots = c(extent = "numeric", sr = "list"))

setMethod("initialize", "arc.featuredataset_impl",
    def = function(.Object, path = NULL, base = NULL) {
      if (is.null(path) && is.null(base))
        stop("no parameters")
      if (is.null(base)) {
        .Object <- .call_proxy("container.create", .Object)
        .call_proxy("dataset.open", .Object, path)
      } else {
        path <- base@path
        .Object <- .call_proxy("container.create_from", .Object, base)
      }
      .Object@extent <- .call_proxy("featuredataset.extent", .Object)
      .Object@sr <- .call_proxy("featuredataset.sr", .Object)
      .Object@.ptr <- 0
      callNextMethod(.Object, path)
    })

setIs("arc.dataset_impl", "arc.table",
    test = function(from) .call_proxy("dataset.is_table", from),
    coerce = function(from) new("arc.table_impl", base = from),
    replace = function(obj, value) value)

setIs("arc.dataset_impl", "arc.feature",
    test = function(from) .call_proxy("dataset.is_feature_class", from),
    coerce = function(from) new("arc.feature_impl", base = from),
    replace = function(obj, value) value)

setIs("arc.dataset_impl", "arc.container",
    test = function(from) any(from@dataset_type == c("Container", "FeatureDataset")),
    coerce = function(from)
      {
        if (from@dataset_type == "Container")
          return(new("arc.workspace_impl", base = from))
        if (from@dataset_type == "FeatureDataset")
          return(new("arc.featuredataset_impl", base = from))
      },
    replace = function(obj, value) value)

#arc.workspace <- R6::R6Class("arc.workspace",
#    private = list(.ptr = NA, .ds = NA),
#    public = list(
#      path = NA,
#      initialize = function(base)
#       {
#          self$path <- base@path
#          private$.ds <- base
#          private$.ptr <- .call_proxy("workspace.create_from", self, base)
#      },
#      ls = function()
#        {
#          .call_proxy("workspace.ls", self)
#        }
#    )
#)

