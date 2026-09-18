# 独立 HTTP QML 应用站

本目录可单独复制部署，不依赖 Q-Browser 源码或 Qt。Python 3.10+，无第三方依赖。

```powershell
E:\Python310\python.exe server.py --packages C:\QBrowserQmlDemos\release-deploy\packages --port 18880
```

普通浏览器打开 http://127.0.0.1:18880/ 查看介绍。使用新版 Q-Browser（Pilot 主包、package 模式和本机信任公钥）访问：

- http://127.0.0.1:18880/demos/elisa
- http://127.0.0.1:18880/demos/tokodon
- http://127.0.0.1:18880/demos/coffee

服务仅发布三个签名包的只读快照，不发布源码目录、信任私钥或本机状态。修改包后重启服务。可传 `--bind 0.0.0.0` 在局域网发布，客户端使用服务器 IP。默认仅本机监听；这是一项前台开发服务，不自动注册 Windows 服务或修改防火墙。

## 协议 v1

访问应用 URL 时，Q-Browser 发送 `Accept: application/vnd.qbrowser.site+json`，同一 URL 返回描述 JSON。普通浏览器返回 HTML。`Vary: Accept` 区分响应。

```json
{
  "schemaVersion": 1,
  "appId": "com.qbrowser.demo.elisa",
  "route": "/demos/elisa",
  "packageUrl": "/packages/com.qbrowser.demo.elisa-1.0.0.qapkg",
  "sha256": "64 lowercase hexadecimal characters"
}
```

Host 下载同源包、校验摘要、使用本机预配公钥验签并安装，再按验证后的包启动当前标签。网站不能提供新信任公钥；摘要不代替签名。HTTP 不提供服务器身份认证，生产发布建议在反向代理后使用 HTTPS。客户端支持 HTTPS，保持系统证书校验。

首版只绑定现有三个 Demo 路由，不自动注册任意网站应用；主包需使用 Pilot。地址栏、历史和恢复保留 HTTP URL；恢复和重新加载会重新请求站点，无离线回退。普通浏览器不会直接执行 QML。

限制：描述 64 KiB、包 64 MiB、每阶段总计 30 秒、禁止重定向、禁止跨源包、禁止 URL 账号/片段/查询串。离开或关闭标签后下载取消，过期结果不能启动 Worker。已进入验签安装的任务可能完成包缓存，但不会切回旧标签。现有本机包许可和能力边界保持适用。

运行服务测试：`python -m unittest discover -s tests -v`。
