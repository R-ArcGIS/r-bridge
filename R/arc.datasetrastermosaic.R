setMethod("initialize", "arc.datasetrastermosaic", def = function(.Object, path)
{
  .Object<-callNextMethod(.Object, path)
  shapeinfo <- .call_proxy("feature_class.shape_info", .Object)
  #class(shapeinfo) <- c(class(shapeinfo), "arc.shapeinfo")
  class(shapeinfo) <- c("arc.shapeinfo", class(shapeinfo))
  .Object@shapeinfo <- shapeinfo
  .Object@fields <- as.list(.call_proxy("table.fields", .Object))
  return(.Object)
})

setMethod("show", "arc.datasetrastermosaic", function(object)
{
  #assign(".nextMethod", selectMethod("show", "arc.table"))
  callNextMethod()
  m <- object@.info$mosaic
  if (!is.null(m))
  {
    x <- c(mosaic = paste(paste0(names(m), "=", m),collapse=", "))
    cat(paste(paste0(format(names(x), width=16), ": ", x), collapse="\n"), "\n")
  }
  cat(names(object@fields), sep = ", ", labels = paste0(format("fields", width=16), ":"), fill = TRUE)
  #assign(".nextMethod", selectMethod("show", "arc.datasetraster"))
  #callNextMethod()
  invisible(object)
})

setClass("arc.datasetrastermosaic_impl", contains = "arc.datasetrastermosaic")
setMethod("initialize", "arc.datasetrastermosaic_impl", def = function(.Object, base)
{
  stopifnot(!is.null(base))
  ptr <- .call_proxy("raster_mosaic_dataset.create_from", .Object, base)
  callNextMethod(.Object, base@path)
})

setIs("arc.dataset", "arc.datasetrastermosaic",
  test = function(from) from@dataset_type == "MosaicDataset",
  coerce = function(from) new("arc.datasetrastermosaic_impl", from),
  replace = function(obj, value) value)
