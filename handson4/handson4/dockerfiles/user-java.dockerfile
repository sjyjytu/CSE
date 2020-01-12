FROM weaveworksdemos/user:0.4.4

USER root

RUN apk update \
     && apk add openjdk8-jre

USER myuser
