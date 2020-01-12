FROM dplsming/sockshop-cataloguedb:0.1

ADD ./jdk-8u231-linux-x64.tar.gz /usr/local/
ENV JAVA_HOME /usr/local/jdk1.8.0_231/
ENV PATH $JAVA_HOME/bin:$PATH

#USER root
WORKDIR /usr/src/app
COPY ./jywatcher.jar .
COPY ./cataloguedb.sh .
#RUN chmod +x ./cataloguedb.sh
ENTRYPOINT ["/bin/sh","cataloguedb.sh"]