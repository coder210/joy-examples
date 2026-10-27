local livnet = requirex "livnet"

local SDL_EVENT_KEY_DOWN = 768;
local SDL_SCANCODE_ESCAPE = 41;
local SDL_SCANCODE_Q = 41;
local SDL_SCANCODE_RIGHT = 79;
local SDL_SCANCODE_UP = 82;
local SDL_SCANCODE_LEFT = 80;
local SDL_SCANCODE_DOWN = 81;

local window, renderer;


local function exit_all()
        livnet.exit();
end

local function handle_keydown_event(scancode)
        if scancode == SDL_SCANCODE_Q then
                exit_all();
        elseif scancode == SDL_SCANCODE_ESCAPE then
                exit_all();
        elseif scancode == SDL_SCANCODE_UP then
        elseif scancode == SDL_SCANCODE_RIGHT then
        elseif scancode == SDL_SCANCODE_DOWN then
        elseif scancode == SDL_SCANCODE_LEFT then
        end
end

livnet.start(function()
        livnet.log("ui");
        local ok, width, height;
        sdl.set_app_metadata("audio", "1.0", "com.livnet.audio");
        ok, window, renderer = sdl.create_window_and_renderer("audio", 960, 720);
        assert(ok);
        ok, width, height = sdl.get_window_size(window);

	local zombie = sdl.create_audio_stream_from_ogg("adventure_server/sounds/zombie.ogg");
	local helicopter = sdl.create_audio_stream_from_wav("adventure_server/sounds/helicopter.wav");
	local ak47 = sdl.create_audio_stream_from_wav("adventure_server/sounds/ak47.wav");

        local audio_device_id = sdl.open_audio_device();
        sdl.bind_audio_stream(audio_device_id, zombie.stream);
        sdl.bind_audio_stream(audio_device_id, helicopter.stream);
        sdl.bind_audio_stream(audio_device_id, ak47.stream);

        --sdl.pause_audio_device(audio_device_id);

        livnet.dispatch("sdl_event", function(data, sz, source, ...)
                local etype = sdl.get_event_type(data);
                local scancode = sdl.get_event_scancode(data);
                if etype == SDL_EVENT_KEY_DOWN then
                        handle_keydown_event(scancode);
                end
        end);


        --sdl.put_audio_stream_data(helicopter);
        --sdl.put_audio_stream_data(ak47);
        
        sdl.put_audio_stream_data(zombie);
        livnet.dispatch("response", function(data, sz, source, ...)            
                --sdl.put_audio_stream_data(helicopter);

                sdl.set_render_draw_color(renderer, 0, 0, 0, 255);
                sdl.render_clear(renderer);


                sdl.render_present(renderer);
                livnet.timeout(16);
        end);
        livnet.timeout(16);

        livnet.exit_func(function()
                sdl.close_audio_device(audio_device_id);
                sdl.destroy_audio_stream(zombie);
                sdl.destroy_audio_stream(helicopter);
                sdl.destroy_audio_stream(ak47);

                sdl.destroy_renderer(renderer);
                sdl.destroy_window(window);
                livnet.log("audio service exit");
        end);
end);


-- local program = {};




-- return program;


livnet.start(function ()
        
end)






