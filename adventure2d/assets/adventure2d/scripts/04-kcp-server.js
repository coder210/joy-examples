// KCP reliable UDP server demo
var kcpserver;

joy.load = function()
{
    kcpserver = get_env("kcpserver");
    if (!kcpserver) {
        console.log("kcp_server test");
        kcpserver = network.kcpserver.create("127.0.0.1", 30000);
    }
};

joy.update = function(dt)
{
    if (!kcpserver) return;
    network.kcpserver.update(kcpserver);
    var result, msg = network.kcpserver.poll(kcpserver);
    if (result) {
        console.log(msg.type + ":" + msg.conv);
        if (msg.type == 3) {
            console.log(msg.type + ":" + msg.conv + ":" + msg.data);
            network.kcpserver.send(kcpserver, msg.conv, "hello from server");
        }
    }
};

joy.draw = function() {};

joy.hotfix = function()
{
    if (kcpserver) set_env("kcpserver", kcpserver);
};

joy.quit = function()
{
    if (kcpserver) network.kcpserver.destroy(kcpserver);
};
