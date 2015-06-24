#' Convert PROJ.4 Coordinate Reference System string to Well-known Text.
#'
#' The \code{arc.fromP4ToWkt} command converts a PROJ.4 coordinate
#' reference system (CRS) string to a well-known text (WKT) representation.
#' Well-known text is used by ArcGIS and other applications to robustly
#' describe a coordinate reference system. Converts PROJ.4 stings which
#' include either the '+proj' fully specified projection parameter, or the
#' '+init' form that takes well-known IDs (WKIDs), such as EPSG codes,
#' as input.
#'
#' The produced WKT is equivalent to the ArcPy spatial reference
#' exported string:
#'
#'   \code{arcpy.Describe(layer).SpatialReference.exportToString()}
#' @note The '+init' method currently only works with ArcGIS Pro.
#'
#' @section References:
#'  \enumerate{
#'    \item OGC specification
#'      \href{http://docs.opengeospatial.org/is/12-063r5/12-063r5.html#36}{
#'    12-063r5}
#'    \item \href{http://desktop.arcgis.com/en/desktop/latest/guide-books/map-projections/what-are-map-projections.htm}{ArcGIS Help: What are map projections?}
#' }
#' @param proj4 PROJ.4 projection string
#' @seealso \code{\link{arc.fromWktToP4}}
#' @examples
#'
#' arc.fromP4ToWkt("+proj=eqc") # Equirectangular
#'
#' arc.fromP4ToWkt("+proj=latlong +datum=wgs84") # WGS 1984 geographic
#'
#' arc.fromP4ToWkt("+init=epsg:2806") # initalize based on EPSG code
#' @export
arc.fromP4ToWkt <- function (proj4)
{
  if (is.null(proj4) || is.na(proj4))
    return(as.character(NA))
  if (class(proj4) == "CRS")
    str <- as.character(proj4@projargs)
  else
    str <- as.character(proj4)

  .call_proxy("arc_fromP42Wkt", str)
}

#' Convert a Well-known Text Coordinate Reference System into a PROJ.4 string.
#'
#' Convert a well-known text (WKT) coordinate reference system (CRS) string to
#' a PROJ.4 representation. PROJ.4 strings were created as a convenient way to
#' pass CRS information to the command-line PROJ.4 utilities, and have an
#' expressive format. Alternatively, can accept a well-known ID (WKID),
#' a numeric value that ArcGIS uses to specify projections. See the 'Using
#' spatial references' resource for lookup tables which map between WKIDs and
#' given projection names.
#'
#' @section References:
#'  \enumerate{
#'    \item \href{http://resources.arcgis.com/en/help/arcgis-rest-api/index.html#/Using_spatial_references/02r3000000qq000000/}{ArcGIS REST API: Using spatial references}
#'    \item OGC specification
#'      \href{http://docs.opengeospatial.org/is/12-063r5/12-063r5.html#36}{
#'    12-063r5}
#'    \item \href{http://desktop.arcgis.com/en/desktop/latest/guide-books/map-projections/what-are-map-projections.htm}{ArcGIS Help: What are map projections?}
#' }
#' @param wkt WKT projection string, or a WKID integer
#' @seealso \code{\link{arc.fromP4ToWkt}}
#' @examples
#'
#' d <- arc.open(system.file("extdata", "ca_ozone_pts.shp",
#'                           package="arcgisbinding"))
#' arc.fromWktToP4(d@@shapeinfo$WKT)
#'
#' arc.fromWktToP4(4326) # use a WKID for
#' @export
arc.fromWktToP4 <- function (wkt)
{
  .call_proxy("arc_fromWkt2P4", wkt)
}
