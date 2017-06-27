#' Convert an arc.dataframe object to an sp SpatialDataFrame object
#'
#' Convert an ArcGIS \code{data.frame} to the equivalent \code{sp} data frame
#' type. The output types that can be generated: \code{SpatialPointsDataFrame},
#' \code{SpatialLinesDataFrame}, or \code{SpatialPolygonsDataFrame}.
#'
#' @param x \code{data.frame} result of \code{\link{arc.select}}
#' @examples
#'
#' require(sp)
#' d <- arc.open(system.file("extdata", "ca_ozone_pts.shp",
#'                           package="arcgisbinding"))
#' df <- arc.select(d, 'ozone')
#' sp.df <- arc.data2sp(df)
#' \dontrun{spplot(sp.df)}
#' @export
arc.data2sp <- function (x) UseMethod("arc.data2sp")
#arc.data2sp.default <- function (x)
#{
#  print("incorrect type")
#  return(NULL)
#}
#' @export
arc.data2sp.arc.data <- function (x)
{
  if (!requireNamespace("sp", quietly = TRUE))
    stop("This function requires the sp package.")
  shape <- arc.shape(x)
  df <- NULL
  if (length(x) > 0L)
    df <- as.data.frame(x)
  shape_info <- shape@shapeinfo
  if (shape_info$type == "Point")
    shape <- na.exclude(cbind(shape$x, shape$y))
  else
    shape <- na.exclude(shape)
  idx_na <- attr(shape, "na.action")

  if (!is.null(idx_na))
  {
    message("removed missing geometry:", paste(idx_na, collapse = " "))
    if (!is.null(df))
      df <- df[-idx_na, , drop = F]
  }
  spl <- arc.shape2sp(shape, shape_info$WKT)
  if (!is.null(df))
  {
    if (class(spl) == "SpatialPoints")
      spl <- sp::SpatialPointsDataFrame(spl, data = df, match.ID = F)
    if (inherits(spl, "SpatialPolygons"))
      spl <- sp::SpatialPolygonsDataFrame(spl, data = df, match.ID = F)
    else if (inherits(spl, "SpatialLines"))
      spl <- sp::SpatialLinesDataFrame(spl, data = df, match.ID = F)
  }
  return(spl)
}

._shp <- function (buf, oid)
{
  type <- buf[1]
  stopifnot(type == 5 || type == 3)

  nparts <- readBin(buf[37:40], integer(), 1, size = 4)
  npts <- readBin(buf[41:44], integer(), 1, size = 4)
  pts_begin <- 45 + nparts * 4
  pts_end <- pts_begin + npts * 2 * 8
  if (nparts == 1)
  {
    part_map <- list(1:npts)
  }
  else
  {
    part_map <- c(readBin(buf[45:pts_begin], integer(), nparts, size = 4), npts)
    part_map <- lapply(1:nparts, function(i,v) seq.int(v[i]+1, v[i+1]), v=part_map)
  }
  pts <- matrix(readBin(buf[pts_begin:pts_end],
                numeric(), npts * 2, size = 8), npts, 2, byrow = TRUE)
  if (type == 5)
    return(sp::Polygons(sapply(part_map,
                        function(i) sp::Polygon(pts[i,])),ID = oid))
  else if (type == 3)
    return(sp::Lines(sapply(part_map,
                            function(i) sp::Line(pts[i,])), ID = oid))
  warning("unsupported geometry type")
  return(pts)
}

#' Convert Esri shape to sp spatial geometry
#'
#' Convert \code{\link{arc.shape-class}} to \code{sp} spatial geometry:
#' \code{SpatialPoints}, \code{SpatialLines}, or \code{SpatialPolygons}.
#'
#'
#' @param shape \code{\link{arc.shape-class}}
#' @param wkt WKT spatial reference
#' @seealso \code{\link{arc.shape}}
#' @examples
#'
#' require(sp)
#' d <- arc.open(system.file("extdata", "ca_ozone_pts.shp",
#'                           package="arcgisbinding"))
#' df <- arc.select(d, 'ozone')
#' sp.df <- arc.shape2sp(arc.shape(df))
#' \dontrun{plot(sp.df)}
#' @export
arc.shape2sp <- function (shape, wkt = arc.shapeinfo(shape)$WKT)
{
  if (!requireNamespace("sp", quietly=TRUE))
    stop("This function requires the sp package.")
  p4 <- arc.fromWktToP4(wkt)
  if (!is.raw(shape[[1]]))
    return(sp::SpatialPoints(shape, proj4string = sp::CRS(p4)))
  pl <- apply(cbind(seq(length(shape)), shape), 1, function(i) ._shp(i[[2]], i[[1]]))
  if (class(pl[[1]])[[1]] == "Polygons")
    sl <- sp::SpatialPolygons(pl, proj4string = sp::CRS(p4))
  else if (class(pl[[1]])[[1]] == "Lines")
    sl <- sp::SpatialLines(pl, proj4string = sp::CRS(p4))
  return(sl)
}
