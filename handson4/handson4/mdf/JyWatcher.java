import com.alibaba.fastjson.JSONObject;
import java.util.Map;
import org.apache.curator.RetryPolicy;
import org.apache.curator.framework.CuratorFramework;
import org.apache.curator.framework.CuratorFrameworkFactory;
import org.apache.curator.framework.recipes.cache.NodeCache;
import org.apache.curator.framework.recipes.cache.NodeCacheListener;
import org.apache.curator.retry.ExponentialBackoffRetry;
import java.io.*;

//暂时还不支持退出的时候把自己remove掉
public class JyWatcher {

  /** session超时时间 */
  static final int SESSION_OUTTIME = 30000;//ms

  public static void main(String[] args) throws Exception {
    if (args.length != 4) {
      System.err
          .println("USAGE: JyWatcher hostPort znode myNames hostfile");
      System.exit(2);
    }
    String zkpPath = args[0];  // "47.103.0.246:2181"
    String nodePath = args[1];  // "/jy"
    String myNames = args[2];  //自己的名字（DNS
    String []dnsNames = myNames.split(",");
    final String hostfile = args[3];  // "/etc/hosts" （DNS文件
    //1 重试策略：初次间隔时间为1s ，重试10次
    RetryPolicy retryPolicy = new ExponentialBackoffRetry(1000, 10);
    //2 通过工厂创建连接
    CuratorFramework cf = CuratorFrameworkFactory.builder()
        .connectString(zkpPath)
        .sessionTimeoutMs(SESSION_OUTTIME)
        .retryPolicy(retryPolicy)
        .build();

    //3 建立连接
    cf.start();

    //4 建立一个cache缓存
    // NodeCache： 第3个参数：是否压缩。与PathChildrenCache类的第3个参数不同
    final NodeCache cache = new NodeCache(cf, nodePath, false);
    cache.start(true);

    cache.getListenable().addListener(new NodeCacheListener() {
      /**
       * 重写nodeChanged() 方法
       */
      @Override
      public void nodeChanged() throws Exception {
        System.out.println("path: " + cache.getCurrentData().getPath());
        System.out.println("old data: " + new String(cache.getCurrentData().getData()));
        System.out.println("state: " + cache.getCurrentData().getStat());
        System.out.println("---------------------------------------");
        //把最新内容写到hostfile
        try {
          File file = new File(hostfile); // 相对路径，如果没有则要建立一个新的output.txt文件
          file.createNewFile(); // 创建新文件,有同名的文件的话直接覆盖
          try (FileWriter writer = new FileWriter(file);
              BufferedWriter out = new BufferedWriter(writer)
          ) {
            String newContent = new String(cache.getCurrentData().getData());
            JSONObject jsonObject = new JSONObject();
            jsonObject = JSONObject.parseObject(newContent);
            for(Map.Entry<String, Object> entry : jsonObject.entrySet()) {
              out.write(entry.getValue()+"\t"+entry.getKey()+"\n");
            }
            out.flush(); // 把缓存区内容压入文件
          }
        } catch (IOException e) {
          e.printStackTrace();
        }
      }
    });
    System.out.println("connect ok!");

    //如果不存在node，则创建node
    if (cf.checkExists().forPath(nodePath)==null)
    {
      System.out.println("init node!");
      cf.create().forPath(nodePath, "".getBytes());
    }

    //起来的时候把自己加上，先去读取hostfile，然后新增到zookeeper中
    //先获取zookeeper中已有的jsonObject，没有则新建
    JSONObject jsonObject;
    if (cache.getCurrentData()==null || cache.getCurrentData().getData()==null)
    {
      //第一个吃螃蟹的人
      jsonObject = new JSONObject();
    }else {
      String oldContent = new String(cache.getCurrentData().getData());
      if (oldContent.equals("")) {
        jsonObject = new JSONObject();
      } else {
        jsonObject = JSONObject.parseObject(oldContent);
      }
    }

    //读取自己的hostfile，并加入到jsonObject中
    //hostfile应该长这样
    //10.0.0.1 app1
    //10.0.0.2 app2
    File file2 = new File(hostfile);
    BufferedReader reader = null;
    try {
      System.out.println("以行为单位读取文件内容，一次读一整行：");
      reader = new BufferedReader(new FileReader(file2));
      String tempString = null;
      // 一次读入一行，直到读入null为文件结束
      while ((tempString = reader.readLine()) != null) {
        // 显示行号
        String[] kv = tempString.split("\t");
        if (kv.length > 0 &&kv[0].split("\\.").length==4 && !kv[1].equals("localhost"))
        {
          for (String dnsName : dnsNames) {
            jsonObject.put(dnsName, kv[0]);
          }
        }
      }
      reader.close();
    } catch (IOException e) {
      e.printStackTrace();
    } finally {
      if (reader != null) {
        try {
          reader.close();
        } catch (IOException e1) {
        }
      }
    }

    //修改到zookeeper
    cf.setData().forPath(nodePath, jsonObject.toString().getBytes());
    System.out.println("init myself ok!");
    while (true)
    {
      Thread.sleep(Integer.MAX_VALUE);
    }
  }
}
