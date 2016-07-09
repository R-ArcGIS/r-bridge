._sp2shp <- function (type, src, bbox)
{
  nparts <- length(src)
  part_map <- c(1:nparts)
  npts <- 0
  for (i in 1:nparts)
  {
    n <- length(src[[i]]@coords)
    part_map[i] <- npts
    npts <- npts + n/2
  }
  cb <- 44 + nparts * 4 + npts * 2 * 8
  buf <- raw(cb)
  buf[1] <- as.raw(type)
  if (type == 3 || type == 5)
  {
    stopifnot(!is.null(bbox))
    buf[5:36] <- writeBin(bbox, buf, size = 8)
  }
  buf[37:40] <- writeBin(as.integer(nparts), buf, size = 4)
  buf[41:44] <- writeBin(as.integer(npts), buf, size = 4)
  pts_begin <- 45 + nparts * 4
  if (nparts > 1) 
    buf[45:(pts_begin - 1)] <- writeBin(as.integer(part_map), buf, size = 4)
  for (i in 1:nparts)
  {
    xy <- src[[i]]@coords
    xy <- as.vector(t(xy))
    n <- length(xy)
    end <- pts_begin + n * 8
    buf[pts_begin:(end - 1)] <- writeBin(xy, buf, size = 8)
    pts_begin <- end
  }
  return(buf)
}

#' Convert a sp SpatialDataFrame object to an arc.dataframe object
#'
#' Convert \code{sp} \code{SpatialPointsDataFrame}, \code{SpatialPolygonsDataFrame},
#' and \code{SpatialLinesDataFrame} objects to an ArcGIS-compatible \code{data.frame}.
#'
#'
#' @param sp.df \code{SpatialPointsDataFrame}, \code{SpatialPolygonsDataFrame},
#'  or \code{SpatialLinesDataFrame}
#' @seealso \code{\link{arc.data2sp}}
#' @keywords convert sp
#' @examples
#'
#' require(sp)
#' s1 <- Polygon(cbind(c(2,4,4,1,2),c(2,3,5,4,2)))
#' s2 <- Polygon(cbind(c(5,4,2,5),c(2,3,2,2)))
#' s3 <- Polygon(cbind(c(4,4,5,10,4),c(5,3,2,5,5)))
#' s4 <- Polygon(cbind(c(5,6,6,5,5),c(4,4,3,3,4)), hole = TRUE)
#' ss1 <- Polygons(list(s1), "s1")
#' ss2 <- Polygons(list(s2), "s2")
#' ss3 <- Polygons(list(s3, s4), "s3/4")
#' spp <- SpatialPolygons(list(ss1,ss2,ss3), 1:3)
#' sp.df <- SpatialPolygonsDataFrame(spp, data=data.frame(df=1:3),
#'                                match.ID = FALSE)
#' arc.df = arc.sp2data(sp.df)
#' arc.write(tempfile("sp_poly", fileext=".shp"), arc.df)
#' @export
arc.sp2data <- function (sp.df)
{
  if (!requireNamespace("sp", quietly = TRUE))
    stop("This function requires the sp package.")
  stopifnot(inherits(sp.df, "Spatial"))
  if (inherits(sp.df, "SpatialPoints"))
  {
    xy <- sp.df@coords
    shape <- lapply(seq_len(ncol(xy)), function(i) xy[,i])
  }
  else if (inherits(sp.df, "SpatialPolygons"))
  {
    bbox <- as.vector(sp.df@bbox)
    shape <- lapply(sp.df@polygons, function(it) ._sp2shp(5, it@Polygons, bbox))
  }
  else if (inherits(sp.df, "SpatialLines"))
  {
    bbox <- as.vector(sp.df@bbox)
    shape <- lapply(sp.df@lines, function(it) ._sp2shp(3, it@Lines, bbox))
  }
  else stop("unsupported SP type")

  df <- sp.df@data
  shapeinfo <- arc.shapeinfo(sp.df)
  attr(df, "shape") <- new("arc.shape", shape, shapeinfo = shapeinfo)
  class(df) <- append(class(df), "arc.data")
  return(df)
}
