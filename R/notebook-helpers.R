
#' @export
repr_html.arc.productinfo<-function(obj,...)
{
  sprintf("<pre>%s</pre>", format(obj, fmt = "html"));
}

#' @export
repr_html.arc.portalinfo<-function(obj,...)
{
  sprintf("<pre>%s</pre>", format(obj, fmt = "html"));
}

#' @export
repr_html.arc.shapeinfo <- function(obj, ...)
{
  x <- format(obj, fmt = "html")
  ret <- paste(paste0(format(names(x), width=16), ": ", x), collapse="\n")
  return (sprintf("<pre>%s</pre>", ret))
}

#' @export
repr_html.arc.data <- function(obj, ...)
{
  s <- arc.shape(obj)
  if (is.null(s))
    return (NextMethod())
  ret <- format(arc.shapeinfo(s), fmt = "html")
  return (ret)
  #paste(ret, NextMethod("repr_html", obj, ...), sep = "<br/>")
}

#' @export
repr_html.leaflet <- function(obj, ...)
{
  if (is.null(obj$elementId))
    stop("$elementId == NULL. Required property $elementId.")

  dir_maps <-normalizePath('~/../.ipython/nbextensions/maps', mustWork = FALSE)
  libdir <- 'mlib'
  dir.create(dir_maps, showWarnings = FALSE, recursive = TRUE, mode = "0777")
  #oldwd <- setwd(dir_maps); on.exit(setwd(oldwd), add = TRUE)

  name<-paste0(obj$elementId, '.html')
  html <- htmltools::as.tags(obj, standalone=T)
  htmltools::save_html(html, file = file.path(dir_maps, name), libdir = libdir, background = "white")

  w <- ifelse(is.null(obj$width), obj$sizingPolicy$defaultWidth, obj$width)
  h <- ifelse(is.null(obj$height), obj$sizingPolicy$defaultHeight, obj$height)
  #sprintf('<div><object width="%s" height="%s" type="text/html" data="/nbextensions/maps/%s"></object></div>', w, h, name)
  sprintf('<iframe width="%s" height="%s" frameborder="0" src="/nbextensions/maps/%s"></iframe>', w, h, name)

  #rendered <- htmltools::renderTags(html)
  #deps <- lapply(rendered$dependencies, function(dep) {
  #  dep <- htmltools::copyDependencyToDir(dep, libdir, FALSE)
  #  dep <- htmltools::makeDependencyRelative(dep, libdir, FALSE)
  #  dir <- dep$src[[1]]
  #  dep$src <- c(file=file.path('/nbextensions/maps/mlib',dir))
  #  dep
  #})

  #html <- c("<!DOCTYPE html>",
  #          #"<html><head>",
  #          #"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF8\">",
  #          htmltools::renderDependencies(deps, c("href", "file")),
  #          rendered$head,
  #          #"</head><body>",
  #          rendered$html,
  #          #"</div></body></html>",
  #          "</div>")
  #paste0(html, collapse='')
}

#' @export
polygonData.arc.data <- function(obj)
{
  if (requireNamespace("sf", quietly = TRUE))
  {
   utils::getFromNamespace("polygonData.sf", "leaflet")(arc.data2sf(obj))
   #(leaflet:::polygonData(arc.data2sf(obj)))
  }
  else if (requireNamespace("sp", quietly = TRUE))
  {
    utils::getFromNamespace("polygonData.sp", "leaflet")(arc.data2sp(obj))
    #(leaflet:::polygonData(arc.data2sp(obj)))
  }
  else
    stop("This function requires 'sf' or 'sp' package.")
}

#' @export
pointData.arc.data <- function(obj)
{
  xy <- arc.shape(obj)
  stopifnot(arc.shapeinfo(xy)$type == "Point")
  names(xy) <- c("lng", "lat")
  xy
}
