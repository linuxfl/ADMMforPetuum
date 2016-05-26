#!/usr/bin/env python

"""
This script starts a process locally, using <client-id> <hostfile> as inputs.
"""

import os
from os.path import dirname
from os.path import join
import time
import sys

if len(sys.argv) != 3:
  print "usage: %s <client-id> <hostfile>" % sys.argv[0]
  sys.exit(1)

# Please set the FULL app dir path here
app_dir = "/home/fangling/petuum/bosen/app/ADMM"

client_id = sys.argv[1]
hostfile = sys.argv[2]
proj_dir = dirname(dirname(app_dir))

params = {
    "table_staleness": 0
    #, "output_path": "hdfs://hdfs-domain/user/bosen/dataset/ADMM/sample/output"
    , "feature": 3
    , "row": 3
    , "rho": 1.0
    , "num_epochs": 1000
    #, "data_file": "hdfs://hdfs-domain/user/bosen/dataset/ADMM/sample/data/sample.txt"
    }

petuum_params = {
    "hostfile": hostfile,
    "num_worker_threads": 4
    }

prog_name = "linearRegression_main"
prog_path = join(app_dir, "bin", prog_name)

hadoop_path = os.popen('hadoop classpath --glob').read()

env_params = (
  "GLOG_logtostderr=true "
  "GLOG_v=-1 "
  "GLOG_minloglevel=0 "
  )

# Get host IPs
with open(hostfile, "r") as f:
  hostlines = f.read().splitlines()
host_ips = [line.split()[1] for line in hostlines]
petuum_params["num_clients"] = len(host_ips)

cmd = "killall -q " + prog_name
# os.system is synchronous call.
os.system(cmd)
print "Done killing"

cmd = "export CLASSPATH=`hadoop classpath --glob`:$CLASSPATH; "
cmd += env_params + prog_path
petuum_params["client_id"] = client_id
cmd += "".join([" --%s=%s" % (k,v) for k,v in petuum_params.items()])
cmd += "".join([" --%s=%s" % (k,v) for k,v in params.items()])
print cmd
os.system(cmd)
