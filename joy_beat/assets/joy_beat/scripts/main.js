// Demo selector for joy_beat (JS)
// Usage: joy_beat.exe joy_beat/scripts/XX-name.js
// Run a specific demo script directly.

joy.load = function()
{
    console.log("main.js loaded");
    console.log("Run a specific demo script directly, e.g.:");
    console.log("  joy_beat.exe joy_beat/scripts/00-hello.js");
    console.log("  joy_beat.exe joy_beat/scripts/05-collision-circles.js");
    console.log("  joy_beat.exe joy_beat/scripts/12-ui.js");
};

joy.update = function(dt)
{
};

joy.draw = function()
{
};
