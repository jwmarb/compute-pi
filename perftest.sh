rm logs/*

a=1
for i in {1..5};
do
  b=1
  for j in {1..5};
  do
    ./run.sh $a $b
    b=$((b * 2))
  done
  a=$((a * 2))
done

a=1
for i in {1..5};
do
  ./run.sh 32 $a
  a=$((a * 2))
done

a=1
for i in {1..5};
do
  ./run.sh 64 $a
  a=$((a * 2))
done