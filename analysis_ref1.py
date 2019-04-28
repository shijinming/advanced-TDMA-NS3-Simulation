import sys
import numpy as np

def analysis(root_dir,index):
    f=open("../mobility/fcd-trace-"+str(index)+".ns2.output.xml.node_start_time.txt")
    start=np.array([float(i) for i in f.read().split('\n') if i!=''])
    f.close()
    f=open("middle_time.txt")
    middle_time=np.array([float(i) for i in f.read().split('\n')[int(index)-1].split(',')])
    nnode=len(start)
    position=np.array([-1]*nnode)
    outter=list(range(8,nnode))
    throughput=np.array([0.0]*nnode)
    with open(root_dir+"/stdout_"+str(index)+".log") as a:
        for i in a:
            tmp=i.split(',')
            if len(tmp)<3:
                continue
            if len(tmp)==3:
                sid=256*int(tmp[0].split(':')[-2],16)+int(tmp[0].split(':')[-1],16)-1
                throughput[sid]+=1
    throughput=throughput/(600-start)
    return [sum(throughput[:8])/8,sum(throughput[8:])/(nnode-8)]

if __name__ == "__main__":
    index=sys.argv[1]
    result = analysis("output_ref",index)
    with open("result_ref1.txt",'a') as f:
        f.write(str(index)+'\n')
        for r in result:
            f.write(str(r)+',')
        f.write('\n')