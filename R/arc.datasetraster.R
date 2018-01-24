
setMethod("initialize", "arc.datasetraster", def = function(.Object, path)
{
  .Object <- callNextMethod(.Object, path)
  .init_datasetraster <- function(x)
  {
    x@extent <- .call_proxy("dataset.extent", x)
    x@sr <- .call_proxy("dataset.sr", x)
    info <- x@.info
    #convinience
    pt <- info$bands[[1]]$pixel_type + 1
    x@pixel_type <- .pixel_type2str(pt)
    x@compression_type <- info$compression_type
    x@nrow <- info$bands[[1]]$nrow
    x@ncol <- info$bands[[1]]$ncol
    x@bands <- info$bands;
    x@.info$bands<-NULL
    x
  }
  .init_datasetraster(.Object)
})

setMethod("dim", "arc.datasetraster", def = function(x)
{
  nband <- length(x@bands)
  c(nrow = x@nrow, ncol = x@ncol, nband = length(x@bands))
})

#setMethod("arc.select", signature(object = "arc.datasetraster", fields = "missing", where_clause = "missing", selected = "missing", sr = "ANY"), def = arc.raster)

setMethod("show", "arc.datasetraster", function(object)
{
  callNextMethod(object)
  x <- c(format=object@.info$format,
    pixel_type=paste0(object@pixel_type, " (", .pixel_depth(object@pixel_type), "bit)"),
    compression_type=object@compression_type,
    nrow=object@nrow,
    ncol=object@ncol)

  na_val <- object@bands[[1]]$no_data_value
  if (!is.null(na_val))
    x <- c(x, nodata=na_val)
  x <- c(x, extent=.format_extent(object@extent), .format_sr(object@sr))
  if (!is.null(object@.info$raster_function))
    x <- c(x, raster_function = paste0(object@.info$raster_function[1], " (", object@.info$raster_function[2], ")"))

  cat(paste(paste0(format(names(x), width=16), ": ", x), collapse="\n"))

  .print_bands(object@bands)
  if (object@.info$format == "TIFF")
  {
    meta = object@.info[grepl("^TIFFTAG_", names(object@.info))]
    if (length(meta) > 0)
    {
      cat("tiff metadata  :\n ")
      cat(paste(names(meta),unlist(meta),"\n"))
    }
  }
  invisible(object)
})

setClass("arc.datasetraster_impl", contains = "arc.datasetraster")

setMethod("initialize", "arc.datasetraster_impl", def = function(.Object, base, path)
{
  stopifnot(!is.null(base))
  ptr <- .call_proxy("raster_dataset.create_from", .Object, base)
  stopifnot(!is.null(ptr))
  if (missing(path))
    path = base@path
  callNextMethod(.Object, path)
})

setIs("arc.dataset", "arc.datasetraster",
  test = function(from) from@dataset_type == "RasterDataset",
  coerce = function(from) new("arc.datasetraster_impl", from),
  replace = function(obj, value) value)
