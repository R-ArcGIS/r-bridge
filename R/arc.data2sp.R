# Convert an arc.dataframe or arc.raster object to an sp SpatialDataFrame object
#' @export
arc.data2sp <- function(x, ...)
{
  if (!requireNamespace("sp", quietly = TRUE))
    stop("This function requires the sp package.")

  if (inherits(x, "arc.raster"))
    return (.asSpatialGridDataFrame(x))

  stopifnot(inherits(x, "arc.data"))

  shp <- arc.shape(x)
  info <- arc.shapeinfo(shp)
  idx_na <- if (info$type == "Point")
    which(is.na(shp$x) | is.na(shp$y))
  else
    which(sapply(shp[[1]], is.null))

  if (length(idx_na) > 0)
  {
    message("removed missing geometry:", .one_line(paste(idx_na, collapse = " ")))
    x <- x[-idx_na, , drop = F]
    shp <- arc.shape(x)
  }

  crs <- list(...)$crs
  if (is.null(crs))
    spl <- arc.shape2sp(shape = shp, wkt = info$WKT)
  else
    spl <- arc.shape2sp(shape = shp, crs = crs)

  class(x) <- setdiff(class(x), "arc.data")
  attr(x, "shape") <- NULL

  if (ncol(x) > 0)
  {
    switch(info$type,
      "Point" = sp::SpatialPointsDataFrame(spl, data = x, match.ID = F),
      "Polygon" = sp::SpatialPolygonsDataFrame(spl, data = x, match.ID = F),
      "Polyline" = sp::SpatialLinesDataFrame(spl, data = x, match.ID = F),
      "Multipoint" = sp::SpatialMultiPointsDataFrame(spl, data = x, match.ID = F))
  }
  else
    spl
}

.asSpatialGrid <- function(from)
{
  grd <- sp::GridTopology(from$extent[1:2], from$cellsize, dim(from)[c(2,1)])
  sp::SpatialGrid(grd, proj4string=arc.fromWktToP4(from$sr$WKT))
}

.asSpatialGridDataFrame <- function(from)
{
  data <- as.data.frame(from$pixel_block())
  sp::SpatialGridDataFrame(.asSpatialGrid(from), data=data)
}

# Convert Esri shape to sp spatial geometry
#' @export
arc.shape2sp <- function(shape, ...)
{
  if (!requireNamespace("sp", quietly = TRUE))
    stop("This function requires the sp package.")
  info <- arc.shapeinfo(shape)

  args <- list(...)
  if (!is.null(args$crs))
    crs <- args$crs
  else if (!is.null(args$wkt))
    crs <- arc.fromWktToP4(args$wkt)
  else
    crs <- arc.fromWktToP4(info$WKT)

  p4 <- sp::CRS(crs)
  parts <- function(x) lapply(seq_along(x), function(i) .shp2sp(x[[i]], i, info$type))

  switch(info$type,
    "Point" = sp::SpatialPoints(cbind(shape$x, shape$y), proj4string = p4),
    "Polygon" = sp::SpatialPolygons(parts(shape[[1]]), proj4string = p4),
    "Polyline" = sp::SpatialLines(parts(shape[[1]]), proj4string = p4),
    "Multipoint" = sp::SpatialMultiPoints(parts(shape[[1]]), proj4string = p4))
}

.shp2sp <- function (buf, oid, type)
{
  if (is.null(buf))
    stop("empty geometry")
  xtype <- buf[1]
  if (xtype < 1 || xtype > 25)
    stop("unknown geometry type")

  nparts <- readBin(buf[37:40], integer(), 1, size = 4)
  npts <- readBin(buf[41:44], integer(), 1, size = 4)
  pts_begin <- 45 + nparts * 4
  if (type == "Multipoint")
  {
    npts <- nparts
    nparts <- 1
    pts_begin <- 41
  }
  pts_end <- pts_begin + npts * 2 * 8
  part_map <- function()
  {
    if (nparts == 1)
      list(1:npts)
    else
    {
      pmap <- c(readBin(buf[45:pts_begin], integer(), nparts, size = 4), npts)
      lapply(1:nparts, function(i,v) seq.int(v[i]+1, v[i+1]), v=pmap)
    }
  }
  pts <- matrix(readBin(buf[pts_begin:pts_end], numeric(), npts * 2, size = 8), npts, 2, byrow = TRUE)
  switch(type,
   "Polygon" = sp::Polygons(sapply(part_map(), function(i) sp::Polygon(pts[i,])),ID = oid),
   "Polyline" = sp::Lines(sapply(part_map(), function(i) sp::Line(pts[i,])), ID = oid),
   "Multipoint" = pts,
   stop("unsupported geometry type"))
}
