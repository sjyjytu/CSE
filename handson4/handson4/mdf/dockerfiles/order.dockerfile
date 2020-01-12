FROM weaveworksdemos/orders:0.4.7
USER root
WORKDIR /usr/src/app
COPY ./jywatcher.jar .
COPY ./order.sh .
RUN chmod +x ./order.sh
ENTRYPOINT [ "/bin/sh", "./order.sh" ]