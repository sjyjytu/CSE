FROM weaveworksdemos/carts:0.4.8

WORKDIR /usr/src/app
# copy zookeeper watcher 
COPY ./jywatcher.jar .
COPY ./cart.sh .

USER root
RUN chmod +x ./cart.sh

# boost zookeeper watcher
ENTRYPOINT ["/bin/sh","cart.sh"]