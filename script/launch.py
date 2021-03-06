#!/usr/bin/env python

import os
from os.path import dirname, join
import time

#hostfile_name = "machinefiles/localserver"
hostfile_name = "machinefiles/serverlist"

app_dir = dirname(dirname(os.path.realpath(__file__)))
proj_dir = dirname(dirname(app_dir))

hostfile = join(proj_dir, hostfile_name)

ssh_cmd = (
    "ssh "
	"-o StrictHostKeyChecking=no "
    "-o UserKnownHostsFile=/dev/null "
    )

# Get host IPs
with open(hostfile, "r") as f:
  hostlines = f.read().splitlines()
host_ips = [line.split()[1] for line in hostlines]

for client_id, ip in enumerate(host_ips):
  cmd_cp = "scp "+join(app_dir,"script/")+"run_local.py " + "fangling@%s:"%ip
  cmd_cp += join(app_dir,"script/")
  cmd = ssh_cmd + ip + " "
  cmd += "\'python " + join(app_dir, "script/run_local.py")
  cmd += " %d %s\'" % (client_id, hostfile)
  cmd += " &"
  print cmd
  print cmd_cp
  os.system(cmd)
  os.system(cmd_cp)

  if client_id == 0:
    print "Waiting for first client to set up"
    time.sleep(2)
