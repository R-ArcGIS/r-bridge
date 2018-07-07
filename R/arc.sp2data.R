#create Esri shape buffer from 'sp' object
.sp2shp <- function (type, src, bbox)
{
  stopifnot(type == 3L || type == 5L || type == 8L)
  stopifnot(!is.null(bbox))

  con <- raw()
  #0 type
  ret <- list(as.raw(c(type, 0L, 0L, 0L)))
  #4 box
  ret <- list(ret, writeBin(bbox, con, size = 8L))

  if (type == 8)
  {
    stopifnot(is.matrix(src))
    stopifnot(ncol(src) == 2) #Z,M?
    npts <- nrow(src)
    #36 npts
    ret <- list(ret, writeBin(npts, con, size = 4L))
    #40 XY
    ret <- list(ret, writeBin(as.vector(t(src)), con, size = 8L))
    return (unlist(ret))
  }

  nparts <- length(src)
  npts <- 0L
  part_map <- c(0L, sapply(src, function(x){ npts<<-npts+nrow(sp::coordinates(x));npts}))

  #36 nparts
  ret <- list(ret, writeBin(nparts, con, size = 4L))
  #40 npts
  ret <- list(ret, writeBin(npts, con, size = 4L))
  #44 part_map
  ret <- list(ret, writeBin(part_map[-(nparts+1L)], con, size = 4L))
  #(44 + nparts*4) XY
  ret <- list(ret, sapply(src, function(x)
  {
    xy <- as.vector(t(sp::coordinates(x)))
    writeBin(xy, con, size = 8L)
  }))
  return (unlist(ret))
}

.sp2shape <- function (spl)
{
  shape <- if (inherits(spl, "SpatialPoints"))
  {
    xy <- sp::coordinates(spl)
    list(x=xy[,1], y=xy[,2])
  }
  else if (inherits(spl, "SpatialPolygons"))
    list(shape_buffer=lapply(spl@polygons, function(it) .sp2shp(5, it@Polygons, bbox = as.vector(sp::bbox(spl)))))
  else if (inherits(spl, "SpatialLines"))
    list(shape_buffer=lapply(spl@lines, function(it) .sp2shp(3, it@Lines, bbox = as.vector(sp::bbox(spl)))))
  else if (inherits(spl, "SpatialMultiPoints"))
    list(shape_buffer=lapply(spl@coords, function(it) .sp2shp(8, it, bbox = as.vector(sp::bbox(spl)))))
  else stop("unsupported SP type")

  return (new("arc.shape", shape, shapeinfo = .get_shape_info_from_sp(spl)))
}

#' @export
arc.sp2data <- function (sp.df)
{
  .Deprecated("", msg="arc.sp2data() will be removed. Use sp object directly in arc.*()", package = "arcgisbinding")
  if (!requireNamespace("sp", quietly = TRUE))
    stop("This function requires the sp package.")
  .sp2data(sp.df)
}

.sp2data <- function (sp.df)
{
  stopifnot(inherits(sp.df, "Spatial"))
  df <- sp.df@data
  attr(df, "shape") <- .sp2shape(sp::geometry(sp.df))
  class(df) <- c(class(df), "arc.data")
  return(df)
}

.get_shape_info_from_sp <- function (sp.df)
{
  shapeinfo <- NULL
  if (inherits(sp.df, "SpatialPoints"))
    shapeinfo <- list(type = "Point")
  else if (inherits(sp.df, "SpatialPolygons"))
    shapeinfo <- list(type = "Polygon")
  else if (inherits(sp.df, "SpatialLines"))
    shapeinfo <- list(type = "Polyline")
  else if (inherits(sp.df, "SpatialMultiPoints"))
    shapeinfo <- list(type = "Multipoint")
  else
    stop("unsupported sp object")
  wkt <- arc.fromP4ToWkt(sp.df@proj4string@projargs)
  if (!is.null(wkt))
    shapeinfo["WKT"] <- wkt
  class(shapeinfo) <- c(class(shapeinfo), "arc.shapeinfo")
  #shapeinfo <- structure(shapeinfo, class="arc.shapeinfo")
  return (shapeinfo)
}
