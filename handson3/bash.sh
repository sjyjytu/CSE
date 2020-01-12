sudo docker network create --subnet 172.30.0.1/16 handson3
sudo docker run -d --network handson3 --ip 172.30.0.2 mongo:3.4
sudo docker run -d --network handson3 --ip 172.30.0.3 dplsming/sockshop-cataloguedb:0.1
sudo docker run -d --network handson3 --ip 172.30.0.4 weaveworksdemos/shipping:0.4.8
sudo docker run -d --network handson3 --ip 172.30.0.5 --add-host user-db:172.30.0.2 -e MONGO_HOST=user-db:27017 weaveworksdemos/user:0.4.4
sudo docker run -d --network handson3 --ip 172.30.0.6 --add-host carts-db:172.30.0.2 weaveworksdemos/carts:0.4.8
sudo docker run -d --network handson3 --ip 172.30.0.7 weaveworksdemos/payment:0.4.3
sudo docker run -d --network handson3 --ip 172.30.0.8 --add-host orders-db:172.30.0.2 --add-host shipping:172.30.0.4 --add-host user:172.30.0.5 --add-host carts:172.30.0.6 --add-host payment:172.30.0.7 weaveworksdemos/orders:0.4.7
sudo docker run -d --network handson3 --ip 172.30.0.9 --add-host catalogue-db:172.30.0.3 weaveworksdemos/catalogue:0.3.5
sudo docker run -d --network handson3 --ip 172.30.1.0 --cpus 0.5 --add-host session-db:172.30.3.0 --add-host shipping:172.30.0.4 --add-host user:172.30.0.5 --add-host orders:172.30.0.8 --add-host carts:172.30.0.6 --add-host payment:172.30.0.7 --add-host catalogue:172.30.0.9 -e SESSION_REDIS=1 dplsming/sockshop-frontend:0.1
sudo docker run -d --network handson3 --ip 172.30.1.1 --cpus 0.5 --add-host session-db:172.30.3.0 --add-host shipping:172.30.0.4 --add-host user:172.30.0.5 --add-host orders:172.30.0.8 --add-host carts:172.30.0.6 --add-host payment:172.30.0.7 --add-host catalogue:172.30.0.9 -e SESSION_REDIS=1 dplsming/sockshop-frontend:0.1
sudo docker run -d --network handson3 --ip 172.30.1.2 --cpus 0.5 --add-host session-db:172.30.3.0 --add-host shipping:172.30.0.4 --add-host user:172.30.0.5 --add-host orders:172.30.0.8 --add-host carts:172.30.0.6 --add-host payment:172.30.0.7 --add-host catalogue:172.30.0.9 -e SESSION_REDIS=1 dplsming/sockshop-frontend:0.1
sudo docker run -d --network handson3 --ip 172.30.2.0 -v $(pwd)/conf.d:/etc/nginx/conf.d  -v  $(pwd)/frontend/public:/usr/public -p 8090:8090 nginx
sudo docker run -d --network handson3 --ip 172.30.3.0 redis:latest