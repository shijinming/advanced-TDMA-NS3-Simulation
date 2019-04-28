import sys
import numpy as np

def getLayer(nid, outter):
    if nid<8:
        r=0
    else:
        if nid in outter:
            r=2
        else:
            r=1
    return r

def analysis(root_dir,index):
    f=open("../mobility/fcd-trace-"+str(index)+".ns2.output.xml.node_start_time.txt")
    start=np.array([float(i) for i in f.read().split('\n') if i!=''])
    f.close()
    nnode=len(start)
    outter=list(range(8,nnode))
    middle_time=np.array([0.0]*nnode)
    middle_start=np.array([0.0]*nnode)
    with open(root_dir+"/stdout_"+str(index)+".log") as a:
        for i in a:
            if 'middle' in i:
                nid=int(i.split(' ')[0])
                if 'add' in i:          
                    outter.remove(nid)
                    middle_start[nid]=int(i.split('.')[1])
                elif 'quit' in i:
                    outter.append(nid)
                    middle_time[nid]+=int(i.split('.')[1])-middle_start[nid]           
    for i in range(8,nnode):
        if i not in outter:
            middle_time[i]+=600000000-middle_start[i]
    middle_time=middle_time/1000000
    return middle_time

if __name__ == "__main__":
    with open("middle_time.txt",'w+') as f:
        for index in range(1,17):
            print(index)
            result = analysis("output",index)
            for r in result[:-1]:
                f.write(str(r)+',')
            f.write(str(result[-1])+'\n')