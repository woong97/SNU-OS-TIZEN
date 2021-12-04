### ext2_set_gps_location
- fs/attr.c notify_chane : 파일이 수정될때 location 기록
- fs/ext2/namei.c : 파일이 생성될 때 location 기록

-fs/ext2/ialloc.c ext2_new_inode do we need?
-fs/ext2/inode.c setsize do we need?


### Operations
- Arithmetic
- Taylor series
 
![image](https://user-images.githubusercontent.com/60849888/144718566-ccec63d2-046a-4b62-bf98-6d0dc923b4dc.png)



### haversine distance
- ref: https://en.wikipedia.org/wiki/Haversine_formula
- ref: python haversine package function
```C
lat1, lng1 = point1
lat2, lng2 = point2

# convert all latitudes/longitudes from decimal degrees to radians
lat1 = radians(lat1)
lng1 = radians(lng1)
lat2 = radians(lat2)
lng2 = radians(lng2)

# calculate haversine
lat = lat2 - lat1
lng = lng2 - lng1
harv_d = sin(lat * 0.5) ** 2 + cos(lat1) * cos(lat2) * sin(lng * 0.5) ** 2

return 2 * get_avg_earth_radius(unit) * asin(sqrt(harv_d))
```

![image](https://user-images.githubusercontent.com/60849888/144707258-2aab5d37-07ef-418e-8325-22c91e2e9846.png)

