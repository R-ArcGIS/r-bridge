# R-integration for ArcGIS Pro and Desktop

## Requirements

Latest R Core for Windows ([http://cran.r-project.org/bin/windows/base/](http://cran.r-project.org/bin/windows/base/))

Official web page [https://r-arcgis.github.io/](https://r-arcgis.github.io/)

### Installation

```R
> install.packages("arcgisbinding", repos="https://r.esri.com", type="win.binary")
```

### Basic GP Tool R script

```R
tool_exec <- function (in_params, out_params)
{
  value0 <- in_params[[1]]
  print(value0)
# ...
  out_params$output1 <- TRUE
  return (out_params)
}
```

**`tool_exec`**(*in_params*, *out_params*) - required main function

### Parameters

- `in_params` list of all input parameters
- `output_params` list of all output and derived parameters

 You can get tool parameter value by index `in_params[[1]]` or by parameter name `output_params$output1`

### Return Value

- set or update output GP parameters and return `output_params`.

### Using `arcgisbinding` in standalone R script

```R
> library(arcgisbinding)
> arc.check_product()
> d <- arc.open("c:/mydb.gdb/points")
```

### Build

```bash
> git clone https://devtopia.esri.com/r-integration .
```

Rebuild Visual Studio solution `r-integration/src/msprojects/R-bridge.sln`

or

```bash
> cd r-integration/build/
> build.bat pro debug
```

or

[Production build with Docker image](https://devtopia.esri.com/ArcGISPro/r-integration/blob/master/build/Docker/README.md)

### Build dependencies

- Rtools [http://cran.r-project.org/bin/windows/Rtools](http://cran.r-project.org/bin/windows/Rtools)
- *(optional)* MiKTeX 2.9 [http://miktex.org/](http://miktex.org/) - build package PDF
