R-bridge for ArcGIS
=========
#### Requirements
 - [ArcGIS 10.3.1](http://desktop.arcgis.com/en/desktop/) or [ArcGIS Pro 1.1](http://pro.arcgis.com/en/pro-app/) ([don't have it? try a 60 day trial](http://www.esri.com/software/arcgis/arcgis-for-desktop/free-trial))
 - [R Statistical Computing Software, 3.1.0 or later](http://cran.cnr.berkeley.edu/bin/windows/base/) ([What is R?](http://www.r-project.org/about.html))
  + 32-bit version required for ArcMap, 64-bit version required for ArcGIS Pro (Note: the installer installs both by default).
  + 64-bit version can be used with ArcMap by installing [Background Geoprocessing](http://desktop.arcgis.com/en/desktop/latest/analyze/executing-tools/64bit-background.htm) and configuring scripts to [run in the background](http://desktop.arcgis.com/en/desktop/latest/analyze/executing-tools/foreground-and-background-processing.htm).

####Installation
#####Update command:
####Basic GP Tool script
```R
tool_exec <- function (in_params, out_params)
{
  value0 <- in_params[[1]]
  print(value0)
# ...
  return (out_params)
}
```
>- `tool_exec(in_params, out_params)` main function
- `in_params` list of all input parameters. You can get parameter value by index `in_params[[1]]` or by parameter name `in_params$param1`
- `output_params` list of all output and derived parameters.

####Using `arcgisbinding` in standalone R script
```R
> library(arcgisbinding)
> arc.check_product()
```

####Build from source
- Create new folder `<ArcGIS>\R-bridge`
- Set as current directory and clone repository  
`git clone https://github.com/R-ArcGIS/r-bridge.git ./src`
- Open `R-bridge.sln` (Visual Studio 2012) and build solution

######Build dependencies:
- ArcObjects SDK for C++ 10.3.1 ([requirements](http://desktop.arcgis.com/en/desktop/latest/get-started/system-requirements/arcobjects-sdk-system-requirements.htm))
- [Rtools 3.1 or 3.2](http://cran.r-project.org/bin/windows/Rtools)

######Documentation dependencies:
- [roxygen2](https://github.com/yihui/roxygen2), install from R: `install.packages('roxygen2')`
- [MiKTeX 2.9](http://miktex.org/), to build package PDF

###Repository layout
- .\package  
  `arcgisbinding` (native R package) - collection of classes and functions for script-level bindings between R and bridge dll.
- .\rarcproxy  
  C++, Bridge between ArcGIS 10.3.1 and `arcgisbinding`.
- .\rarcproxy_pro  
  C++, Bridge between ArcGIS Pro and `arcgisbinding`.
- .\libarcobjects  
  (private) - Static library for rarcproxy_pro. Wrapper classes for ArcObjects API.

##Credits

This package depends on the R Statistical Computing Software:

> Copyright (C) 2015 The R Foundation for Statistical Computing
> R is free software and comes with ABSOLUTELY NO WARRANTY.
> See the [COPYRIGHTS file](https://github.com/wch/r-source/blob/trunk/doc/COPYRIGHTS) for details.

## License
[Apache 2.0](LICENSE)
