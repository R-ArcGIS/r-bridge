#' @export
arc.shape <- function (x)
{
  if (inherits(x, "arc.data"))
    attr(x, "shape")
  else
    NULL
}

.shape_len<-function(x) length(x[[1]])

setMethod("length", "arc.shape", .shape_len)

#setMethod("[", signature(x="arc.shape", i="numeric"), function(x,i)
"[.arc.shape" <-  function(x, i)
{
  stopifnot(is.numeric(i))
  info <- arc.shapeinfo(x)
  new(class(x), lapply(x, function(o) o[i]) , shapeinfo=info)
}

setMethod("show", "arc.shape", function(object)
{
  info <- arc.shapeinfo(object)
  x <- c(format(info), length=.shape_len(object))
  cat(paste(paste0(format(names(x), width=16), ": ", x), collapse="\n"), "\n")
  invisible(object)
})
