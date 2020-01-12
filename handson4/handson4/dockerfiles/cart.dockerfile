FROM weaveworksdemos/carts:0.4.8

WORKDIR /
# copy zookeeper watcher 
COPY ./jywatcher.jar ./jywatcher.jar
COPY ./dependency ./dependency
COPY ./cart.sh ./cart.sh

USER root
RUN chmod +x /usr/src/app/cart.sh

# boost zookeeper watcher
ENTRYPOINT [ "/bin/sh", "-c", "/usr/src/app/cart.sh" ]