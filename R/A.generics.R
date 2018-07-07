# Create RasterLayer or RasterBrick (raster package)
if (!isGeneric("as.raster"))
{
  setGeneric("as.raster", function(x, ...) standardGeneric("as.raster"))
}

# Load dataset to a standard data frame.
setGeneric(name = "arc.select", def = function(object, fields, where_clause, selected, sr, ...) standardGeneric("arc.select"))

setGeneric(name = "arc.shapeinfo", def = function(object) standardGeneric("arc.shapeinfo"))

# Create arc.raster object
setGeneric(name = "arc.raster", def = function(object, bands, ...) standardGeneric("arc.raster"))

# Get metadata
setGeneric(name = "arc.metadata", def = function(object) standardGeneric("arc.metadata"))
