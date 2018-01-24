#' @export
arc.write <- function(path, data, ...)
{
  if (inherits(path, "arc.container"))
    path <- path@path

  stopifnot(is.character(path))
  if (missing(data))
    data<-NULL

  args <- list(path=path, data=data, ...)
  overwrite <- args$overwrite
  if (is.null(overwrite))
    args$overwrite <- FALSE
  else
    stopifnot(is.logical(overwrite))

  if (inherits(data, c("Raster", "arc.raster", "SpatialPixels", "SpatialGrid")))
    do.call(.write_raster, args)
  else
    do.call(.write_feature, args)
}
