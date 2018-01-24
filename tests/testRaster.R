# raster tests

if (!isNamespaceLoaded("arcgisbinding"))
{
  library("arcgisbinding")
  arc.check_product()
}
.arc <- asNamespace("arcgisbinding")$.arc

fgdb_path = tempfile("new_databse",fileext=".gdb")

#pixel types match
check_1 <- function()
{
  rasterDT <- c('LOG1S', 'INT1S', 'INT1U', 'INT2S', 'INT2U', 'INT4S', 'INT4U', 'FLT4S', 'FLT8S')
  pixelT   <- c( 'U1',    'S8',   'U8',     'S16' ,  'U16',   'S32' ,  'U32',   'F32',   'F64')
  for(i in seq_along(rasterDT))
  {
    px_type <- names(.arc$pixel_type2data_type)[which(.arc$pixel_type2data_type == rasterDT[i])]
    stopifnot(px_type == pixelT[i])
  }
}

# reading and compare "logo.jpg", save to different formats
check_2 <- function()
{
  fn <- system.file("pictures", "logo.jpg", package="rgdal")
  r <- arc.raster(arc.open(fn))
  rx <- raster::raster(fn)
  stopifnot(dim(rx) == dim(r))
  stopifnot(any(raster::getValues(rx) == r$pixel_block()))

  r$save_as(tempfile("new_raster", fileext=".img"))
  r$save_as(tempfile("new_raster", fileext=".crf"))
  r$save_as(file.path(fgdb_path, "logo"))
}

# arc.write() support spacial pixels
check_3 <- function()
{
  data(meuse.grid, package="sp")
  sp::coordinates(meuse.grid) <- c("x", "y")
  sp::gridded(meuse.grid) <- TRUE
  meuse.grid@proj4string=sp::CRS(arc.fromWktToP4(28992))

  #copy meuse.grid to FDGB, force pixel type to double
  arc.write(file.path(fgdb_path, "meuse_grid"), meuse.grid, pt="F64")
}

# r$write_pixels(), copy R logo to my office
check_4 <- function()
{
  to_new_raster_dataset <- function(vals, px_type, nrow, ncol)
  {
    # create an empty raster dataset
    r <- arc.raster(NULL, path=file.path(fgdb_path, paste0("r_logo", px_type)),
                  dim=sz, pixel_type=px_type, sr=3310,
                  extent=c(258743.988738779,-436020.790477021,258755.37546478,-436005.376738167))

    r$write_pixel_block(vals, nrow = nrow, ncol = ncol)
    #close and compute stats
    r$commit()
  }

  x <- rgdal::GDAL.open(system.file("pictures", "logo.jpg", package="rgdal"))
  vals <- rgdal::getRasterData(x)
  sz <- dim(x)
  rgdal::GDAL.close(x)

  #write all types but 1 bit
  ret<-sapply(names(.arc$pixel_type2data_type[-1]), function(x)
  {
    d <-to_new_raster_dataset(vals, x, sz[1], sz[2]);
    v <- arc.raster(d)$pixel_block()
    sum(v)
  })

  stopifnot(ret == c("7174656","48640","7174656","7174656","7174656","7174656","7174656", "7174656"))
}

if (Sys.getenv("_R_CHECK_INTERNALS2_")[[1]] != "")
{
  check_1()
  check_2()
  check_3()
  check_4()
}
