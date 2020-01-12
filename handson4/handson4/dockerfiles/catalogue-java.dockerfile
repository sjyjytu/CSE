FROM weaveworksdemos/catalogue:0.3.5

USER root

RUN apk update \
     && apk add openjdk8-jre

USER myuser
