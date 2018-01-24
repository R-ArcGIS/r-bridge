
.select <- function (object, fields, where_clause, selected, na.rm, sr, ...)
{
  if (missing(where_clause)) where_clause <- ""
  if (missing(selected)) selected <- TRUE
  if (missing(sr)) sr <- NULL

  suspend_msg <- FALSE
  if (missing(fields))
  {
    fields <- "*"
    suspend_msg <- TRUE
  }
  #args <- list(...)
  #asWKB <- identical(as.logical(args$asWKB), TRUE)
  #dencify <- if (is.null(args$dencify)) TRUE else  dencify <- identical(as.logical(args$dencify), TRUE)

 if (is.list(fields))
    fields <- unlist(fields, recursive = TRUE, use.names = FALSE)
  all_fields <- names(object@fields)

  if (any(fields == "*"))
  {
    fields <- all_fields
    skip_idx <- grep("\\.?(shape_length|shape_area)$", ignore.case = T, fields)
    skip_idx <- c(skip_idx, which(object@fields %in% c("Blob", "Raster")))
    if (length(skip_idx) > 0)
    {
      if (!suspend_msg)
        warning("In: arc.select() - auto ignoring field: ", paste0(fields[skip_idx], sep=","), call.=F)
      fields <- fields[-skip_idx]
    }
  }
  not_in <- tolower(fields) %in% tolower(all_fields)
  if (any(not_in == FALSE))
    stop("In: arc.select() - field(s) do not exist: ", paste0(fields[!not_in], sep=" "), call.=F)

  selected <- as.logical(selected)

  if (is(object, "arc.feature"))
  {
    shape_idx <- which(object@fields == "Geometry")
    stopifnot(length(shape_idx) == 1)
    shape_name <- names(shape_idx[1])
    # remove all geometry fields from 'fields'
    rm_idx <- grep(paste0("^", shape_name, "$"), fields, ignore.case = T)
    if (length(rm_idx) > 0)
      fields <- fields[-rm_idx]
    # push shape to the front
    fields <- c(shape_name, fields)
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
  args = list(selected = selected,
              where_clause = as.character(where_clause),
              sr = sr, ...)
  ret_list <- .call_proxy("table.select", object, fields, as.pairlist(args))

  if (is(object, "arc.feature"))
  {
    #geometry column always first
    shp <- ret_list[[1]]
    #str(shp)
    info <- object@shapeinfo
    # move new SpatialRefrence (wkt, wkid) to shapeinfo of result dataframe
    wkt <- attr(shp, "WKT")
    if (!is.null(wkt))
    {
      info$WKT <- wkt
      attr(shp, "WKT") <- NULL
    }
    wktid <- attr(shp, "WKID")
    if (!is.null(wktid))
    {
      info$WKID <- wktid
      attr(shp, "WKID") <- NULL
    }
    ret <- if (length(ret_list) > 1)
      as.data.frame(ret_list[-1], row.names = NULL, stringsAsFactors = FALSE)
    else
    {
      #n <- if(is.raw(shp[[1]])) length(shp) else length(shp[[1]])
      n <- length(shp[[1]])
      as.data.frame(row.names = 1:n)
    }
    attr(ret, "shape") <- new("arc.shape", shp, shapeinfo = info)
  }
  else
    ret <- as.data.frame(ret_list, row.names = NULL, stringsAsFactors = FALSE)
  class(ret) <- c("arc.data", class(ret))
  return(ret)
}
