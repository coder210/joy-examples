// msgpack JS 测试
// 演示 msgpack.pack / unpack 的基本用法
var win, ren, font;
var results = [];

function add(msg) {
    results.push(msg);
    console.log(msg);
}

joy.load = function() {
    win = get_env("msg_win");
    ren = get_env("msg_ren");
    font = get_env("msg_font");
    if (!font) {
        win = window.create("msgpack-js-test", 800, 600, 0);
        ren = r2d.create(win);
        font = r2d.create_font(ren, "joy_beat/fonts/simhei.ttf", 18);

    // ====== 测试开始 ======
    add("=== msgpack JS test ===");

    // 1. 基本类型
    var data = msgpack.pack(null, true, false, 42, -1, 3.14, "hello");
    var vals = msgpack.unpack(data);
    add("1. basic: nil=" + vals[0] + " true=" + vals[1] + " false=" + vals[2]);
    add("   42=" + vals[3] + " -1=" + vals[4] + " pi=" + vals[5] + " str=" + vals[6]);

    // 2. array
    var arr = msgpack.pack([10, 20, 30]);
    var a = msgpack.unpack(arr);
    add("2. array: count=" + a.length + " [0]=" + a[0] + " [1]=" + a[1] + " [2]=" + a[2]);

    // 3. object
    var map = msgpack.pack({name:"joy", ver:1, ok:true});
    var m = msgpack.unpack(map);
    add("3. map: name=" + m.name + " ver=" + m.ver + " ok=" + m.ok);

    // 4. nested
    var nest = msgpack.pack([1, [2, 3], {x:10, y:20}]);
    var n = msgpack.unpack(nest);
    add("4. nested: [0]=" + n[0] + " [1][0]=" + n[1][0] + " [1][1]=" + n[1][1] + " [2].x=" + n[2].x + " [2].y=" + n[2].y);

    // 5. roundtrip
    var orig = {a:1, b:"test", c:[1,2,3]};
    var bin = msgpack.pack(orig);
    var back = msgpack.unpack(bin);
    add("5. roundtrip: a=" + back.a + " b=" + back.b + " c[0]=" + back.c[0]);

    // 6. 单值
    var single = msgpack.unpack(msgpack.pack("direct"));
    add("6. single: " + single);

    // 7. 空/错误
    var empty = msgpack.unpack("");
    add("7. empty: " + empty);

    add("=== done ===");
    }
};

joy.hotfix = function() {
    if (win) set_env("msg_win", win);
    if (ren) set_env("msg_ren", ren);
    if (font) set_env("msg_font", font);
};

joy.draw = function() {
    r2d.begin(ren, 25, 30, 40, 255);
    var y = 20;
    for (var i = 0; i < results.length; i++) {
        r2d.print(ren, font, results[i], 15, y, 200, 220, 255, 255);
        y = y + 22;
    }
    r2d.end_frame(ren);
};

joy.keypressed = function(key) {
    if (key === "escape" || key === "q") event.quit();
};
