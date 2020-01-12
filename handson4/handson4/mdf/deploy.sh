sudo docker run -d --network dockerfiles_mynet --name cardb -e NAME=carts-db jy/mongo:0.1
sudo docker run -d --network dockerfiles_mynet --name orddb -e NAME=orders-db jy/mongo:0.1
sudo docker run -d --network dockerfiles_mynet --name usedb -e NAME=user-db jy/mongo:0.1
sudo docker run -d --network dockerfiles_mynet --name catdb -e NAME=catalogue-db jy/cataloguedb:0.1
sudo docker run -d --network dockerfiles_mynet --name ship -e NAME=shipping jy/shipping:0.1
sudo docker run -d --network dockerfiles_mynet --name user -e NAME=user -e MONGO_HOST=user-db:27017 jy/user:0.1
sudo docker run -d --network dockerfiles_mynet --name cart -e NAME=carts jy/cart:0.1
sudo docker run -d --network dockerfiles_mynet --name paym -e NAME=payment jy/payment:0.1
sudo docker run -d --network dockerfiles_mynet --name orde -e NAME=orders jy/order:0.1
sudo docker run -d --network dockerfiles_mynet --name cata -e NAME=catalogue jy/catalogue:0.1
sudo docker run -d --network dockerfiles_mynet --name fron -e NAME=frontend -p 8079:8079 jy/frontend:0.1