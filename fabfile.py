import re
import os

from fabric.api import env, cd, run
from fabric.contrib.project import rsync_project

user = 'vehicle'
env.hosts = [
    "%s@vlionthu.com:9022" % user
]
project_root = "/home/%s/Advanced-TDMA-NS3-Simulation" % user

def deploy(configure=False):
    with open("./.gitignore") as f:
        excludes = re.sub('\n+', "\n", f.read()).split("\n")
    excludes.append(".git")
    rsync_project(project_root, "./", exclude=excludes)
    with cd(os.path.join(project_root, "ns-3.29")):
        if configure:
            run("./waf configure --build-profile=optimized --enable-examples")
        run("./waf build")