#!/bin/bash

ZK_ROOT='/Users/sagi/Code/zookeeper-3.3.5'
MONGODB_ROOT='/Users/sagi/Code/mongodb-osx-x86_64-2.0.7'

# start zookeeper server
cd $ZK_ROOT
sh bin/zkServer.sh start
echo "start ZooKeeper OK. $ZK_ROOT"
cd -

# start mongodb server
cd $MONGODB_ROOT
nohup bin/mongod --dbpath ./data &
echo "start MongoDB OK. $MONGODB_ROOT"
cd -

echo "prepare enverionments OK."
