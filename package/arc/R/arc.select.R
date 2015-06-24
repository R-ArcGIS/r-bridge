.select <- function (object, what, where_clause, selected, na.rm, sr)
{
  if (is.list(what))
    what <- unlist(what, recursive = TRUE, use.names = FALSE)
  all_fields <- names(object@fields)
  if (any(what == "*"))
  {
    what <- all_fields
    skip_idx <- grep("\\.?(shape_length|shape_area)$", ignore.case=T, what)
    if (length(skip_idx) > 0)
    {
      warning("In: arc.select() - ignoring field: ",
              paste0(what[skip_idx], sep=","), call.=F)
      what <- what[-skip_idx]
    }
  }
  not_in <- tolower(what) %in% tolower(all_fields)
  if (any(not_in == FALSE))
    stop("In: arc.select() - field(s) do not exist: ",
         paste0(what[!not_in], sep=" "), call.=F)

  selected <- as.logical(selected)

  if (is(object, "arc.feature"))
  {
    shape_idx <- grep("\\.?shape$", ignore.case=T, all_fields)[1]
    if(is.na(shape_idx)) stop("shape fields missing")
    rm_idx <- grep("\\.?shape$", ignore.case=T, what)[1]
    if (!is.na(rm_idx))
      what <- what[-rm_idx]
    what <- c(all_fields[shape_idx], what)
  }

  if (!is.null(sr))
  {
    if (is.numeric(sr))
      sr <- as.integer(sr)
    else if (class(sr) == "CRS")
      sr <- arc.fromP4ToWkt(sr)
    else if (is.character(sr)) # accept +proj
    {
      if (length(grep("^\\+proj=.", sr)) == 1)
        sr <- arc.fromP4ToWkt(sr)
    }
  }

#  ret_list <- .external_proxy("table.select2", object, what,
#                              as.character(where_clause), selected, sr)
  ret_list <- .call_proxy("table.select", object, what,
                            pairlist(selected = selected,
                              where_clause = as.character(where_clause),
                              sr = sr))

  if (is(object, "arc.feature"))
  {
    #geometry column always first
    shp <- ret_list[[1]]
    #str(shp)
    si <- object@shapeinfo
    # move new SpatialRefrence (wkt, wkid) to shapeinfo of result dataframe
    if (!is.null(shp$WKT))
    {
      si$WKT <- shp$WKT
      shp$WKT <- NULL
    }
    if (!is.null(shp$WKID))
    {
      si$WKID <- shp$WKID
      shp$WKID <- NULL
    }
    ret <- as.data.frame(ret_list[-1], row.names = NULL,
                         stringsAsFactors = FALSE)
    attr(ret, "shape") <- new("arc.shape", shp, shapeinfo = si)
  }
  else
    ret <- as.data.frame(ret_list, row.names = NULL, stringsAsFactors = FALSE)
  class(ret) <- append(class(ret), "arc.data")
  return(ret)
}

#' @export
setMethod("arc.select", "arc.table",
    def = function(object, fields = "*", where_clause = "",
                   selected = TRUE, sr = NULL)
      .select(object, fields, where_clause, selected, FALSE, sr))
#' @export
setMethod("arc.select", "arc.feature",
    def = function(object, fields = "*", where_clause = "",
                   selected = TRUE, sr = NULL)
      .select(object, fields, where_clause, selected, FALSE, sr))
