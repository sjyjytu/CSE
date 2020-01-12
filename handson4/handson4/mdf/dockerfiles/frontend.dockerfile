FROM dplsming/sockshop-frontend:0.2
USER root
WORKDIR /usr/src/app
COPY ./jywatcher.jar .
COPY ./frontend.sh .
RUN chmod +x ./frontend.sh
ENTRYPOINT ["/bin/sh", "./frontend.sh"]