import sys
import numpy as np

disMax=465

def analysis(root_dir,index):
    f=open("../mobility/fcd-trace-"+str(index)+".ns2.output.xml.node_start_time.txt")
    start=np.array([float(i) for i in f.read().split('\n') if i!=''])
    f.close()
    nnode=len(start)
    packets={}
    delay=[[],[]]
    throughput=np.array([0]*nnode)
    with open(root_dir+"/stdout_"+str(index)+".log") as a:
        for i in a:
            if "Tick" in i:
                print(i,end='')
            tmp=i.split(',')
            if len(tmp)<3:
                continue
            if tmp[1] not in packets:
                packets[tmp[1]]=[0,0]
            if len(tmp)==3:
                sid=256*int(tmp[0].split(':')[-2],16)+int(tmp[0].split(':')[-1],16)-1
                packets[tmp[1]][0]=sid
                packets[tmp[1]][1]=int(tmp[2])
            elif len(tmp)==4:
                rid=256*int(tmp[0].split('.')[-2])+int(tmp[0].split('.')[-1])-1
                sid=256*int(tmp[2].split(':')[-2],16)+int(tmp[2].split(':')[-1],16)-1
                if rid<8:
                    if sid<8:
                        throughput[rid]+=1
                else:
                    throughput[rid]+=1 
                if packets[tmp[1]][0]<8:
                    ind=0
                else:
                    ind=1
                delay[ind].append(int(tmp[3])-packets[tmp[1]][1])
                
    throughput=throughput/(600-start)
    apDelay=sum(delay[0])/len(delay[0])
    hdvDelay=sum(delay[1])/len(delay[1])
    coreThroughput=sum(throughput[:8])/len(throughput[:8])
    hdvThroughput=sum(throughput[8:])/len(throughput[8:])
    return [apDelay,hdvDelay,coreThroughput,hdvThroughput]

if __name__ == "__main__":
    index=sys.argv[1]
    result = analysis("output_ref",index)
    with open("result_ref.txt",'a') as f:
        f.write(str(index)+'\n')
        for r in result:
            f.write(str(r)+',')
        f.write('\n')