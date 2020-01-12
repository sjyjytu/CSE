FROM dplsming/sockshop-catalogue:0.1
USER root
WORKDIR /usr/src/app
COPY jywatcher.jar .
COPY catalogue.sh .
RUN chmod +x ./catalogue.sh
ENTRYPOINT ["/bin/sh", "./catalogue.sh"]