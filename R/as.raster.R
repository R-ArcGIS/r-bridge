setMethod("as.raster", "arc.raster", function(x, kind, ...)
{
  if (!requireNamespace("raster", quietly=TRUE))
    stop("This function requires the raster package.")
  if (missing(kind))
    kind = "ram"
  fn = if (dim(x)[3] == 1) .make_raster else .make_brick
  switch(kind,
    "ram" = fn(x, kind),
    "tmp" = fn(x, kind),
    #"drv" = .make_raster_drv(x),
    stop("invalid parameter")
  )
})

# implementation

#untested experimental add-in raster driver
.drv_readRows <- function(x, startrow, nrows, startcol, ncols, lyrs)
{
  #r <- get0(x@file@name, envir = .arc)
  if (missing(lyrs))
    lyrs <- x@data@band
  r <- x@file@.arc.raster
  #cat(".drv_readRows(", startrow, nrows, startcol, ncols, "[",lyrs, "])\n")

  vals <- r$pixel_block(startcol - 1, startrow - 1, nrows, ncols, lyrs)
  if (length(lyrs) == 1L)
    dim(vals) <- NULL
  #TODO mask by nodatavalue - test
  #vals[vals == x@file@nodatavalue] <- NA
  return (vals)
}

.drv_readCells <- function(x, cells, layers)
{
  r <- x@file@.arc.raster

  #cat(".drv_readCells(", length(cells), layers, ")\n")

  #str(cells)
  #str(layers)
  # exact copy of .readCellsGDAL()
  nl <- raster::nlayers(x)
  if (nl == 1) {
    if (inherits(x, 'RasterLayer')) {
      layers <- raster::bandnr(x)
    } else {
      layers <- 1
    }
  }
  laysel <- length(layers)

  colrow <- matrix(ncol = 2 + laysel, nrow = length(cells))
  colrow[, 1] <- raster::colFromCell(x, cells)
  colrow[, 2] <- raster::rowFromCell(x, cells)
  colrow[, 3] <- NA
  rows <- sort(unique(colrow[, 2]))
  #str(rows)

  nc <- x@ncols

  if (laysel == 1) {
    for (i in 1:length(rows))
    {
      offs <- c(rows[i] - 1, 0)
#      cat("pixel_block(ul_x=", offs[2], ", ul_y=", offs[1], ", nrow=", 1, ", ncol=",nc, ")\n")
      v <- r$pixel_block(ul_x = offs[2], ul_y = offs[1], nrow = 1, ncol = nc, bands = layers)
      thisrow <- colrow[colrow[, 2] == rows[i],, drop = FALSE]
      colrow[colrow[, 2] == rows[i], 3] <- v[thisrow[, 1]]
    }
  } else {
    for (i in 1:length(rows))
    {
      thisrow <- colrow[colrow[, 2] == rows[i],, drop = FALSE]
      if (nrow(thisrow) == 1) {
        offs <- c(rows[i] - 1, thisrow[, 1] - 1)
        v <- as.vector(r$pixel_block(ul_x = offs[2], ul_y = offs[1], nrow = 1, ncol = 1))
        colrow[colrow[, 2] == rows[i], 2 + (1:laysel)] <- v[layers]
      } else {
        offs <- c(rows[i] - 1, 0)
        v <- r$pixel_block(ul_x = offs[2], ul_y = offs[1], nrow = 1, ncol = nc)
        v <- do.call(cbind, lapply(1:nl, function(i) v[,, i]))
        colrow[colrow[, 2] == rows[i], 2 + (1:laysel)] <- v[thisrow[, 1], layers]
      }
    }
  }
  colnames(colrow)[2 + (1:laysel)] <- names(x)[layers]
  colrow[, 2 + (1:laysel)]
}

# experimantal external raster driver
#.make_raster_drv <- function(x)
#{
#  drv_name = "arcgisbinding-driver"
#  rx = if (dim(x)[3] == 1) .make_raster(x, "") else .make_brick(x, "")
#  if (is.null(.arc$drv.count))
#  {
#    drv <- list(
#      readRows = .drv_readRows,
#      readCells = .drv_readCells
#    )
#    raster::registerExternalDriver(drv_name, drv)
#    .arc$drv.count <- 0L
#    cat("register external raster driver:", drv_name, "\n")
#  }
#
#  .arc$drv.count <- .arc$drv.count + 1L
#
#  slot(rx@data, "fromdisk", F) <- TRUE
#  slot(rx@file, "driver", F) <- drv_name
#  slot(rx@file, "name", F) <- x$.info$path
#  #reg.finalizer(e, function(x) rm(x@file@name, envir = .arc))
#  slot(rx@file, ".arc.raster", F) <- x$copy()
#  return (rx)
#}

# copy pixelblock to tmp raster
.in_tmp <- function(rx, r)
{
  bs <- raster::blockSize(rx)
  tmp.file <- raster::rasterTmpFile()
  rx <- raster::writeStart(rx, tmp.file, overwrite=TRUE)

  #e <- new.env()
  #e$self <- rx
  #reg.finalizer(e, function(ev) { cat("unlink file:", ev$self@file@name, "\n"); unlink(ev$self@file@name) }, TRUE)

  for (i in 1:bs$n)
  {
    v <- r$pixel_block(ul_x = 0L, ul_y = bs$row[i] - 1L, nrow = bs$nrows[i])
    out <- raster::writeValues(rx, v, bs$row[i])
  }
  rx <- raster::writeStop(rx)
  return (rx)
}

.make_raster <- function(x, kind)
{
  extent <- x$extent
  rx <- raster::raster(nrow = x$nrow, ncol = x$ncol, crs = x$proj4string(),
        xmn = extent[1L], xmx = extent[3L], ymn = extent[2L], ymx = extent[4L])
  dtype <- if(x$pixel_type %in% c("U2", "U4"))
  {
    warning(paste0("as.raster() : up casting pixel type ", x$pixel_type, " to INT1U"), call.=FALSE)
    "INT1U"
  } else .arc$pixel_type2data_type[x$pixel_type]

  #print(dtype)
  raster::dataType(rx) <- dtype
  if (!is.na(x$nodata)) raster::NAvalue(rx) <- x$nodata

  if (x$has_colormap())
    raster::colortable(rx) <- x$colormap

  names(rx) <- names(x)

  if (kind == "ram")
  {
    v = x$pixel_block(bands = 1L)
    dim(v) <- NULL
    raster::setValues(rx, v)
  }
  else if (kind == "tmp")
    .in_tmp(rx, x)
  else
    return (rx)
}

.make_brick <- function(x, kind)
{
  d <- dim(x)
  bands = 1:d[3]
  ex <- x$extent
  extent <- raster::extent(ex[1], ex[3], ex[2], ex[4])
  #rx <- raster::brick(nrow = d[1], ncol = d[2], crs = x$proj4string(),
  #    nl = length(bands),
  #    xmn = extent[1], xmx = extent[3], ymn = extent[2], ymx = extent[4])
  rx <- raster::brick(extent, nrows=d[1], ncols=d[2], crs=x$proj4string(), nl=length(x$bands))
  names(rx) <- names(x$bands)
  dtype <- if(x$pixel_type %in% c("U2", "U4"))
  {
    warning(paste0("as.raster() : up casting pixel type ", x$pixel_type, " to INT1U"), call.=FALSE)
    "INT1U"
  } else .arc$pixel_type2data_type[x$pixel_type]
  raster::dataType(rx) <- dtype
  if (!is.na(x$nodata)) raster::NAvalue(rx) <- x$nodata

  switch(kind,
    "ram" = raster::setValues(rx, x$pixel_block(bands = bands)),
    "tmp" = .in_tmp(rx, x),
    rx)
}
