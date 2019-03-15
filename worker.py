import subprocess
import os
from huey import RedisHuey

huey = RedisHuey()

@huey.task()
def run_bash_task(cmd, std_out=None, working_dir=".", **kwargs):
    print("Entring %s" % working_dir)
    print("\nRunning command %s\nStdout to %s\n\n" % (cmd, std_out))

    os.chdir(working_dir)
    if std_out is None:
        subprocess.check_call(cmd, **kwargs)
    else:
        with open(std_out, "w") as f:
            subprocess.check_call(cmd, stdout=f, **kwargs)
    print("finish task %s\n\n" % cmd)

# @huey.task()
# def run_analysis(index):
#     cmd = ['python','analysis.py',str(index)]
#     subprocess.check_call(cmd)