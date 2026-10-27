// Audio playback demo with buttons
var device_id, bgm, shot;
var ren, win;

// 鎸夐挳瀹氫箟 {x, y, w, h, label, color, action}
var buttons = [];

joy.load = function()
{
    win = get_env("audio_win");
    ren = get_env("audio_ren");
    device_id = get_env("audio_dev");
    if (!device_id) {
        win = window.create("audio", 960, 720, 0);
    ren = r2d.create(win);
    device_id = audio.open_device();
    console.log("audio device: " + device_id);

    var cx = 480, y0 = 100, bw = 200, bh = 60, gap = 20;

    buttons = [
        { x: cx-bw/2, y: y0,          w: bw, h: bh, label: "WAV",  r: 60,  g: 120, b: 220,
          action: function() {
              var ok = audio.create("joy_beat/sounds/ak47.wav", device_id);
              console.log("WAV create: " + ok);
          }},
        { x: cx-bw/2, y: y0+bh+gap,   w: bw, h: bh, label: "OGG",  r: 60,  g: 200, b: 100,
          action: function() {
              var ok = audio.create("joy_beat/sounds/zombie.ogg", device_id);
              console.log("OGG create: " + ok);
          }},
        { x: cx-bw/2, y: y0+(bh+gap)*2, w: bw, h: bh, label: "Shot", r: 220, g: 120, b: 60,
          action: function() {
              if (shot) audio.shot_destroy(shot);
              shot = audio.create_shot("joy_beat/sounds/ak47.wav", device_id);
              audio.shot_play(shot);
              console.log("shot played");
          }},
        { x: cx-bw/2, y: y0+(bh+gap)*3, w: bw, h: bh, label: "BGM",  r: 180, g: 60,  b: 180,
          action: function() {
              if (bgm) audio.bgm_destroy(bgm);
              bgm = audio.create_bgm("joy_beat/sounds/zombie.ogg", device_id);
              audio.bgm_set_gain(bgm, 0.5);
              console.log("BGM started, gain=0.5");
          }},
        { x: cx-bw/2, y: y0+(bh+gap)*4, w: bw, h: bh, label: "Stop", r: 200, g: 60,  b: 60,
          action: function() {
              if (bgm) { audio.bgm_stop(bgm); console.log("BGM stopped"); }
          }},
    ];
    }  // end if !device_id
};

joy.hotfix = function() {
    if (win) set_env("audio_win", win);
    if (ren) set_env("audio_ren", ren);
    if (device_id) set_env("audio_dev", device_id);
};

joy.draw = function()
{
    r2d.set_color(ren, 40, 40, 40, 255);
    r2d.begin(ren);

    for (var i = 0; i < buttons.length; i++) {
        var btn = buttons[i];
        r2d.set_color(ren, btn.r, btn.g, btn.b, 255);
        r2d.rectangle(ren, "fill", btn.x, btn.y, btn.w, btn.h);
        r2d.set_color(ren, 255, 255, 255, 80);
        r2d.rectangle(ren, "line", btn.x, btn.y, btn.w, btn.h);
    }

    r2d.end_frame(ren);
};

joy.mousepressed = function(x, y, button)
{
    for (var i = 0; i < buttons.length; i++) {
        var btn = buttons[i];
        if (x >= btn.x && x <= btn.x + btn.w && y >= btn.y && y <= btn.y + btn.h) {
            btn.action();
            break;
        }
    }
};

joy.keypressed = function(key, scancode, repeat_, ctrl, shift)
{
    if (key == "q" || key == "escape") event.quit();
};

joy.quit = function()
{
    if (shot) audio.shot_destroy(shot);
    if (bgm) audio.bgm_destroy(bgm);
    if (device_id) audio.close_device(device_id);
    if (ren) { r2d.destroy(ren); ren = null; }
    if (win) { window.destroy(win); win = null; }
    console.log("audio demo exit");
};
