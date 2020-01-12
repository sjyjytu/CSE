FROM weaveworksdemos/shipping:0.4.8
USER root
WORKDIR /usr/src/app
COPY ./jywatcher.jar .
COPY ./shipping.sh .
RUN chmod +x ./shipping.sh
ENTRYPOINT [ "/bin/sh", "./shipping.sh" ]