FROM ubuntu:20.04
RUN apt-get update
RUN apt install -y python3-pip
COPY requirements.txt .
RUN apt-get install -y curl 
RUN curl https://packages.microsoft.com/keys/microsoft.asc | apt-key add -
RUN curl https://packages.microsoft.com/config/ubuntu/20.04/prod.list | tee /etc/apt/sources.list.d/msprod.list
RUN apt-get update
RUN ACCEPT_EULA=Y apt-get install -y msodbcsql17
RUN ACCEPT_EULA=Y apt-get install -y mssql-tools
RUN echo 'export PATH="$PATH:/opt/mssql-tools/bin"' >> ~/.bash_profile
RUN echo 'export PATH="$PATH:/opt/mssql-tools/bin"' >> ~/.bashrc
RUN apt-get install -y unixodbc-dev
RUN python3 -m pip install --no-cache-dir -r requirements.txt
RUN apt-get install libgl1 -y
#apt-get install libglib2.0-0 -y
COPY . /rfid_access
WORKDIR rfid_access
CMD /opt/mssql-tools/bin/sqlcmd -S localhost,1415 -U SA -P "7u8i9o0P" -i initDatabase.sql && /bin/bash
# export FLASK_APP=project && export FLASK_DEBUG=1
# python3 create_db.py && flask run