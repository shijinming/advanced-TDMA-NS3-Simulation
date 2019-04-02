import os
import argparse
import subprocess
import time
import sys
from worker import run_bash_task

mobility_data_num = 17
BASE_DIR = os.path.abspath(os.path.dirname(__file__))

def reset_dir():
    os.chdir(BASE_DIR)

def build_project(profile="debug"):
    os.chdir(os.path.join(BASE_DIR, "ns-3.29"))
    subprocess.check_call(["./waf", "configure", 
        "--build-profile={}".format(profile),
        "--enable-examples", "--enable-tests"])
    subprocess.check_call(["./waf", "build"])
    reset_dir()

def convert_settings_to_cmd_flags(settings):
    flags = []
    for (key, val) in settings.items():
        flags.append("--{}={}".format(key, val))
    return flags

def run_project(settings, print_cmd_only=False, stdout=None):
    flags = convert_settings_to_cmd_flags(settings)
    flags = " ".join(flags)
    cmd_to_run = ["./waf", "--cwd={}".format(BASE_DIR), 
        '--run', 'mix-autonomy-example {}'.format(flags)]
    if not print_cmd_only:
        os.chdir(os.path.join(BASE_DIR, "ns-3.29"))
        subprocess.check_call(cmd_to_run, stdout=stdout, stderr=stdout)
    reset_dir()
    return cmd_to_run

def get_configure_params():
    parser = argparse.ArgumentParser("run the project with ns2 mobility data")
    parser.add_argument("-b", "--build", action="store_true", default=True,
        help="build the project")
    parser.add_argument("-r", "--run", action="store_true",default=True,
        help="run the project")
    parser.add_argument("--profile", type=str, default="optimized",
        help="build profile, should be debug or optimized")
    parser.add_argument("--worker", type=int, default=8,
        help="number of workers")
    parser.add_argument("--output", type=str, default="output1",
        help="Output directory")
    parser.add_argument("--sim-idx", type=int,
        help="REQUIRED, Simulation id, must be unique")
    parser.add_argument("--extra", type=str, default=None, 
        help="Extra simulation config params input, in yml file")
    parser.add_argument("--wait", action="store_true",default=False,
        help="wait until all tasks finish")
    parser.add_argument("--task-id", type=int, default=-1,
        help="run this task only")
    parser.add_argument("--sim-time", type=int, default=600,
        help="simulation time")
    parser.add_argument("--reversed", action="store_true", 
        help="if true, add tasks by task_id increasingly")
    return parser.parse_args()

def get_node_num(input_file):
    f=open(input_file)
    a=f.read().split('\n')
    f.close()
    return len([i for i in a if i!=''])

class SequentialSimulator:
    
    def __init__(self, cmd_opts, *args, **kwargs):
        self.max_workers = cmd_opts.worker
        self.cmd_opts = cmd_opts
#         self.pool = concurrent.futures.ThreadPoolExecutor(self.max_workers)

        if not cmd_opts.run:
            return
        self.output_dir = os.path.join(BASE_DIR, cmd_opts.output)
        if not os.path.isabs(self.output_dir):
            self.output_dir = os.path.join(BASE_DIR, self.output_dir)
        self.sim_idx = cmd_opts.sim_idx
        if not os.path.exists(self.output_dir):
            os.mkdir(self.output_dir)

    def start(self, tasks=[]):
        print("%s starts" % self.__class__.__name__)
        if not self.prepare(tasks):
            print("Warning: no tasks to run")
            exit(1)
        res = []
        for task in tasks:
            res.append(self.do_run(task['task-id'], task))
        if self.cmd_opts.wait:
            for x in res:
                x.get()
            print("%s ends" % self.__class__.__name__)
        else:
            print("task dispatched!")
    
    def do_run(self, task_id, task):
        if self.cmd_opts.task_id >= 0 and self.cmd_opts.task_id != task_id:
            return
        print("Running task %s" % task_id)
        cmd = run_project(task, print_cmd_only=True, stdout=None)
        stdout_file = os.path.join(BASE_DIR, self.output_dir, "stdout_{}.log".format(task_id))
        working_dir = os.path.abspath(os.path.join(BASE_DIR, "ns-3.29"))
        res = run_bash_task(cmd, stdout_file, working_dir)
        time.sleep(0.1)
        return res

    def prepare(self, tasks):
        if self.cmd_opts is None:
            print("WARN: Cmd opts is None!")
            return False
        if self.cmd_opts.build:
            build_project(self.cmd_opts.profile)
        if len(tasks) == 0 and self.cmd_opts.run:
            print(len(tasks),self.cmd_opts.run)
            self.genrate_tasks(tasks)
        return self.cmd_opts.run
    
    def genrate_tasks(self, tasks):
        task_num = 16
        if self.cmd_opts.reversed:
            iterator = range(1, 1 + task_num)
        else:
            iterator = reversed(range(1, 1 + task_num))
        iterator=range(16,17)
        for idx in iterator:
            task = {
                "sim-time": self.cmd_opts.sim_time,
                "ap-num": 8,
            }
            task["cw-min"]=15
            task["cw-max"]=1023
            task["send-num"]=8
            task["mobility"] = os.path.join(BASE_DIR,
                "../mobility1", "fcd-trace-{}.ns2.output.xml".format(idx))
            trace_file_name = task["mobility"].split("/")[-1]
            node_start_time = os.path.join(BASE_DIR, "../mobility",
                "{}.node_start_time.txt".format(trace_file_name))
            task["nnodes"] = get_node_num(node_start_time)
            task["task-id"] = idx
            tasks.append(task)

if __name__ == "__main__":
    opts = get_configure_params()
    SequentialSimulator(opts).start()