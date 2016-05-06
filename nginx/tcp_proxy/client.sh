for i in {1..30}
do
    (echo "www"|nc 127.0.0.1 12345 -i 1)&
done

