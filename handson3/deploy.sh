sudo docker network create --subnet 172.30.0.1/16 handson3
sudo docker run -d --network handson3 --ip 172.30.0.2 mongo:3.4
sudo docker run -d --network handson3 --ip 172.30.0.3 dplsming/sockshop-cataloguedb:0.1
sudo docker run -d --network handson3 --ip 172.30.0.4 weaveworksdemos/shipping:0.4.8
sudo docker run -d --add-host user-db:172.30.0.2 --network handson3 --ip 172.30.0.5 -e MONGO_HOST=user-db:27017 weaveworksdemos/user:0.4.4
sudo docker run -d --add-host carts-db:172.30.0.2 --network handson3 --ip 172.30.0.6 weaveworksdemos/carts:0.4.8
sudo docker run -d --network handson3 --ip 172.30.0.7 weaveworksdemos/payment:0.4.3
sudo docker run -d --add-host orders-db:172.30.0.2 --add-host shipping:172.30.0.4 --add-host user:172.30.0.5 --add-host carts:172.30.0.6 --add-host payment:172.30.0.7 --network handson3 --ip 172.30.0.8 weaveworksdemos/orders:0.4.7
sudo docker run -d --add-host catalogue-db:172.30.0.3 --network handson3 --ip 172.30.0.9 weaveworksdemos/catalogue:0.3.5
sudo docker run -d --add-host shipping:172.30.0.4 --add-host user:172.30.0.5 --add-host orders:172.30.0.8 --add-host carts:172.30.0.6 --add-host payment:172.30.0.7 --add-host catalogue:172.30.0.9 --network handson3 --ip 172.30.1.0 -p 8080:8079 dplsming/sockshop-frontend:0.1