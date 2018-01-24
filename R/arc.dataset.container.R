setMethod("arc.metadata", "arc.container", function(object) invisible(NULL))

setMethod("initialize", "arc.container", def = function(.Object, path) 
{
  .Object <- callNextMethod(.Object, path)
  .Object@children <- .call_proxy("container.children", .Object)
  .discard(.Object)
  return(.Object)
})

setMethod("show", "arc.container", function(object)
{
  callNextMethod(object)
  x <- if(.hasSlot(object, "sr")) c(.format_sr(object@sr)) else c()
  ch <- sapply(object@children, function(i) paste0(i, collapse=", "))
  n<-lengths(object@children)
  x <- c(x, "children"=paste0(names(n), "[",n,"]", collapse=", "), ch)
  cat(paste(paste0(format(names(x), width=16), ": ", x), collapse="\n"), "\n")
  invisible(object)
})

setClass("arc.workspace_impl", contains = "arc.container")
setMethod("initialize", "arc.workspace_impl", def = function(.Object, base)
{
  stopifnot(!is.null(base))
  .Object@.ptr <- .call_proxy("container.create_from", .Object, base)
  callNextMethod(.Object, base@path)
})

setClass("arc.featuredataset_impl", contains = "arc.container", representation(extent = "numeric", sr = "list"))
setMethod("initialize", "arc.featuredataset_impl", def = function(.Object, base)
{
  stopifnot(!is.null(base))
  .Object@.ptr <- .call_proxy("container.create_from", .Object, base)
  .Object@extent <- .call_proxy("dataset.extent", .Object)
  .Object@sr <- .call_proxy("dataset.sr", .Object)
  callNextMethod(.Object, base@path)
})

setIs("arc.dataset", "arc.container",
  test = function(from) any(from@dataset_type == c("Container", "FeatureDataset")),
  coerce = function(from)
  {
    switch(from@dataset_type,
      Container = new("arc.workspace_impl", base = from),
      FeatureDataset = new("arc.featuredataset_impl", base = from),
      from)
  },
  replace = function(obj, value) value
)
