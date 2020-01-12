FROM dplsming/sockshop-frontend:0.1

USER root

RUN apk update \
     && apk add openjdk8-jre

USER myuser
