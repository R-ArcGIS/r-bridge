setMethod("dim", "arc.raster", def = function(x) x$dim())

setMethod("dim<-", "arc.raster", def = function(x, value) x$set_dim(value))

setMethod("names", "arc.raster", def = function(x) names(x$bands))

#@method arc.raster arc.datasetraster
setMethod("arc.raster", "arc.datasetraster", def = function(object, bands, ...) {
  if (missing(bands))
    bands <- 1L:length(object@bands)
  else
    stopifnot(min(bands) > 0 && max(bands) <= length(object@bands))

  r <- new("arc.raster", object, list(bands = as.integer(bands), ...))
  return(r)
})

#@method arc.raster NULL
setMethod("arc.raster", "NULL", def = function(object, path, dim, nrow, ncol, nband, extent, origin_x, origin_y, cellsize_x, cellsize_y, pixel_type, nodata, sr, ...){
  if (missing(nodata)) nodata <- as.numeric(NA)
  if (missing(pixel_type)) pixel_type <- "F64"
  if (missing(sr)) sr <- 0
  if (missing(dim))
  {
    stopifnot(!(missing(nrow) && missing(ncol) && missing(nband)))
    dim <- c(nrow, ncol, nband)
  }
  if (missing(extent))
  {
    stopifnot (missing(origin_x) == FALSE && missing(origin_y) == FALSE)
    if (missing(cellsize_x) && missing(cellsize_y))
    {
      cellsize_x <- 1
      cellsize_y <- 1
    }
    else if (missing(cellsize_y))
      cellsize_y <- cellsize_x
    else
      cellsize_x <- cellsize_y
  }
  else
  {
    extent <- .prepare_raster_prop_value(NULL, "extent", extent)
    origin_x <- extent[1]
    cellsize_x <- (extent[3] - origin_x)/dim[2]
    origin_y <- extent[2]
    cellsize_y <- (extent[4] - origin_y)/dim[1]
  }

  args = list( path = path,
               origin_x = origin_x,
               origin_y = origin_y,
               cellsize_x = cellsize_x,
               cellsize_y = cellsize_y,
               dim = as.integer(dim),
               sr = sr,
               pixel_type = pixel_type,
               nodata_value = nodata,
               ...
             )
  r <- new("arc.raster", NULL, args)
  return(r)
})

getRefClass("arc.raster")$methods(
  # construct
  initialize = function(source, args)
  {
    #str(args)
    args.names <- names(args)
    args = lapply(args.names, function(x) .prepare_raster_prop_value(NULL, x, args[[x]]))
    names(args) <- args.names

    nd <- args[["nodata"]]
    if(is.null(nd))
      nd <- args[["nodata_value"]]
    if(!is.null(nd) && !is.numeric(nd))
      stop(paste('nodata is not numeric:', typeof(nd)))
    #str(args)

    ptr <- .call_proxy("raster.create", .self, source, as.pairlist(args))
    stopifnot(!is.null(ptr))
    info <- .call_proxy("raster.rasterinfo", .self)
    if ("path" %in% args.names)
      info$path = args$path
    else
      info$path <- if (inherits(source, "arc.dataset")) source@path else source$.info$path
    .self$.info <- info
    .self$sr <- .call_proxy("raster.sr", .self)
    .self
  },
  copy = function(shallow = FALSE)
  {
    def <- .refClassDef
    value <- new(def, .self, NULL)
    return (value)
  },
  show = function()
  {
    x <- c(type="Raster",
            pixel_type = paste0(pixel_type, " (", pixel_depth, "bit)"),
            nrow=nrow,
            ncol=ncol)

    if (.info$show_resample_type)
      x<-c(x, resample_type= resample_type)

    x <- c(x, cellsize = paste0(cellsize, collapse=", "),
            nodata = nodata,
            extent = .format_extent(extent))

    if (!is.null(.info$raster_function))
      x <- c(x, raster_function = paste0(.info$raster_function[1], " (", .info$raster_function[2], ")"))
    if (!is.null(.info$mosaic))
      x <- c(x, mosaic = paste(paste0(names(.info$mosaic), "=",.info$mosaic),collapse=", "))

    x <- c(x, .format_sr(sr))

    if (has_colormap())
      x <- c(x, colormap = .one_line(colormap))
    #.print_bands(bands)
    cat(paste(paste0(format(names(x), width=16), ": ", x), collapse="\n"), "\n")
    .print_band_names(bands)
    invisible()
  },
  dim = function() c(nrow = .info$nrow, ncol = .info$ncol, nband = length(.info$bands)),
  set_dim = function(value)
  {
    if (length(value) == 1) value <- c(value, ncol)
    value <- as.integer(pmax(round(value[1:2]), c(1, 1)))
    stopifnot(.call_proxy("raster.update", .self, pairlist(nrow = value[1], ncol = value[2])))
    .self$.info <- .update_info(.self)
    invisible(.self)
  },
  pixel_block = function(ul_x, ul_y, nrow, ncol, bands)
  {
    .pixel_block(.self, bands, ul_x, ul_y, nrow, ncol)
  },
  proj4string = function() arc.fromWktToP4(sr$WKT),
  has_colormap = function() !is.na(colormap[1]),
  save_as = function(path, opt, overwrite)
  {
    if (missing(opt))
      opt <- NULL #"build-stats"
    if (missing(overwrite))
      overwrite <- FALSE

    .call_proxy("raster.save_as", .self, path, overwrite)
    commit(opt)
  },
  write_pixel_block = function(values, ul_x, ul_y, ncol, nrow)
  {
    if (missing(ul_x))
      ul_x <- 0L
    if (missing(ul_y))
      ul_y <- 0L

    nband <- length(.info$bands)
    nc <- if (missing(ncol)) (length(values)/(nrow * nband)) else ncol

    stopifnot(length(values) == nc * nrow * nband)
    .call_proxy("raster.write_pixelblock", .self, values, as.integer(c(ul_y, ul_x, nrow, nc)))
    invisible(TRUE)
  },
  commit = function(opt)
  {
    if (missing(opt))
      opt <- "build-stats"
    for (it in opt) .call_proxy("raster.commit", .self, it)
    gc()
    invisible(new("arc.datasetraster_impl", .self, .self$.info$path))
  },
  attribute_table = function()
  {
    ret_list <- .call_proxy("raster.attribute_table", .self)
    if (is.null(ret_list))
      return (ret_list)
    return (as.data.frame(ret_list, row.names = NULL, stringsAsFactors = FALSE))
  }
)

#
.pixel_depth <- function(pt) .arc$pixel_type[pt]

.update_info <- function(x)
{
  info <- .call_proxy("raster.rasterinfo", x)
  stopifnot(!is.null(info))
  return(info)
}

.prepare_raster_prop_value <- function(x, name, value)
{
  to_extent <- function(ex, val)
  {
    if (inherits(val, "Extent"))
      return(val[c(1L,3L,2L,4L)])

    stopifnot(is.numeric(val))
    n <- min(4L, length(val))
    val <- val[1L:n]
    na <- names(val)
    if (!is.null(na) && all(na %in% names(ex)))
      ex[na] <- val
    else
      ex[1L:n] <- val
    ex
  }
  to_resample_type <- function(val)
  {
    idx <- if (is.numeric(val))
      ifelse (val[1L] > 0 && val[1L] < length(.arc$resample_types), val[1L], 0L)
    else
      which(names(.arc$resample_types) == as.character(val))
    if (length(idx) > 0)
      return (as.integer(idx[1L] - 1L))
    stop(paste0("supported resample_types:\n", paste0(names(.arc$resample_types), collapse="\n")))
  }
  to_colormap <- function(val)
  {
    rgb <- function(s) as.raw(c(strtoi(substr(s[1],2,3),base=16),strtoi(substr(s[1],4,5),base=16), strtoi(substr(s[1],6,7),base=16)))
    as.raw(sapply(val, rgb))
  }

  switch(name,
      "nrow" = max(1L, as.integer(value)[1]),
      "ncol" = max(1L, as.integer(value)[1]),
      "extent" = to_extent(ifelse(is.null(x),value, x$extent), value),
      "pixel_type" = .to_pixeltype(value),
      "resample_type" = to_resample_type(value),
      "colormap" = to_colormap(value),
      value
    )
}
.raster_prop <- function(x, name, value)
{
  info <- x$.info
  if (missing(value))
  {
    switch(name,
      "pixel_type" = .pixel_type2str(info$pixel_type + 1L),
      "resample_type" = .resample_types_names()[info$resample_type + 1L],
      "pixel_depth" = .pixel_depth(info$pixel_type + 1L),
      "colormap" = if (!is.null(info$colormap)) info$colormap else as.character(NA),
      "nodata" = if (!is.null(info$bands[[1]]$nodata)) info$bands[[1]]$nodata else ifelse(info$pixel_type < 9L, as.integer(NA), as.double(NA)),
      {
        v <- info[name]
        if (is.null(v))
          stop("unknown properties")
        v[[1L]]
      }
    )
  }
  else
  {
    if (!(name %in% c("nrow", "ncol", "extent", "pixel_type", "resample_type", "colormap")))
      stop(gettextf("field '%s' is not editable", name))
    v <- .prepare_raster_prop_value(x, name, value)
    p <- pairlist(v)
    names(p)<-name
    stopifnot(.call_proxy("raster.update", x, p))
    x$.info <- .update_info(x)
    invisible(x)
  }
}

.pixel_type2str <- function(pt)
{
  if (pt < 0L || pt >14L) return ("unknown")
  names(.arc$pixel_type)[pt]
}

.to_pixeltype <- function(val)
{
  idx <- if (is.numeric(val))
    ifelse (val[1L] > 0 && val[1L] < 12L, val[1L], NULL)
  else
    which(names(.arc$pixel_type) == as.character(val))
  if (length(idx) > 0)
    return (as.integer(idx[1L] - 1L))
  stop(paste0("supported pixel types:\n", paste0(names(.arc$pixel_type)[1:12], collapse="\n")))
}

.print_bands <- function(bands)
{
  fld <- which(names(bands[[1]]) %in% c("ncol", "nrow", "min", "max", "mean", "stddev", "nodata"))
  df <- data.frame(t(simplify2array(bands)))

  cat("\n", format("bands", width=16),": ", length(bands), "\n", sep="")
  print(df[fld])
}

.print_band_names <- function(bands)
{
  nbands <- length(bands)
  labels <- if (nbands > 1) paste0("bands[1:", nbands, "]") else "band"
  cat(names(bands), sep = ", ", labels = paste0(format(labels, width=16), ":"), fill = TRUE)
}

.pixel_block = function(x, bands, ul_x, ul_y, nrow, ncol)
{
  if (missing(bands)) bands <- 1L:dim(x)[3]
  if (missing(ul_x)) ul_x <- 0L
  if (missing(ul_y)) ul_y <- 0L
  if (missing(nrow)) nrow <- x$nrow - ul_y
  if (missing(ncol)) ncol <- x$ncol - ul_x

  no_data_val <- ifelse(x$.info$pixel_type < 9L, as.integer(NA) , as.double(NA))
  nl <- dim(x)[3]
  bands <- as.integer(bands)
  if (min(bands) < 1L && max(bands) > nl)
    stop(paste0("band is out of range[1;", nl, "]"))
  #allocate all pixels once
  #gc()
  px <- matrix(no_data_val, nrow = nrow * ncol, ncol = length(bands))
  .call_proxy("raster.fill_pixelblock", x, px, as.integer(c(ul_y, ul_x, nrow, ncol)), bands)
  #class(px) <- append(class(px), "arc.pixelblock")
  colnames(px) <- names(x$bands)[bands]
  return (px)
}

.save_Raster <- function(path, rx, overwrite, opt)
{
  d <- as.integer(dim(rx))
  px_type <- names(.arc$pixel_type2data_type)[which(.arc$pixel_type2data_type == raster::dataType(rx))]

  r <- arc.raster(NULL,
    path = path,
    origin_x = raster::xmin(rx),
    origin_y = raster::ymin(rx),
    cellsize_x = raster::xres(rx),
    cellsize_y = raster::yres(rx),
    dim = d,
    #extent = extent(rx),
    sr = arc.fromP4ToWkt(raster::crs(rx)),
    pixel_type = px_type,
    nodata_value = raster::NAvalue(rx),
    overwrite = overwrite
  )
  #r <- new("arc.raster", NULL, args)
  ncol <- d[2]
  bs <- raster::blockSize(rx)
  for (i in 1:bs$n)
  {
    v <- raster::getValues(rx, row = bs$row[i], nrows = bs$nrows[i])
    if (!r$write_pixel_block(values = v, ul_x = 0L, ul_y = bs$row[i] - 1, ncol = ncol, nrow=bs$nrows[i]))
      break
  }
  ct <- raster::colortable(rx)
  if (length(ct) > 0L)
    r$colormap <- ct
  d <- r$commit(opt)
  .discard(r)
  return (invisible(d))
}

.save_SpatialPixels <- function(path, spdf, pt, overwrite, opt)
{
  is_mask <- !is(spdf, "SpatialPixelsDataFrame")
  px_type <- if (is_mask) "U1" else "F32"
  if (!missing(pt))
    px_type = pt

  ddata <- if (is_mask) 1 else spdf@data #as mask raster
  .save_sp(path, spdf, ddata, px_type, overwrite, opt)
}

.save_SpatialGrid <- function(path, sgdf, pt, overwrite, opt)
{
  .save_sp(path, sgdf, sgdf@data, pt, overwrite, opt)
}

.save_sp <- function(path, x, ddata, pt, overwrite, opt)
{
  if (missing(pt))
    pt = "F32"
  grid <- sp::getGridTopology(x)
  nrow <- grid@cells.dim[2]
  ncol <- grid@cells.dim[1]
  extent <- as.double(sp::bbox(x))
  ddata <- ddata[sapply(ddata, function(c) is.numeric(c) || is.factor(c))]

  nband <- ncol(ddata)
  if (is.null(nband))
    nband <- 1L
  r <- arc.raster(NULL,
    path = path,
    extent = extent,
    dim = c(nrow, ncol, nband),
    sr = arc.fromP4ToWkt(sp::proj4string(x)),
    pixel_type = pt,
    overwrite = overwrite)
  if (inherits(x, "SpatialPixels"))
  {
    px <- matrix(NA, nrow = nrow*ncol, ncol = nband)
    px[x@grid.index,] = unlist(lapply(ddata, function(x) if(is.factor(x)) as.double(levels(x)[x]) else as.double(x)), use.names=FALSE)
    r$write_pixel_block(values = px, ncol = ncol, nrow = nrow)
  }
  else
  {
    r$write_pixel_block(values = as.matrix(ddata), ncol = ncol, nrow = nrow)
  }
  d <- r$commit(opt)
  .discard(r)
  return (invisible(d))
}

.write_raster <- function(path, data, ..., overwrite)
{
  stopifnot(!missing(data))
  stopifnot(!is.null(data))

  if (inherits(data, "arc.raster"))
    return (data$save_as(path, ..., overwrite=overwrite))
  if (inherits(data, "Raster"))
    return (.save_Raster(path, data, ..., overwrite=overwrite))
  if (inherits(data, "SpatialPixels"))
    return (.save_SpatialPixels(path, data, ..., overwrite=overwrite))
  if (inherits(data, "SpatialGridDataFrame"))
    return (.save_SpatialGrid(path, data, ..., overwrite=overwrite))

  stop("unsupported raster object")
}
