# feature tests
if (!isNamespaceLoaded("arcgisbinding"))
{
  library(arcgisbinding)
  arc.check_product()
}

fgdb_path = tempfile("new_databse",fileext=".gdb")

check_1 <- function()
{
  stopifnot(arc.fromWktToP4(2017) == "+proj=tmerc +lat_0=0 +lon_0=-73.5 +k=0.9999 +x_0=304800 +y_0=0 +ellps=clrk66 +units=m +no_defs")
  stopifnot(arc.fromP4ToWkt("+proj=tmerc +lat_0=0 +lon_0=-73.5 +k=0.9999 +x_0=304800 +y_0=0 +ellps=clrk66 +units=m +no_defs") ==
    "PROJCS[\"NAD_1927_DEF_1976_MTM_8\",GEOGCS[\"GCS_NAD_1927_Definition_1976\",DATUM[\"D_NAD_1927_Definition_1976\",SPHEROID[\"Clarke_1866\",6378206.4,294.9786982]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",304800.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",-73.5],PARAMETER[\"Scale_Factor\",0.9999],PARAMETER[\"Latitude_Of_Origin\",0.0],UNIT[\"Meter\",1.0]]")
  stopifnot("+proj=robin +lon_0=0 +x_0=0 +y_0=0 +datum=WGS84 +units=m +no_defs" == arc.fromWktToP4(54030))
  stopifnot(arc.fromP4ToWkt(arc.fromWktToP4(4326)) ==
   "GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]]")
}
check_2 <- function()
{
  #create an empty table
  x <- data.frame(a=1,b=1)
  x <- x[-1,]
  stopifnot(nrow(x) == 0L)

  f <- file.path(fgdb_path, "table_zerro")
  arc.write(f, data=x)

  #validate result
  t <- arc.select(arc.open(f))
  stopifnot(names(x) %in% names(t))
  stopifnot(nrow(t) == nrow(x))
  rm(t)
  #delete table
  stopifnot(arc.delete(f))
}

if (Sys.getenv("_R_CHECK_INTERNALS2_")[[1]] != "")
{
  check_1()
  check_2()
}
