import sys
from worker import run_analysis

def computeDistance(a,b):
    a1=a.split(':')
    b1=b.split(':')
    r=0.0
    if a=="999999:0:0" or b=="999999:0:0":
        return -1
    for i in range(3):
        r+=(float(a1[i])-float(b1[i]))**2
    return r**0.5

def computeRelativeDis(sid,posList):
    r=[]
    for i in range(len(posList)):
        r.append(computeDistance(posList[sid],posList[i]))
    return r

def analysis(file,nnode):
    position=["999999:0:0"]*nnode
    outter=list(range(8,nnode))
    packets={}
    delay=[[],[],[],[]]
    receive=[[],[],[],[]]
    with open(file) as a:
        for i in a:
            if 'position' in i:
                position[int(i.split(' ')[0])]=i.split(' ')[2]
            if 'middle' in i:
                if 'add' in i:
                    outter.remove(int(i.split(' ')[0]))
                elif 'quit' in i:
                    outter.append(int(i.split(' ')[0]))
            tmp=i.split(',')
            if len(tmp)<3:
                continue
            if tmp[1] not in packets:
                    packets[tmp[1]]=[0,0,0,'',0,0]
            if len(tmp)==3:
                sid=int(tmp[0].split(':')[-1],16)-1
                packets[tmp[1]][0]=sid
                packets[tmp[1]][1]=int(tmp[2])
                if sid in outter:
                    packets[tmp[1]][2]=1
                packets[tmp[1]][3]=position[sid]
                relativeDis=computeRelativeDis(sid,position)
                packets[tmp[1]][4]=len([d for d in relativeDis if d<disMax and d>=0])-1
            elif len(tmp)==4:
                rid=int(tmp[0].split('.')[-1])-1
                sid=int(tmp[2].split(':')[-1],16)-1
                packets[tmp[1]][5]+=1
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
            receive[ind].append(packets[i][5]/packets[i][4])
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
    return [coreDelay,middleDelay,outterDelay,coreRec,middleRec,outterRec]

if __name__ == "__main__":
    index=sys.argv[1]
    f=open("../mobility/fcd-trace-"+str(index)+".ns2.output.xml.node_start_time.txt")
    nnode=len([i for i in f.read().split('\n') if i!=''])
    f.close()
    result = analysis("../output/stdout_"+str(index)+".log",nnode)
    with open("result.txt",'a') as f:
        f.write(','.join(result)+'\n')