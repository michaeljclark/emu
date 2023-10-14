#!/usr/bin/env python3

import os
import re
import subprocess

def wsldir(path):
	return re.sub('^([A-Z]):(.*)', lambda m: "/mnt/%s%s" %
		(m.group(1).lower(), m.group(2)), path)

if os.name == 'nt':
	wd = os.getcwd().replace(os.sep, '/')
	cmd = [ "wsl", "-d", "Ubuntu", "make", "-j", "$(nproc)" , "-C", wsldir(wd) ]
	subprocess.run(cmd, check=True)
