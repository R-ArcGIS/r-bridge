#' Write dataset, table or layer
#'
#' Export a \code{data.frame} object to an ArcGIS dataset. If the data frame
#' includes a spatial attribute, this function writes a feature dataset. If no
#' spatial attribute is found, a table is instead written.
#'
#' Supports a variety of output formats. Below are pairs of example paths and the resulting data types:
#'  \itemize{
#'   \item \code{C:/place.gdb/fc}: File Geodatabase Feature Class
#'   \item \code{C:/place.gdb/fdataset/fc}: File Geodatabase Feature Dataset
#'   \item \code{in_memory\\logreg}: In-memory workspace (must be run in ArcGIS Session)
#'   \item \code{C:/place.shp}: Esri Shapefile
#'   \item \code{C:/place.dbf}: Table
#' }
#' @section References:
#' \itemize{
#' \item \href{http://support.esri.com/es/knowledgebase/techarticles/detail/40057}{What is the difference between a shapefile and a layer file?}
#' \item \href{https://desktop.arcgis.com/en/desktop/latest/map/working-with-layers/what-is-a-layer-.htm}{ArcGIS Help: What is a layer?}
#' }
#' @param path full output path
#' @param data input data frame. Accepts \code{data.frame}, spatial \code{data.frame},
#' \code{SpatialPointsDataFrame}, \code{SpatialLinesDataFrame}, and
#' \code{SpatialPolygonsDataFrame} objects.
#' @param coords list containing geometry type and spatial reference (optional)
#' @param shape_info (optional)
#' @keywords datasets open write
#' @seealso \code{\link{arc.dataset-class}}, \code{\link{arc.open}}
#' @examples
#'
#' ## write as a shapefile
#' fc <- arc.open(system.file("extdata", "ca_ozone_pts.shp",
#'                            package="arcgisbinding"))
#' d <- arc.select(fc, 'ozone')
#' d[1,] <- 0.6
#' arc.write(tempfile("ca_new", fileext=".shp"), d)
#' ## write as table
#' arc.write(tempfile("tlb", fileext=".dbf"),
#'           list('f1'=c(23,45), 'f2'=c('hello', 'bob')))
#'
#' ## from scratch as feature class
#' arc.write(tempfile("fc_pts", fileext=".shp"), list('data'=rnorm(100)),
#'           list(x=runif(100,min=0,max=10),y=runif(100,min=0,max=10)),
#'           list(type='Point'))
#'
#' @export
arc.write <- function (path, data, coords = NULL, shape_info = NULL)
{
  stopifnot(is.character(path))
  if (!is.null(data) && is.null(coords))
    coords <- arc.shape(data)
  if (!is.null(coords) && is.null(shape_info))
    shape_info <- arc.shapeinfo(coords)

  if (inherits(data, "Spatial"))
  {
    data <- arc.sp2data(data)
    coords <- arc.shape(data)
    if (!is.null(coords) && is.null(shape_info))
      shape_info <- arc.shapeinfo(coords)
  }
  if (is.null(coords) && is.null(data))
    stop("'coords' and 'data' are NULL")
  if (!is.null(coords))
  {
    if (is.null(shape_info))
      stop("shape is missing 'shape_info' attribute")
    if (!any(names(shape_info) == "type"))
      stop("geometry 'type' is missing 'shape_info' attribute")
  }
  if (!is.null(data))
  {
    if (is.list(data)) #data.frame ok
    {
      stopifnot(ncol(data) > 0)
      if(!is.data.frame(data))
      {
       clen <- sapply(data, length)
       if(min(clen) != max(clen))
         stop("differing number of rows: 'data'")
      }
    }
    else if (is.vector(data))
    {
      stopifnot(length(data) > 0)
      data <- list("data"=data);
    }
    else stop("unsupported 'data' type")
  }
  .call_proxy("arc_export2dataset", path, data, coords, shape_info)
  invisible(TRUE)
}
