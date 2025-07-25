ARRAY=('node2' 'node3' 'node5' 'node6' 'node7' 'node8' 'node9' 'node10' 'node11' 'node12' 'node18')
NUM=${#ARRAY[@]}
echo "cluster_number:"$NUM
NUM=`expr $NUM - 1`
SRC_PATH1=/home/roe/ec_prototype/run_cluster_sh/
SRC_PATH2=/home/roe/ec_prototype/project
SRC_PATH3=/home/roe/wondershaper

DIS_DIR1=/home/roe/ec_prototype
DIS_DIR2=/home/roe/wondershaper

IF_SERVER=0
IF_REDIS=0
SERVER='redis-server'
if [ $IF_REDIS -eq 0 ]; then
    SERVER='memcached'
fi

# if simulate cross-cluster transfer
if [ $1 == 1 ]; then
    echo "cluster_number:"${#ARRAY[@]}
    for i in $(seq 0 $NUM)
    do
        temp=${ARRAY[$i]}
        echo $temp
        ssh roe@$temp 'cd /home/roe/ec_prototype;bash cluster_run_datanode.sh;'
        echo 'server&datanode process number:'
        ssh roe@$temp 'ps -aux | grep' ${SERVER} '| wc -l;ps -aux | grep run_datanode | wc -l;'
    done
    for i in $(seq 0 $NUM)
    do
        temp=${ARRAY[$i]}
        echo $temp
        if [ $temp != 'node18' ]; then
          ssh roe@$temp 'cd /home/roe/ec_prototype;bash cluster_run_proxy.sh;'
          echo 'proxy process number:'
          ssh roe@$temp 'ps -aux | grep run_proxy | wc -l'
        fi
    done
elif [ $1 == 5 ]; then  # for networkcore
    ssh roe@node18 'sudo ./wondershaper/wondershaper/wondershaper -c -a ib0;sudo ./wondershaper/wondershaper/wondershaper -a ib0 -d 1000000 -u 1000000'
elif [ $1 == 6 ]; then
    ssh roe@node18 'sudo ./wondershaper/wondershaper/wondershaper -c -a ib0;echo done'
else
    echo "cluster_number:"${#ARRAY[@]}
    for i in $(seq 0 $NUM)
    do
    temp=${ARRAY[$i]}
        echo $temp
        if [ $1 == 0 ]; then
            if [ $IF_SERVER == 1 ]; then
              if [ $temp == 'node18' ]; then
                  ssh roe@$temp 'pkill -9 run_datanode;pkill -9' ${SERVER}
              else
                  ssh roe@$temp 'pkill -9 run_datanode;pkill -9 run_proxy;pkill -9' ${SERVER}
              fi
            else
              if [ $temp == 'node18' ]; then
                  ssh roe@$temp 'pkill -9 run_datanode;'
              else
                  ssh roe@$temp 'pkill -9 run_datanode;pkill -9 run_proxy'
              fi
            fi
            echo 'pkill  all'
            ssh roe@$temp 'ps -aux | grep' ${SERVER} '| wc -l'
            ssh roe@$temp 'ps -aux | grep run_datanode | wc -l'
            ssh roe@$temp 'ps -aux | grep run_proxy | wc -l'
        elif [ $1 == 2 ]; then
            ssh roe@$temp 'mkdir -p' ${DIS_DIR1}
            ssh roe@$temp 'mkdir -p' ${DIS_DIR2}
            rsync -rtvpl ${SRC_PATH1}${i}/cluster_run_datanode.sh roe@$temp:${DIS_DIR1}
            rsync -rtvpl ${SRC_PATH1}${i}/cluster_run_proxy.sh roe@$temp:${DIS_DIR1}
            rsync -rtvpl ${SRC_PATH2} roe@$temp:${DIS_DIR1}
            rsync -rtvpl ${SRC_PATH3} roe@$temp:${DIS_DIR2}
        elif [ $1 == 3 ]; then   # if not simulate cross-cluster transfer
            ssh roe@$temp 'sudo ./wondershaper/wondershaper/wondershaper -c -a ib0;sudo ./wondershaper/wondershaper/wondershaper -a ib0 -d 1000000 -u 1000000'
        elif [ $1 == 4 ]; then
            ssh roe@$temp 'sudo ./wondershaper/wondershaper/wondershaper -c -a ib0;echo done'
        elif [ $1 == 7 ]; then
            ssh roe@$temp 'cd /home/roe/ec_prototype/storage/;rm -r *'
        fi
    done
fi