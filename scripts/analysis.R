###
### Required libraries
###
library(data.table)

###
### Raw simulation data output directory
###
outputPath <- "/data/workspace/hpc/trustCoalitionHPC/output"

data <- read.csv(paste(outputPath,"/data.csv",sep=""), sep=";", header=TRUE)