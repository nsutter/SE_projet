# SE_projet

##hash0

➜  SE_projet git:(master) ✗ time ./hash.sh
./hash.sh  115.27s user 1188.78s system 105% cpu 20:41.50 total

➜  SE_projet git:(master) ✗ sort hash.log | uniq -c > hash_u.log
➜  SE_projet git:(master) ✗ wc -l hash.log
479828 hash.log
➜  SE_projet git:(master) ✗ wc -l hash_u.log
2353 hash_u.log


##hash1

➜  SE_projet git:(master) ✗ time ./hash.sh
./hash.sh  123.39s user 1283.95s system 104% cpu 22:24.87 total

➜  SE_projet git:(master) ✗ sort hash.log | uniq -c > hash_u.log
➜  SE_projet git:(master) ✗ wc -l hash.log
479828 hash.log
➜  SE_projet git:(master) ✗ wc -l hash_u.log
18263 hash_u.log

##hash2

➜  SE_projet git:(master) ✗ time ./hash.sh
./hash.sh  125.90s user 1283.98s system 104% cpu 22:27.39 total

➜  SE_projet git:(master) ✗ sort hash.log | uniq -c > hash_u.log
➜  SE_projet git:(master) ✗ wc -l hash.log                      
479828 hash.log
➜  SE_projet git:(master) ✗ wc -l hash_u.log                    
198597 hash_u.log
