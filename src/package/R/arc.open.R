#' Open dataset, table, or layer
#'
#' Open ArcGIS datasets, tables and layers. Returns a new
#' \code{\link{arc.dataset-class}} object which contains details on both the spatial
#' information and attribute information (data frame) contained within the dataset.
#'
#' @section Supported Formats:
#' \itemize{
#'   \item \code{Feature Class}: A collection of geographic features with the
#'   same geometry type (i.e. point, line, polygon) and the same spatial reference,
#'   combined with an attribute table. Feature classes can be stored in a variety
#'   of formats, including: files (e.g. Shapefiles), Geodatabases, components
#'   of feature datasets, and as coverages. All of these types can be accessed
#'   using the full path of the relevant feature class (see note below on how to
#'   specify path names).
#'   \item \code{Layer}: A layer references a feature layer, but also includes
#'   additional information necessary to symbolize and label a dataset appropriately.
#'   \code{arc.open} supports active layers in the current ArcGIS session, which
#'   can be addressed simply by referencing the layer name as it is displayed within
#'   the application. Instead of referencing file layers on disk (i.e.
#'   \code{.lyr} and \code{.lyrx} files), the direct reference to the actual dataset
#'   should be used.
#'   \item \code{Table}: Tables are effectively the same as data frames, containing
#'   a collection of records (or observations) organized in rows, with columns
#'   storing different variables (or fields). Feature classes similarly contain a
#'   table, but include the additional information about geometries lacking in a
#'   standalone table. When a standalone table is queries for its spatial information,
#'   e.g. \code{arc.shape(table)}, it will return \code{NULL}. Table data types include
#'   formats such as text files, Excel spreadsheets, dBASE tables, and INFO tables.
#' }
#' @note Paths must be properly quoted for the Windows platform. There are two styles
#' of paths that work within R on Windows:
#' \itemize{
#'   \item Doubled backslashes, such as:
#'         \code{C:\\\\Workspace\\\\archive.gdb\\\\feature_class}.
#'   \item  Forward-slashes such as:
#'         \code{C:/Workspace/archive.gdb/feature_class}.
#' }
#' Network paths can be accessed with a leading
#' \code{\\\\\\\\host\\share} or \code{//host/share} path.
#' To access tables and data within a Feature Dataset, reference the full path to
#' the dataset, which follows the structure:
#'     \code{<directory>/<Geodatabase Name>/<feature dataset name>/<dataset name>}.
#' So for a table called \code{table1} located in a feature dataset {fdataset} within
#' a Geodatabase called \code{data.gdb}, the full path might be:
#'  \code{C:/Workspace/data.gdb/fdataset/table1}
#' @section References:
#' \itemize{
#'   \item \href{http://support.esri.com/es/knowledgebase/techarticles/detail/40057}{What is the difference between a shapefile and a layer file?}
#'   \item \href{https://desktop.arcgis.com/en/desktop/latest/map/working-with-layers/what-is-a-layer-.htm}{ArcGIS Help: What is a layer?}
#'   \item \href{http://desktop.arcgis.com/en/desktop/latest/manage-data/tables/what-are-tables-and-attribute-information.htm}{ArcGIS Help: What are tables and attribute information?}
#' }
#' @name arc.open
#' @docType methods
#' @param path file path or layer name
#' @return An \code{arc.dataset} object
#' @seealso \code{\link{arc.dataset-class}}
#' @keywords datasets open table feature
#' @examples
#'
#' ozone.file <- system.file("extdata", "ca_ozone_pts.shp",
#'                           package="arcgisbinding")
#' d <- arc.open(ozone.file)
#' cat('all fields:', names(d@@fields), fill = TRUE) # print all fields
#'
#' @export
arc.open <- function (path)
{
  ds <- .open.dataset(path)
  if (is(ds, "arc.feature"))
    return(as(ds, "arc.feature"))
  if (is(ds, "arc.table"))
    return(as(ds, "arc.table"))
  if (is(ds, "arc.container"))
    return(as(ds, "arc.container"))
  warning("open as :", ds@dataset_type)
  return(ds)
}

#arc.open.table <- function (path) new("arc.table_impl", path = path)
#arc.open.fc <- function (path) new("arc.feature_impl", path = path)
.open.dataset <- function (path) new("arc.dataset_impl", path = path)
