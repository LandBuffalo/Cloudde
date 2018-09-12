#!/bin/bash
cd ..
bash bash_NCI.sh
cd ./bin

qsub ./CloudDE.pbs
qstat -u cxj595
