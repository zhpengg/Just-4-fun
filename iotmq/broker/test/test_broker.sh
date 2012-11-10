#!/bin/bash

REST_SERVER='http://localhost:12321/message/test-topic'
COUNTER=0

cat ./hosts | while read LINE;
do
    curl $REST_SERVER -d data="$LINE"
    echo "send message $COUNTER: $LINE"
    let COUNTER++
done
