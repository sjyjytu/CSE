Question 1: Please briefly answer the role of DNS in your own words.
**DNS，域名系统，就是将域名和IP建立相互映射，使得人们可以用域名来访问网页而不是记复杂的IP地址，更方便了。**



Question 2: The type field have a few different values to indicate the kind of this record. What do "A", "NS" and "CNAME" mean?

**A: 主机地址资源记录，将DNS域名映射到IPv4的32位地址中
NS: Name Server，将owner中指定的DNS域名映射到name_server_domain_name字段中指定的运行DNS服务器的主机名
CNAME：规范名资源记录，将owner字段中的别名或备用的DNS域名映射到canonical_name字段中指定的标准或主要DNS域名。**



Question 3: How can we ask a specific dns server (instead of the default) for information about a domain name? When I use "dig www.baidu.com",the DNS server is 192.168.110.2. However if this server crashed and I have to ask the server 8.8.8.8, what command should I use?

**用命令dig @8.8.8.8 www.baidu.com**



Question 4: Do you know the process of solving domain name "lirone.csail.mit.edu"? You need to go through the steps of resolving a particular hostname, mimicing a standard recursive query. Assuming it knows nothing else about a name, a DNS resolver will ask a well-known root server. The root servers on the Internet are in the domain root-servers.net. You can use "%dig . ns" to get the list of 13 root servers.You can show us the result of each step or briefly introduce your idea.[Hint: you should start from "edu"]

**首先我用了dig lirone.csail.mit.edu +trace，结果如下：**

![1571813678575](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\1571813678575.png)

![1571813707672](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\1571813707672.png)

**由此可以看出，首先到找到根 . 的服务器100.100.2.136，然后找到edu的服务器202.12.27.33，然后找到mit.edu的192.35.51.30，然后是csail.mit.edu的95.101.36.64，最后找到lirone.csail.mit.edu对应的服务器ip128.30.2.123**



Question 5: Please explain the above phenomenon. Have a guess!

**dig www.baidu.com +trace**

![1571814557650](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\1571814557650.png)

![1571814571186](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\1571814571186.png)

**dig www.twitter.com +trace**

![1571814602363](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\1571814602363.png)

![1571814615194](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\1571814615194.png)

**发现在baidu.com和twitter.com这一步右边都是没有问题的（用whois查了一下都是对的），但是在www.baidu.com这一步百度会返回很多地址；而twitter后就只返回一个ip地址69.171.234.18,又用ip查询发现他是一个来自美国俄勒冈州普赖恩维尔的地址。但是我经过多次测试后发现，每次他都返回一个不一样的地址，有美国的，有爱尔兰的。。。猜测他是返回一个假的ip给我**

多次dig www.twitter.com +trace的返回结果

- 69.171.248.112
- 69.171.247.20
- 31.13.74.1
- 31.13.72.54

**dig www.twitter.com @1.0.0.0**

![1571816473929](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\1571816473929.png)

**dig www.baidu.com @1.0.0.0**

![1571816521090](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\1571816521090.png)

**ping 1.0.0.0发现并不存在，再在一台日本的服务器上dig twitter,发现真正的地址应该是**

- 104.244.42.129
- 104.244.42.65

![1571816997865](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\1571816997865.png)

**之类的。。。实锤了。**



Question 6: The ips which dig returns to you belong to google indeed. Give the reason for the above phenomenon.

ip是正确的却连不上谷歌，可能是发包的时候经过路由器网关发到国外时，数据包被破坏了。