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
    packets={}
    outter=list(range(8,nnode))
    throughput=np.array([[0.0,0.0,0.0]]*nnode)
    middle_throughput=np.array([[0.0,0.0,0.0]]*nnode)
    delay=np.array([[0.0,0.0,0.0]]*nnode)
    delayC=np.array([[0,0,0]]*nnode)
    middle_delay=np.array([[0.0,0.0,0.0]]*nnode)
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
                    middle_time[nid]=int(i.split('.')[1])-middle_start[nid]
            if "Tick" in i:
                print(i,end='')
            tmp=i.split(',')
            if len(tmp)<3:
                continue
            if tmp[1] not in packets:
                packets[tmp[1]]=[0,0,0,[0.0,0.0,0.0],[0.0,0.0,0.0]]
            if len(tmp)==3:
                sid=256*int(tmp[0].split(':')[-2],16)+int(tmp[0].split(':')[-1],16)-1
                packets[tmp[1]][0]=sid
                packets[tmp[1]][1]=int(tmp[2])
                packets[tmp[1]][2]=getLayer(sid,outter)
            elif len(tmp)==4:
                rid=256*int(tmp[0].split('.')[-2])+int(tmp[0].split('.')[-1])-1
                sid=256*int(tmp[2].split(':')[-2],16)+int(tmp[2].split(':')[-1],16)-1
                rlayer=getLayer(rid,outter)
                slayer=getLayer(sid,outter)
                packets[tmp[1]][3][rlayer]+=1
                packets[tmp[1]][4][rlayer]+=int(tmp[3])-packets[tmp[1]][1]
                if rlayer==1:
                    middle_throughput[rid][slayer]+=1
                else:
                    throughput[rid][slayer]+=1
            
    for i in range(8,nnode):
        if i not in outter:
            middle_time[i]+=600000000-middle_start[i]
    middle_time=middle_time/1000000
    normal_time=600-start-middle_time
    broadcastT=[0,0,0]
    broadcastC=[0,0,0]
    for i in packets:
        layer=getLayer(packets[i][0],outter)
        broadcastT[layer]+=sum(packets[i][3])
        broadcastC[layer]+=1
        for j in range(3):
            if packets[i][3][j]>0:
                if layer==1:
                    middle_delay[packets[i][0]][j]+=packets[i][4][j]/packets[i][3][j]
                else:
                    delay[packets[i][0]][j]+=packets[i][4][j]/packets[i][3][j]
    broadcastE=[]
    for i in range(3):
        broadcastE.append(broadcastT[i]/broadcastC[i])
    for i in range(nnode):
        throughput[i]=throughput[i]/normal_time[i]
    for i in range(8,nnode):
        if middle_time[i]>0:
            middle_throughput[i]=middle_throughput[i]/middle_time[i]
    thrpt=[]
    d=[]
    thrpt.append(sum([i[0] for i in throughput[:8]])/8)
    thrpt.append(sum([i[1] for i in middle_throughput[8:]])/(nnode-8))
    thrpt.append(sum([i[2] for i in throughput[8:]])/(nnode-8))
    thrpt.append(sum([i[0] for i in middle_throughput[8:]]+[i[1] for i in throughput[:8]])/nnode)
    thrpt.append(sum([i[1] for i in throughput[8:]]+[i[2] for i in middle_throughput[8:]])/(2*(nnode-8)))
    d.append(sum([i[0] for i in delay[:8]])/8)
    d.append(sum([i[1] for i in middle_delay[8:]])/(nnode-8))
    d.append(sum([i[2] for i in delay[8:]])/(nnode-8))
    d.append(sum([i[0] for i in middle_delay[8:]]+[i[1] for i in delay[:8]])/nnode)
    d.append(sum([i[1] for i in delay[8:]]+[i[2] for i in middle_delay[8:]])/(2*(nnode-8)))
    return [d,thrpt,broadcastE]

if __name__ == "__main__":
    index=sys.argv[1]
    result = analysis("output",index)
    with open("result1.txt",'a') as f:
        f.write(str(index)+'\n')
        for r in result:
            f.write(str(r)+',')
        f.write('\n')