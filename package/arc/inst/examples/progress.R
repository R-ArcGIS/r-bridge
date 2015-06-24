tool_exec <- function(in_params, out_params)
{
  read.time <- 1.5  # Pause to read what's written on dialog
  loop.time <- 0.3  # Loop iteration delay
  print("Running progress demo ")
  arc.progress_label("This is the default progressor")
  Sys.sleep(read.time)

  for (i in 1:4)
  {
    arc.progress_label(paste("Working on 'phase' ", i))
    print(paste("Messages for phase ", i))
    Sys.sleep(read.time)
  }
  arc.progress_label("Step progressor: Counting from 0 to 100")
  i <- 0
  repeat
  {
    arc.progress_label(paste("Iteration: ", i))
    arc.progress_pos(i)
    cat(paste("pos ", i, "\n"))
    Sys.sleep(loop.time)
    i <- i + 3
    if (i > 100) break
  }
  print("Done counting up")
  arc.progress_pos(-1)
  print("Here comes the countdown...")
  arc.progress_label("Step progressor: Counting backwards from 100 to 0")
  Sys.sleep(read.time)
  print("Counting down now...")
  while (i >= 0)
  {
    arc.progress_label(paste("Iteration: ", i))
    arc.progress_pos(i)
    cat(paste("pos ", i, "\n"))
    Sys.sleep(loop.time)
    i <- i - 3
  }
  print("All done")
  arc.progress_pos(-1)
  return (out_params)
}
