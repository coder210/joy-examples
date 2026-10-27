// Worker script: uses joy lifecycle (similar to main thread)
// Enters infinite loop, continuously calling joy.update()

joy.load = function()
{
    console.log("worker: load");
};

joy.update = function(dt)
{
};

joy.quit = function()
{
    console.log("worker: quit");
};
