
.arc$resample_types <- c(
  NearestNeighbor       = 1, # RSP_NearestNeighbor = 0,
  BilinearInterpolation = 2, # RSP_BilinearInterpolation = 1,
  CubicConvolution      = 3, # RSP_CubicConvolution = 2,
  Majority              = 4, # RSP_Majority = 3,
  BilinearInterpolationPlus = 5, # RSP_BilinearInterpolationPlus = 4,
  BilinearGaussBlur     = 6, # RSP_BilinearGaussBlur = 5,
  BilinearGaussBlurPlus = 7, # RSP_BilinearGaussBlurPlus = 6,
  Average               = 8, # RSP_Average = 7,
  Minimum               = 9, # RSP_Minimum = 8,
  Maximum               = 10, # RSP_Maximum = 9,
  VectorAverage         = 11) # RSP_VectorAverage = 10
lockBinding("resample_types", .arc)

.arc$pixel_type <- c(
  "U1"       = 1L,  #"PT_U1"        0
  "U2"       = 2L,  #"PT_U2"        1
  "U4"       = 4L,  #"PT_U4"        2
  "U8"       = 8L,  #"PT_UCHAR"     3
  "S8"       = 8L,  #"PT_CHAR",     4
  "U16"      = 16L, #"PT_USHORT",   5
  "S16"      = 16L, #"PT_SHORT",    6
  "U32"      = 32L, #"PT_ULONG",    7
  "S32"      = 32L, #"PT_LONG",     8
  "F32"      = 32L, #"PT_FLOAT",    9,
  "F64"      = 64L, #"PT_DOUBLE",   10,
  "PT_COMPLEX"  = 64L, #"PT_COMPLEX",  11,
  "PT_DCOMPLEX" = 128L,#"PT_DCOMPLEX"  12,
  "PT_CSHORT"   = 32L, #"PT_CSHORT"    13,
  "PT_CLONG"    = 64L) #"PT_CLONG"     14)
lockBinding("pixel_type", .arc)

.arc$pixel_type2data_type <- c(
  "U1"  = "LOG1S",
  "U8"  = "INT1U",
  "S8"  = "INT1S",
  "U16" = "INT2U",
  "S16" = "INT2S",
  "S32" = "INT4S",
  "U32" = "INT4U",
  "F32" = "FLT4S",
  "F64" = "FLT8S")

lockBinding("pixel_type2data_type", .arc)

# Discard arc.object object
.discard <- function (object)
{
  .call_proxy("object.release_internals", object)
  invisible(object)
}

.one_line <- function(str, n = 60)
{
  str <- toString(str)
  ifelse(nchar(str) > n, paste0(substr(str, 1, n), "..."), str)
}
.resample_types_names <- function() names(.arc$resample_types)

#.show_sr0 <- function(sr)
#{
#  #"^(PROJCS|GEOGCS)\['([\w\+-]+)'|(^Unknown)|.+\],VERTCS\['(\w+)"
#  #"^(PROJCS|GEOGCS)\\[\"([^\"]*)\")|(^Unknown)
#  if (!is.null(sr))
#  {
#    #p = '^(PROJCS|GEOGCS)\\["[^"]*"|^Unknown'
#    w <- ifelse(is.null(sr$WKT), "Unknown", sr$WKT)
#    #m <- regexpr(p, w)
#    #cat("WKT          :", regmatches(w, m), "...")
#    if (!is.null(sr$WKID) && sr$WKID > 0L)
#    if (length(sr$WKID) > 0L && sr$WKID > 0L)
#      cat("\nWKID         :", sr$WKID)
#  }
#  invisible(sr)
#}
.format_extent<-function(ex) paste(paste0(names(ex), "=",prettyNum(ex)),collapse=", ")
.format_sr <- function(sr)
{
  w <- ifelse(is.null(sr$WKT), "Unknown", sr$WKT)
  ret <- c(WKT=.one_line(w))
  if (!is.null(sr$WKID) && sr$WKID > 0L)
    ret <-c(ret, WKID=sr$WKID)
  return(ret)
}

#' @export
.call_proxy <- function (n, ...)
{
#  if (inherits(n, "arc.object") && is.null(n@.ptr))
#    stop("disposed arc.object")

  ret <- .Call(n, PACKAGE=.arc$dll, ...)
  if (exists(".arc_err", mode = "character", envir=.GlobalEnv))
  {
    tmp <- get0(".arc_err", .GlobalEnv);
    rm(".arc_err", envir=.GlobalEnv)
    stop(tmp)
  }
  return(ret)
}
