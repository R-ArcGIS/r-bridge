
#arc.data2sf.default <- function(x) stop()
# Convert an arc.dataframe object to an sf Simple Feature object
#' @export
arc.data2sf <- function (x, ...)
{
  stopifnot(inherits(x, "arc.data"))

  if (!requireNamespace("sf", quietly = TRUE))
    stop("This function requires the sf package.")
  df <- if (length(x) > 0L) as.data.frame(x) else data.frame()
  #class(df) <- setdiff(class(df), "arc.data")
  #attr(df, "shape") <- NULL
  shape <- arc.shape(x)
  stopifnot(!is.null(shape))
  info <- arc.shapeinfo(shape)
  stopifnot(!is.null(info))

  crs <- list(...)$crs
  if (is.null(crs))
    crs <- arc.fromWktToP4(info$WKT)

  if (info$type == "-Point") #alternative
  {
    d2<-data.frame(df, "Shape.."=shape)
    coords<-paste0("Shape...", names(shape))
    #dim<-toupper(paste(names(shape), collapse=""))
    dim<-.shapeinfo_dim(info)
    sf::st_as_sf(d2,coords=coords, dim=dim, crs=crs)
  }
  sf::st_sf(df, geom=arc.shape2sf(shape, crs=crs))
}

# Convert Esri shape to sfc simple feature geometry
#' @export
arc.shape2sf <- function (shape, ...)
{
  stopifnot(inherits(shape, "arc.shape"))
  info <- arc.shapeinfo(shape)

  crs <- list(...)$crs
  if (is.null(crs))
    crs <- arc.fromWktToP4(info$WKT)

  t <- .shapeinfo_dim(info)

  sfgs <- if (info$type == "Point")
  {
    xy <- switch(t,
      "XY" = cbind(shape$x, shape$y),
      "XYZ" = cbind(shape$x, shape$y, shape$z),
      "XYM" = cbind(shape$x, shape$y, shape$m),
      "XYZM" = cbind(shape$x, shape$y, shape$z, shape$m))
    #very slow
    #lapply(1:nrow(xy), function(i) sf::st_point(xy[i,], dim=t))

    #this round trip 1.5x faster
    #x <- sf::st_sfc(sf::st_multipoint(xy, dim=t), crs=arc.fromWktToP4(wkt))
    #(sf::st_cast(x, to="POINT"))

    #fastest
    lapply(1:nrow(xy), function(i) structure(xy[i,], class=c(t, "POINT", "sfg")) )
  }
  else
    lapply(shape[[1]], function(sh) .shp2sfg(sh, info$type, t))
  return(sf::st_sfc(sfgs, crs = crs))
}

#create 'sfg' object from Esri shape buffer
.shp2sfg <- function(buf, type, dim)
{
  if (is.null(buf))
  {
    m<-matrix(0, nrow=0, ncol=nchar(dim))
    return(switch(type,
       "Polygon" = sf::st_polygon(m, dim=dim),
       "Polyline" = sf::st_linestring(m, dim=dim),
       "Multipoint" = sf::st_multipoint(m, dim=dim),
       NULL))
  }
  stopifnot(is.raw(buf))

  xtype <- buf[1]
  stopifnot(xtype > 1L)
  if (xtype == 54L)
    stop("unsupported geometry type: MultiPatch")
  has_curve = (buf[4] & as.raw(0x20)) == 0x20
  stopifnot(has_curve == FALSE)
  #has_Z = (buf[4] & as.raw(0x40)) == 0x40
  #has_M = (buf[4] & as.raw(0x80)) == 0x80


  nparts <- readBin(buf[37L:40L], integer(), 1L, size = 4L)
  npts <- readBin(buf[41L:44L], integer(), 1L, size = 4L)
  pts_begin <- 45L + nparts * 4L
  if (type == "Multipoint")
  {
    npts <- nparts
    nparts <- 1L
    pts_begin <- 41L
  }
  pts_end <- pts_begin + npts * 2L * 8L
  part_map <- function()
  {
    if (nparts == 1L)
      list(1L:npts)
    else
    {
      pmap <- c(readBin(buf[45L:pts_begin], integer(), nparts, size = 4L), npts)
      lapply(1:nparts, function(i,v) seq.int(v[i]+1L, v[i+1L]), v=pmap)
    }
  }
  pts <- matrix(readBin(buf[pts_begin:pts_end], numeric(), npts * 2L, size = 8L), nrow=npts, ncol=2L, byrow = TRUE)

  read_next <- function(begin)
  {
    end <- begin + 2L * 8L
    #range <- readBin(buf[begin:end], numeric(), 2L, size = 8L)
    begin <- end
    end <- begin + npts * 8L
    readBin(buf[begin:end], numeric(), npts, size = 8L)
  }
  if (dim == "XYZ" || dim == "XYZM")
  {
    z <- read_next(pts_end)
    pts <- cbind(pts, z)
    pts_end <- pts_end + npts * 8L + 2L * 8L
  }
  if (dim == "XYM" || dim == "XYZM")
  {
    m <- read_next(pts_end)
    pts <- cbind(pts, m)
  }

  switch(type,
    "Polygon" = if(nparts > 1L) sf::st_multipolygon(list(lapply(part_map(), function(i) pts[i,])), dim=dim) else sf::st_polygon(lapply(part_map(), function(i) pts[i,]), dim=dim),
    "Polyline" = if(nparts > 1L) sf::st_multilinestring(lapply(part_map(), function(i) pts[i,]), dim=dim) else sf::st_linestring(pts, dim=dim),
    "Multipoint" = sf::st_multipoint(pts, dim=dim),
    stop("unsupported geometry type"))
}

.coords_from_sfc <- function (sfc)
{
  if (!requireNamespace("sf", quietly = TRUE))
    stop("This function requires the sf package.")
  stopifnot(inherits(sfc, "sfc"))
  ctype <- class(sfc[1])[1]
  shape <- switch(ctype,
    "sfc_POINT" = sf::st_coordinates(sfc),
    "sfc_MULTIPOINT" = list(shape_buffer=lapply(sfc, function(it) .sfg2shp(8L, it))),
    "sfc_POLYGON" = list(shape_buffer=lapply(sfc, function(it) .sfg2shp(5L, it))),
    "sfc_MULTIPOLYGON" = list(shape_buffer=lapply(sfc, function(it) .sfg2shp(5L, it))),
    "sfc_LINESTRING" = list(shape_buffer=lapply(sfc, function(it) .sfg2shp(3L, it))),
    "sfc_MULTILINESTRING" = list(shape_buffer=lapply(sfc, function(it) .sfg2shp(3L, it))),
    stop(paste0("unsupported type: ", ctype))
   )
}

#create Esri shape buffer from 'sfg' object
.sfg2shp <- function(type, sfg)
{
  stopifnot(type == 3L || type == 5L || type == 8L)
  stopifnot(inherits(sfg,"sfg"))

  if (length(sfg) == 0L)
    return (NULL) #empty geometry

  xyzm <- sf::st_coordinates(sfg)
  stopifnot(is.matrix(xyzm))
  npts <- nrow(xyzm)
  if (npts == 0L) #empty geometry
    return (NULL)

  dim <- class(sfg)[1]

  rwtype <- if (type == 8L) switch(dim, "XYZ"=20L, "XYM"=28L, "XYZM"=18L, type)
       else if (type == 3L) switch(dim, "XYZ"=10L, "XYM"=23L, "XYZM"=13L, type)
       else if (type == 5L) switch(dim, "XYZ"=19L, "XYM"=25L, "XYZM"=15L, type)

  con <- raw()
  #0 type
  ret <- list(as.raw(c(rwtype, 0L, 0L, 0L)))
  #4 box
  bbox <- as.vector(sf::st_bbox(sfg))
  ret <- list(ret, writeBin(bbox, con, size = 8L))

  if (type == 8L) #multipoint
  {
    #36 npts
    ret <- list(ret, writeBin(as.integer(npts), con, size = 4L))
    #40 XY
  }
  else #polygon, polyline
  {
    part_map <- switch(class(sfg)[2],
      "MULTIPOLYGON" = c(0L, sapply(unlist(sfg, FALSE, FALSE), nrow)),
      "LINESTRING" = c(0L, nrow(sfg)),
      c(0L, sapply(sfg, nrow)))
    nparts <- length(part_map) - 1L
    part_map <- cumsum(part_map)

    #36 nparts
    ret <- list(ret, writeBin(nparts, con, size = 4L))
    #40 npts
    ret <- list(ret, writeBin(npts, con, size = 4L))
    #44 part_map
    ret <- list(ret, writeBin(part_map[-(nparts+1L)], con, size = 4L))
    #(44 + nparts*4) XY
  }
  #XY
  ret <- list(ret, writeBin(as.vector(t(xyzm[,1L:2L])), con, size = 8L))

  dim_zm <- nchar(dim) - 2L
  if (dim_zm > 0L) #write ZMs
  {
    write_next <- function(data) writeBin(c(min(data), max(data), data), raw(), size = 8L)
    #Z or M
    ret <- list(ret, write_next(xyzm[,3L]))
    if (dim_zm == 2L) #M
      ret <- list(ret, write_next(xyzm[,4L]))
  }
  return (unlist(ret))
}

.get_shape_info_from_sf <- function (sf)
{
  sfc <- sf::st_geometry(sf)
  stopifnot(inherits(sfc, "sfc"))

  type <- switch(class(sfc[1])[1],
    "sfc_POINT" = "Point",
    "sfc_POLYGON" = "Polygon",
    "sfc_MULTIPOLYGON" = "Polygon",
    "sfc_LINESTRING" = "Polyline",
    "sfc_MULTILINESTRING" = "Polyline",
    "sfc_MULTIPOINT" = "Multipoint",
    stop("unsupported sf type")
  )
  dim <- class(sfc[[1]])[1]
  hasM <- (dim %in% c("XYM", "XYZM"))
  hasZ <- (dim %in% c("XYZ", "XYZM"))
  shapeinfo <- list(type=type, hasZ = hasZ, hasM = hasM)
  sr <- sf::st_crs(sfc)
  wkt <- arc.fromP4ToWkt(sr$proj4string)
  if (!is.null(wkt))
    shapeinfo["WKT"] <- wkt
  #class(shapeinfo) <- append(class(shapeinfo), "arc.shapeinfo")
  class(shapeinfo) <- append("arc.shapeinfo", class(shapeinfo))
  #shapeinfo <- structure(shapeinfo, class="arc.shapeinfo")
  return (shapeinfo)
}
