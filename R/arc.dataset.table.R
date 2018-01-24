setMethod("names", "arc.table", def = function(x) names(x@fields) )

setMethod("initialize", "arc.table", def = function(.Object, path) {
  .Object <- callNextMethod(.Object, path)
  .Object@fields <- as.list(.call_proxy("table.fields", .Object))
  return(.Object)
})

setMethod("show", "arc.table", function(object) {
  callNextMethod(object)
  cat(names(object@fields), sep = ", ", labels = paste0(format("fields", width=16), ":"), fill = TRUE)
  #cat("\n")
  invisible(object)
})

#' @method arc.select arc.table
#' @rdname arc.select
setMethod("arc.select", "arc.table", def = function(object, fields, where_clause, selected, sr, ...)
{
  .select(object, fields=fields, where_clause=where_clause, selected=selected, na.rm = !is(object, "arc.feature"), sr = sr, ...)
})


setClass("arc.table_impl", contains = "arc.table")
setMethod("initialize", "arc.table_impl", def = function(.Object, base)
{
  stopifnot(!is.null(base))
  .call_proxy("table.create_from", .Object, base)
  callNextMethod(.Object, base@path)
})

setIs("arc.dataset", "arc.table",
  test = function(from) .call_proxy("dataset.is_table", from),
  coerce = function(from) new("arc.table_impl", base = from),
  replace = function(obj, value) value)
