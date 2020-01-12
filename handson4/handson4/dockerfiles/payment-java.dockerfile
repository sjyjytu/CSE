FROM weaveworksdemos/payment:0.4.3

USER root

RUN apk update \
     && apk add openjdk8-jre

USER myuser
