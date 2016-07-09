tool_exec <- function(in_params, out_params)
{
### handle input gp tool paramaters
  input_dataset_name <- in_params[[1]]
  fields <- in_params[[2]]

# --- import features to dataframe ---
  fc <- arc.open(input_dataset_name)
  df <- arc.select(fc, fields)

# --- import to SP geometry ---
  g <- arc.data2sp(df)
  print(summary(g))

# update derived parameter

  return(out_params)
}
