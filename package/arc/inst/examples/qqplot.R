tool_exec <- function(in_param, out_param)
{
  message("hello")
  # load as table
  t <- arc.open(as.character(in_param[[1]]))
  df <- arc.select(t, in_param[[2]])

  # prepare plot
  layout(matrix(1:4, nrow=2, ncol=2))

  # get data field
  y <- df[[1]]

  # Histogram
  hist(y, col="lavender", main = in_param[[2]], breaks = 10)
  lines(density(y), lwd = 1.5)

  # QQ Plot
  qqnorm(y)
  qqline(y, col="blue")

  y.sqrt <- sqrt(y)
  qqnorm(y.sqrt, main="Normal Q-Q plot  square-root transformation")
  qqline(y.sqrt, col="red")

  y.log <- log(y)
  qqnorm(y.log, main="Normal Q-Q plot Log transformation")
  qqline(y.log, col="red")
}
