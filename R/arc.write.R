# @export
arc.write <- function(path, data, ..., overwrite = FALSE)
{
  if (inherits(path, "arc.container"))
    path <- path@path

  stopifnot(is.character(path))
  stopifnot(is.logical(overwrite))

  if (missing(data))
    data<-NULL

  args <- list(path=path, data=data, ..., overwrite=overwrite)

  if (inherits(data, c("Raster", "arc.raster", "SpatialPixels", "SpatialGrid")))
    do.call(.write_raster, args)
  else
    do.call(.write_feature, args)
}
