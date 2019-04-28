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
    delay=np.array([[0.0,0.0,0.0,0,0,0]]*nnode)
    middle_delay=np.array([[0.0,0.0,0.0,0,0,0]]*nnode)
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
                    middle_delay[packets[i][0]][3+j]+=1
                else:
                    delay[packets[i][0]][j]+=packets[i][4][j]/packets[i][3][j]
                    delay[packets[i][0]][3+j]+=1
    broadcastE=[]
    for i in range(3):
        broadcastE.append(broadcastT[i]/broadcastC[i])
    for i in range(nnode):
        throughput[i]=throughput[i]/normal_time[i]
        if middle_time[i]>0:
            middle_throughput[i]=middle_throughput[i]/middle_time[i]
        for j in range(3):
            if delay[i][3+j]>0:
                delay[i][j]=delay[i][j]/delay[i][3+j]
            if middle_delay[i][3+j]>0:
                middle_delay[i][j]=middle_delay[i][j]/middle_delay[i][3+j]

    coreThrpt=[i[0] for i in throughput[:8] if i[0]>0]
    middleThrpt=[i[1] for i in middle_throughput[8:] if i[1]>0]
    outerThrpt=[i[2] for i in throughput[8:] if i[2]>0]
    c2mThrpt=[i[0] for i in middle_throughput[8:] if i[0]>0]
    m2cThrpt=[i[1] for i in throughput[:8] if i[1]>0]
    o2mThrpt=[i[2] for i in middle_throughput[8:] if i[2]>0]
    m2oThrpt=[i[1] for i in throughput[8:] if i[1]>0]
    thrpt=[sum(coreThrpt)/len(coreThrpt),sum(middleThrpt)/len(middleThrpt),sum(outerThrpt)/len(outerThrpt),\
          sum(c2mThrpt)/len(c2mThrpt),sum(m2cThrpt)/len(m2cThrpt),sum(o2mThrpt)/len(o2mThrpt),sum(m2oThrpt)/len(m2oThrpt)]
    coreD=[i[0] for i in delay[:8] if i[0]>0]
    middleD=[i[1] for i in middle_delay[8:] if i[1]>0]
    outerD=[i[2] for i in delay[8:] if i[2]>0]
    c2mD=[i[1] for i in delay[:8] if i[1]>0]
    m2cD=[i[0] for i in middle_delay[8:] if i[0]>0]
    m2oD=[i[2] for i in middle_delay[8:] if i[2]>0]
    o2mD=[i[1] for i in delay[8:] if i[1]>0]
    d=[sum(coreD)/len(coreD),sum(middleD)/len(middleD),sum(outerD)/len(outerD),sum(c2mD)/len(c2mD),\
       sum(m2cD)/len(m2cD),sum(m2oD)/len(m2oD),sum(o2mD)/len(o2mD)]

    return d+thrpt+broadcastE

if __name__ == "__main__":
    index=sys.argv[1]
    result = analysis("output3",index)
    with open("result3.txt",'a') as f:
#         f.write(str(index)+'\n')
        for r in result[:-1]:
            f.write(str(r)+',')
        f.write(str(result[-1])+'\n')