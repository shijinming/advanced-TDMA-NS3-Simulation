import sys
import numpy as np

def analysis(root_dir,index):
    f=open("../mobility/fcd-trace-"+str(index)+".ns2.output.xml.node_start_time.txt")
    start=np.array([float(i) for i in f.read().split('\n') if i!=''])
    f.close()
    f=open("middle_time.txt")
    middle_time=np.array([float(i) for i in f.read().split('\n')[int(index)-1].split(',')])
    nnode=len(start)
    packets={}
    position=np.array([-1]*nnode)
    outter=list(range(8,nnode))
    delay=[[],[],[],[]]
    throughput=np.array([0.0]*nnode)
    throughput1=np.array([0.0]*nnode)
    broadcast=[0.0,0,0.0,0]
    with open(root_dir+"/stdout_"+str(index)+".log") as a:
        for i in a:
            if 'position' in i:
                tmp=i.split(' ')
                if start[int(tmp[0])]<=int(tmp[3])/1000:
                    position[int(tmp[0])]=float(tmp[2])
            if 'middle' in i:
                nid=int(i.split(' ')[0])
                if 'add' in i:          
                    outter.remove(nid)
                elif 'quit' in i:
                    outter.append(nid)
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
                if sid<8:
                    broadcast[1]+=1
                else:
                    broadcast[3]+=1
            elif len(tmp)==4:
                rid=256*int(tmp[0].split('.')[-2])+int(tmp[0].split('.')[-1])-1
                sid=256*int(tmp[2].split(':')[-2],16)+int(tmp[2].split(':')[-1],16)-1
                if sid<8:
                    broadcast[0]+=1
                else:
                    broadcast[2]+=1
                if rid<8:
                    if sid<8:
                        throughput[rid]+=1
                        delay[0].append(int(tmp[3])-packets[tmp[1]][1])
                    else:
                        throughput1[rid]+=1
                        delay[1].append(int(tmp[3])-packets[tmp[1]][1])
                else:
                    if sid>=8:
                        throughput[rid]+=1
                        delay[2].append(int(tmp[3])-packets[tmp[1]][1])
                    else:
                        throughput1[rid]+=1
                        delay[3].append(int(tmp[3])-packets[tmp[1]][1])

                
    throughput=throughput/(600-start)
    apDelay=sum(delay[0])/len(delay[0])
    hdvDelay=sum(delay[2])/len(delay[2])
    hdv2ap=sum(delay[1])/len(delay[1])
    ap2hdv=sum(delay[3])/len(delay[3])
    coreThroughput=sum(throughput[:8])/8
    hdvThroughput=sum(throughput[8:])/len(throughput[8:])
    core2hdv=[throughput1[i]/middle_time[i] for i in range(8,nnode) if middle_time[i]>0]
    core2hdv=sum(core2hdv)/len(core2hdv)
    hdv2core=sum(throughput1[:8]/(600-start[:8]))/8
    apBroadcast=broadcast[0]/broadcast[1]
    hdvBroadcast=broadcast[2]/broadcast[3]
    return [apDelay,hdvDelay,ap2hdv,hdv2ap,coreThroughput,hdvThroughput,core2hdv,hdv2core,apBroadcast,hdvBroadcast]

if __name__ == "__main__":
    index=sys.argv[1]
    result = analysis("output_ref",index)
    with open("result_ref.txt",'a') as f:
        f.write(str(index)+'\n')
        for r in result:
            f.write(str(r)+',')
        f.write('\n')