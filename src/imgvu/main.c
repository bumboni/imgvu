//
// bumbread's imgvu
//

#include"format/bmp.c"
#include"format/pnm.c"

internal t_image app_decode_file(t_file_data data) {
  t_image result = {0};
  result.success = false;
  try_parse_pnm(&data, &result);
  try_parse_bmp(&data, &result);
  debug_variable_unused(data);
  return(result);
}

internal bool app_update(t_app_state* appState, struct t_directory_state_s* dirState, t_app_input* input, r32 dt) {
  if(!appState->initialized) {
    app_load_config(&appState->appConfig, platform_get_config_filename());
    
    appState->initialized = true;
    appState->dirState = dirState;
    
    appState->imageLocation.scale = 1.0f;
  }
  
  debug_variable_unused(dt);
  
  if(input->prevImage && !input->nextImage) {
    platform_directory_previous_file(dirState);
    appState->appConfig.backgroundColor += 1;
    if(appState->appConfig.backgroundColor >= appState->appConfig.colorCycle.len) {
      appState->appConfig.backgroundColor = 0;
    }
  }
  if(input->nextImage && !input->prevImage) {
    platform_directory_next_file(dirState);
    appState->appConfig.backgroundColor += 1;
    if(appState->appConfig.backgroundColor >= appState->appConfig.colorCycle.len) {
      appState->appConfig.backgroundColor = 0;
    }
  }
  
  return(false);
}

internal void app_draw(t_app_state* appState) {
  
  if(appState->appConfig.colorCycle.ptr)
    platform_clear_screen(appState->appConfig.colorCycle.ptr[appState->appConfig.backgroundColor]);
  
  if(appState->dirState != 0) {
    t_image* currentImage = platform_get_current_image(appState->dirState);
    if(currentImage != 0) {
      platform_draw_image(&appState->imageLocation, currentImage);
    }
  }
}
