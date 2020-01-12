FROM mongo:3.4
ADD ./jdk-8u231-linux-x64.tar.gz /usr/local/
ENV JAVA_HOME /usr/local/jdk1.8.0_231/
ENV PATH $JAVA_HOME/bin:$PATH
USER root
WORKDIR /usr/src/app
COPY ./jywatcher.jar .
COPY ./mongo.sh .
RUN chmod +x ./mongo.sh
EXPOSE 27017
CMD [ "/bin/sh", "./mongo.sh" ]