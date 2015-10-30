egrep 'http://[^?" /]*' index.html -o |tr -d "'"| sort| uniq > test_dns
