## minifb: set_target_fps broken on high-refresh-rate monitors

minifb enables hardware sync (GLX swap interval) when set_target_fps is called, which locks to the monitor refresh rate instead of the requested FPS. On a 180 Hz monitor, set_target_fps(60) results in 180 FPS. Needs an upstream PR or custom software pacing.
