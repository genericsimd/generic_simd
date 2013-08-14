#/bin/bash
LOG=$1_pmc.log
echo "run $1 to get pmc" | tee $LOG 
for group in {0..260}
do
./grouppower7.sh $group $1 2>&1 | tee -a $LOG 
done
