#!/bin/bash

# very simple helper script to create the commands for starting several processes
# inspired from devices/aliceHLTwrapper/launchSimpleChain.sh from Matthias Richter

inputDigitsFile="/Users/laurent/o2/clustering/basicTiming/169099/MB/merged.digits.MB.196099.root" # 932 MB events
run=169099

msgSize=1000

firstSocketNo=45000
let socketNo=firstSocketNo
trackerInputSockets=

start_device () {

echo "starting $1 for specification $spec in screen \"$sessiontitle\""
# start the screen session

screen -S "$sessiontitle" -t 0 -A -d -m
# propagate the environment to the screen window
env | sed -e '/^[A-Z]/!d' -e 's|=|="|' -e 's|\(.\)$|\1"|' | while read line ; do screen -S "$sessiontitle" -p 0 -X stuff $"export ${line}$(printf \\r)"; done

# change to the directory
screen -S "$sessiontitle" -p 0 -X stuff $"cd ${PWD}$(printf \\r)"
# start the device
# Note: there seems to be a limit how many characters are sent
# to the screen with the -X option. Probably one has to break down the
# socket configuration string
echo $command $sockets $parameters
screen -S "$sessiontitle" -p 0 -X stuff $"${command} "
screen -S "$sessiontitle" -p 0 -X stuff $"${sockets} "
screen -S "$sessiontitle" -p 0 -X stuff $"${parameters}$(printf \\r)"

}

spec=0x0
sessiontitle="MUONDigitReader_0"
command="aliceHLTwrapper $sessiontitle 1 --output type=push,size=$msgSize,method=bind,address=tcp://*:$socketNo --library libAliHLTMUON.so --component MUONDigitReader --parameter '-datafile $inputDigitsFile'"
start_device MUONDigitReader

outSocket=$((socketNo + 1))

sessiontitle="MUONPreclusterFinder_0"
command="aliceHLTwrapper $sessiontitle 1 --input type=pull,method=connect,size=1000,address=tcp://127.0.0.1:$socketNo --output type=push,method=connect,size=1000,address=tcp://localhost:$outSocket --library libAliHLTMUON.so --component MUONPreclusterFinder --parameter '-cdbpath local://$ALICE_ROOT/OCDB -run $run'"
start_device MUONPreclusterFinder

echo
screen -ls

sessiontitle="MUONClusterWriter"
command="aliceHLTwrapper $sessiontitle 1 --input type=pull,method=bind,size=1000,address=tcp://*:$outSocket --library libAliHLTMUON.so --component MUONClusterWriter --parameter '-datafile MUON.RecPoints.root'"

echo $command
#start_device MUONPreclusterFinder
