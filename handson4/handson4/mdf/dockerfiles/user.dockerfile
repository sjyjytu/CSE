FROM dplsming/sockshop-user:0.1
USER root
WORKDIR /usr/src/app
COPY ./jywatcher.jar .
COPY ./user.sh .
RUN chmod +x ./user.sh
ENTRYPOINT [ "/bin/sh", "./user.sh" ]