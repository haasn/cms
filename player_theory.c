//audio-synchronized video player (with smoothmotion and no A/V latency):

start_playing_audio(); // this runs at realtime and continues playing in the background
start_decoding_loop(); // this decodes frames into some arbitrarily sized queue, asynchronously

// video rendering loop
while (true) {
    // we just woke up after a vsync and want to start rendering a frame for the next vsync to show
    the_vsync_even_before_that = prev_vsync;
    prev_vsync = opengl_time_of_last_vsync(); // the time that said vsync actually happened, according to OpenGL

    vsync_interval = prev_vsync - the_vsync_even_before_that; // converted to nanoseconds


    if (audio_synced) {
        current_time = get_audio_pos(); // nanoseconds since the start of the file
    } else {
        current_time = prev_vsync; // we take this to mean “right now”
        current_audio_time = get_audio_pos;

        desync = current_time - current_audio_time;

        if (abs(desync) > vsync_interval) {
            // Oops?
            seek_to(current_time);
        } else {
            // we want to speed up or slow down by this amount until the next vsync, ideally
            speed_coeff = 1 + (desync / vsync_interval);
            update_playback_speed(speed_coeff);
        }
    }


    next_vsync_time = current_time + vsync_interval; // this is when we guess that the frame will appear on screen, right?
    vsync_time_after_that = next_vsync_time + vsync_interval; // and the frame that's *after* it (in the distant future)

    frame1 = get_frame_with_pts(next_vsync_time);
    frame2 = get_frame_with_pts(vsync_time_after_that);

    // we want to display a mix of frame1+frame2, depending on the frame2's actual PTS
    mix_coeff = (frame2.pts - next_vsync_time) / vsync_interval;
    mix_coeff = clamp(mix_coeff, 0, 1); // alternatively, we could display simply just frame1 if they are the same frame

    draw(mix(frame1, frame2, mix_coeff));
    flush();

    garbage_collect(current_time);
    sleep_until_after_next_vsync();
}

// this is where the “hard bit” happens, essentially. We want to go through
// the input queue until we reach a frame that matches the requirements
get_frame_with_pts(nanoseconds t) {
    frame this = NULL;
    for (int i = 0; i < QUEUE_SIZE; i++) {
        if (decoding_queue[i].pts < t) {
            this = decoding_queue[i];
        } else {
            return this;
        }
    }

    // if we got here, then something BAD happened: the decoder isn't
    // delivering frames fast enough, we need to skip a few frames to compensate
    // (or just tell the user to upgrade hardware, really)
    tell_the_decoder_to_seek_to(t);
    // alternatively:
    tell_the_decoder_to_skip(t - this.pts);
    // return the last good frame anyways
    return this;
}

// this just takes care of consuming frames that we will never care about again
garbage_collect(nanoseconds t) {
    // excuse my french
    while (decoding_queue[0].pts < t)
        decoding_queue.remove(0);
}
