// KCP reliable UDP client demo
var kcpclient;

joy.load = function()
{
    kcpclient = get_env("kcpclient");
    if (!kcpclient) {
        console.log("kcp_client test");
        kcpclient = network.kcpclient.create("127.0.0.1", 30000);
    }
};

joy.update = function(dt)
{
    if (!kcpclient) return;
    network.kcpclient.update(kcpclient);
    var result, msg = network.kcpclient.poll(kcpclient);
    if (result) {
        console.log(msg.type + ":" + msg.conv);
        if (msg.type == 1) {
            network.kcpclient.send(kcpclient, "hello from client");
        } else if (msg.type == 3) {
            console.log(msg.type + ":" + msg.conv + ":" + msg.data);
        }
    }
};

joy.draw = function() {};

joy.hotfix = function()
{
    if (kcpclient) set_env("kcpclient", kcpclient);
};

joy.quit = function()
{
    if (kcpclient) network.kcpclient.destroy(kcpclient);
};
