#!/bin/bash

FUSIONFS_HOME="/home/dongfang/fusionFS/fusion_mount"
TMP_FILE="xdfz_file"
TMP_DIR="xdfz_dir"

echo 
echo ==================================
echo ===== start testing FusionFS =====
echo ==================================

echo 
echo ===== read member list =====
FILENAME='src/zht/neighbor'
while read LINE
do
	newaddr=$(echo $LINE | awk '{print $1}')
	addrlist=("${addrlist[@]}" "$newaddr")
done < $FILENAME
echo "${#addrlist[@]}" neighbors: "${addrlist[@]}"

firstnode=${addrlist[0]}
lastnode=${addrlist[$((${#addrlist[@]}-1))]}

echo
echo ===== create empty temp file from 1st node =====
ssh $firstnode "cd $FUSIONFS_HOME; rm -r *; touch $TMP_FILE; ls"

echo
echo ===== check all nodes can see the temp file =====
for node in $addrlist
do
	echo -n "$node: "	
	ssh $node "cd $FUSIONFS_HOME; ls"
	echo
done

echo
echo ===== modify the temp file on the last node =====
echo -n "$TMP_FILE@$lastnode: "
ssh $lastnode "cd $FUSIONFS_HOME; echo '$TMP_FILE updated by $lastnode' > $TMP_FILE; cat $TMP_FILE"

echo
echo ===== check all nodes can see the updates =====
for node in $addrlist
do
	echo -n "$TMP_FILE@$node: "
	ssh $node "cd $FUSIONFS_HOME; cat $TMP_FILE"	
	echo
done

echo 
echo ===== remove the temp file on the first node =====
ssh $firstnode "cd $FUSIONFS_HOME; rm $TMP_FILE; ls"

echo
echo ===== check all nodes have the temp file removed =====
for node in $addrlist
do
	echo -n "$node (you should see nothing): "
	ssh $node "cd $FUSIONFS_HOME; ls"
	echo
done

echo
echo ===== create a temp directory on the first node =====
ssh $firstnode "cd $FUSIONFS_HOME; mkdir $TMP_DIR; ls"

echo
echo ===== check all nodes can see the new directory =====
for node in $addrlist
do
	echo -n "$node: "
	ssh $node "cd $FUSIONFS_HOME; ls"
	echo
done

echo
echo ===== remove the temp directory from the last node =====
ssh $lastnode "cd $FUSIONFS_HOME; rmdir $TMP_DIR"

echo
echo ===== check all nodes can see the new directory =====
for node in $addrlist
do
	echo -n "$node (you should see nothing): "
	ssh $node "cd $FUSIONFS_HOME; ls"
	echo
done

echo
echo ==================================
echo ===== FusionFS test complete =====
echo ==================================
echo
