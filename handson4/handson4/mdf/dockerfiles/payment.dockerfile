FROM dplsming/sockshop-payment:0.1
USER root
WORKDIR /usr/src/app
COPY ./jywatcher.jar .
COPY ./payment.sh .
RUN chmod +x ./payment.sh
ENTRYPOINT [ "/bin/sh", "./payment.sh" ]