import sys
import numpy as np

disMax=465

def analysis(root_dir,index):
    f=open("../mobility/fcd-trace-"+str(index)+".ns2.output.xml.node_start_time.txt")
    start=np.array([float(i) for i in f.read().split('\n') if i!=''])
    f.close()
    nnode=len(start)
    position=np.array([-1]*nnode)
    outter=list(range(8,nnode))
    packets={}
    delay=[[],[],[],[]]
    receive=[[],[],[],[]]
    throughput=np.array([0]*nnode)
    dismax=0
    with open(root_dir+"/stdout_"+str(index)+".log") as a:
        for i in a:
            if "Tick" in i:
                print(i,end='')
            if 'position' in i:
                tmp=i.split(' ')
                if start[int(tmp[0])]<=int(tmp[3])/1000:
                    position[int(tmp[0])]=float(tmp[2])
            if 'middle' in i:
                if 'add' in i:
                    outter.remove(int(i.split(' ')[0]))
                elif 'quit' in i:
                    outter.append(int(i.split(' ')[0]))
            tmp=i.split(',')
            if len(tmp)<3:
                continue
            if tmp[1] not in packets:
                packets[tmp[1]]=[0,0,0,-1,0,0]
            if len(tmp)==3:
                sid=256*int(tmp[0].split(':')[-2],16)+int(tmp[0].split(':')[-1],16)-1
                packets[tmp[1]][0]=sid
                packets[tmp[1]][1]=int(tmp[2])
                if sid in outter:
                    packets[tmp[1]][2]=1
                if position[sid]>=0:
                    packets[tmp[1]][3]=position[sid]
                    relativeDis = abs([j for j in position if j>=0]-position[sid])
                    packets[tmp[1]][4]=len([d for d in relativeDis if d<disMax])-1
            elif len(tmp)==4:
                rid=256*int(tmp[0].split('.')[-2])+int(tmp[0].split('.')[-1])-1
                sid=256*int(tmp[2].split(':')[-2],16)+int(tmp[2].split(':')[-1],16)-1
                if position[rid]>=0:
                    if sid<8:
                        if rid<8:
                            packets[tmp[1]][5]+=1
                    else:
                        packets[tmp[1]][5]+=1
                    if rid<8:
                        if sid<8:
                            throughput[rid]+=1
                    else:
                        throughput[rid]+=1 
                if packets[tmp[1]][2]==1:
                    ind=3
                else:
                    if packets[tmp[1]][0]==0:
                        ind=0
                    elif packets[tmp[1]][0]<8:
                        ind=1
                    else:
                        ind=2
                delay[ind].append(int(tmp[3])-packets[tmp[1]][1])
                if abs(position[sid]-position[rid])>dismax and position[sid]>=0 and position[rid]>=0:
                    dismax=abs(position[sid]-position[rid])

    for i in packets:
        if packets[i][2]==1:
            ind=3
        else:
            if packets[i][0]==0:
                ind=0
            elif packets[i][0]<8:
                ind=1
            else:
                ind=2
        if packets[i][4]>0:
            if packets[i][0]<8:
                receive[ind].append(packets[i][5]/7)
            else:
                receive[ind].append(packets[i][5]/packets[i][4])
    throughput=throughput/(600-start)
    leaderDelay=sum(delay[0])/len(delay[0])
    followerDelay=sum(delay[1])/len(delay[1])
    coreDelay=sum(delay[0]+delay[1])/len(delay[0]+delay[1])
    middleDelay=sum(delay[2])/len(delay[2])
    outterDelay=sum(delay[3])/len(delay[3])
    leaderRec=sum(receive[0])/len(receive[0])
    followerRec=sum(receive[1])/len(receive[1])
    coreRec=sum(receive[0]+receive[1])/len(receive[0]+receive[1])
    middleRec=sum(receive[2])/len(receive[2])
    outterRec=sum(receive[3])/len(receive[3])
    coreThroughput=sum(throughput[:8])/len(throughput[:8])
    hdvThroughput=sum(throughput[8:])/len(throughput[8:])
    return [coreDelay,middleDelay,outterDelay,coreRec,middleRec,outterRec,coreThroughput,hdvThroughput,dismax]

if __name__ == "__main__":
    index=sys.argv[1]
    result = analysis("output_tmp",index)
    with open("result_tmp.txt",'a') as f:
        f.write(str(index)+'\n')
        for r in result:
            f.write(str(r)+',')
        f.write('\n')