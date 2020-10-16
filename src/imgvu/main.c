//
// bumbread's imgvu
//
// el manifesto
//
// This is a fast powerful image viewer that allows you to
// use your mouse as little as possible, while getting the maximum
// out of viewing images.
// 
// Goals: 
// - Robust
// - Instanteneous (as fas as user's perception is concerned)
// - Easy to port.
// - Simple control
// - Supporting lots of image formats.
// NOTE(bumbread): the following formats are planned to be supported:
// ppm bmp tga gif jpeg png jpeg2000 openexr heif avif flif bpg
// NOTE(bumbread): the following formats are already supported:
// <none>
// 
// 

internal t_image app_decode_file(t_image_data data) {
  t_image result = {0};
  result.skip = true;
  debug_variable_unused(data);
  return(result);
}

internal bool app_update(struct t_directory_state_s* dirState, t_button* keyboard, r32 dt) {
  if(keyboard[VKEY_ESCAPE].pressed) return(true);
  debug_variable_unused(dt);
  
  {
    bool left = keyboard[VKEY_LEFT].pressed;
    bool right = keyboard[VKEY_RIGHT].pressed;
    if(left && !right) {
      platform_directory_previous_file(dirState);
    }
    if(right && !left) {
      platform_directory_next_file(dirState);
    }
  }
  
  return(false);
}

internal void app_draw(t_app_state* state) {
  // NOTE(bumbread): platform graphics calls and stuff
  debug_variable_unused(state);
}
