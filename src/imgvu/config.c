
//#include<stdio.h>

struct {
  u32 len;
  u32* ptr;
} typedef t_array_i;

struct {
  u32 len;
  r32* ptr;
} typedef t_array_f;

struct {
  u32 len;
  t_string* ptr;
} typedef t_array_s;

struct {
  bool error;
  t_array_i colorCycle;
  u32 backgroundColor;
} typedef t_app_config;

internal void config_load_default(t_app_config* appConfig) {
  appConfig->error = false;
  appConfig->colorCycle.len = 1;
  appConfig->colorCycle.ptr = malloc(appConfig->colorCycle.len * sizeof(u32));
  appConfig->colorCycle.ptr[0] = 0xffff00ff;
  appConfig->backgroundColor = 0;
}

internal void app_write_config_to_file(t_app_config* appConfig, t_string16 filename) {
  t_file_data output = {0};
  output.filename = filename;
  platform_write_file(output);
  debug_variable_unused(appConfig);
}

enum {
  TYPE_INTEGER,
  TYPE_STRING,
  TYPE_FLOAT,
  TYPE_ARRAY_INTEGER,
  TYPE_ARRAY_STRING,
  TYPE_ARRAY_FLOAT,
} typedef t_setting_type;

struct {
  t_string name;
  t_setting_type type;
  union {
    i32 value_i;
    r32 value_f;
    t_string value_s;
    t_array_i value_ai;
    t_array_f value_af;
    t_array_s value_as;
  };
} typedef t_setting;

struct {
  t_setting* v;
  u32 count;
  u32 alloc;
} typedef t_setting_list;

internal void settings_add(t_setting_list* list, t_setting setting) {
  if(list->count + 1 > list->alloc) {
    list->alloc *= 2;
    if(list->alloc == 0) list->alloc = 1;
    list->v = realloc(list->v, list->alloc * sizeof(t_setting));
  }
  list->v[list->count] = setting;
  list->count += 1;
}

enum {
  
  TOKEN_TYPE_IDENTIFIER,
  TOKEN_TYPE_INTEGER,
  TOKEN_TYPE_FLOAT,
  TOKEN_TYPE_STRING,
  
  TOKEN_TYPE_ARRAY_OPEN,
  TOKEN_TYPE_ARRAY_CLOSE,
  TOKEN_TYPE_ASSIGMENT,
  
  TOKEN_TYPE_EOF,
  
} typedef t_token_type;

struct {
  t_token_type type;
  char* start;
  u32 len;
} typedef t_token;

struct {
  t_token* v;
  u32 count;
  u32 alloc;
} typedef t_token_list;

internal void token_push(t_token_list* list, t_token token) {
  if(list->count + 1 > list->alloc) {
    list->alloc *= 2;
    if(list->alloc == 0) list->alloc = 1;
    list->v = realloc(list->v, list->alloc * sizeof(t_token));
  }
  list->v[list->count] = token;
  list->count += 1;
}

internal inline bool in_range(char c, char s, char e) {return(c>=s && c<=e);}

internal inline bool is_letter(char c) { return(in_range(c,'a','z') || in_range(c, 'A', 'Z') || (c=='_'));}
internal inline bool is_dec_digit(char c) { return(in_range(c, '0', '9')); }
internal inline bool is_hex_digit(char c) { return(in_range(c, '0', '9') || in_range(c, 'a', 'f') || in_range(c,'A','F')); }
internal inline bool is_bin_digit(char c) { return(in_range(c, '0', '9') || in_range(c, 'a', 'f') || in_range(c,'A','F')); }
internal inline bool is_whitespace(char c) { return(c==' ' || c==',' || c==';' || in_range(c, 1, 31)); }
internal inline bool is_alphanumeric(char c) { return(is_letter(c) || is_dec_digit(c)); }

internal inline int get_dec_digit(char c) { return((int)(c - '0')); }
internal inline int get_hex_digit(char c) { 
  if(in_range(c, 'a', 'f')) return(10 + (int)(c - 'a'));
  if(in_range(c, 'A', 'F')) return(10 + (int)(c - 'A'));
  if(in_range(c, '0', '9')) return((int)(c - '0'));
  return(-1);
}

internal bool parse_config_file(t_setting_list* list, t_file_data fileData) {
  
  char* text = (char*)fileData.ptr;
  u32 index = 0;
  
  t_token_list tokens = {0};
  t_token token = {0};
  
#define advance_to(state)   {index+=1;token.len+=1; goto state;}
  //#define jump_to(state) {goto state;}
#define token_start()       token.start = text + index;token.len = 0
#define token_finish(state) {token_push(&tokens, token); goto state;}
#define state_start(t)      token.type = (t); char c = text[index]
  
  state_main: {
    state_start(0);
    token_start();
    if(is_letter(c))          {advance_to(state_identifier)}
    else if(c=='0')           {advance_to(state_integer_prefix)}
    else if(c=='.')           {advance_to(state_float)}
    else if(c=='{')           {advance_to(state_array_start)}
    else if(c=='}')           {advance_to(state_array_end)}
    else if(c=='"')           {advance_to(state_string)}
    else if(c=='=')           {advance_to(state_assigment)}
    else if(is_dec_digit(c))  {advance_to(state_dec_integer)}
    else if(is_whitespace(c)) {advance_to(state_main)}
    else if(c==0)             goto end;
    else                      goto error;
  }
  
  state_identifier: {
    state_start(TOKEN_TYPE_IDENTIFIER);
    if(is_alphanumeric(c))    {advance_to(state_identifier)}
    else if(is_whitespace(c)) {token_finish(state_main)}
    else if(c=='}')           {token_finish(state_main)}
    else if(c=='=')           {token_finish(state_main)}
    else if(c==0)             {token_finish(state_main)}
    else                      goto error;
  }
  
  state_integer_prefix: {
    state_start(TOKEN_TYPE_INTEGER);
    if(is_dec_digit(c))       {advance_to(state_dec_integer)}
    else if(c=='x')           {advance_to(state_hex_integer)}
    else if(c=='b')           {advance_to(state_bin_integer)}
    else if(c=='.')           {advance_to(state_float)}
    else if(c=='=')           {token_finish(state_main)}
    else if(c=='}')           {token_finish(state_main)}
    else if(is_whitespace(c)) {token_finish(state_main)}
    else if(c==0)             goto error;
    else                      goto error;
  }
  
  state_dec_integer: {
    state_start(TOKEN_TYPE_INTEGER);
    if(is_dec_digit(c))       {advance_to(state_dec_integer)}
    else if(c=='.')           {advance_to(state_float)}
    else if(c=='}')           {token_finish(state_main)}
    else if(c=='=')           {token_finish(state_main)}
    else if(c==0)             {token_finish(state_main)}
    else if(is_whitespace(c)) {token_finish(state_main)}
    else                      goto error;
  }
  
  state_hex_integer: {
    state_start(TOKEN_TYPE_INTEGER);
    if(is_hex_digit(c))       {advance_to(state_hex_integer)}
    else if(c=='}')           {token_finish(state_main)}
    else if(c=='=')           {token_finish(state_main)}
    else if(c==0)             {token_finish(state_main)}
    else if(is_whitespace(c)) {token_finish(state_main)}
    else                      goto error;
  }
  
  state_bin_integer: {
    state_start(TOKEN_TYPE_INTEGER);
    if(is_bin_digit(c))       {advance_to(state_bin_integer)}
    else if(c=='}')           {token_finish(state_main)}
    else if(c=='=')           {token_finish(state_main)}
    else if(c==0)             {token_finish(state_main)}
    else if(is_whitespace(c)) {token_finish(state_main)}
    else                      goto error;
  }
  
  state_float: {
    state_start(TOKEN_TYPE_FLOAT);
    if(is_dec_digit(c))       {advance_to(state_float)}
    else if(c=='}')           {token_finish(state_main)}
    else if(c=='=')           {token_finish(state_main)}
    else if(c==0)             {token_finish(state_main)}
    else if(is_whitespace(c)) {token_finish(state_main)}
    else                      goto error;
  }
  
  state_array_start: {
    state_start(TOKEN_TYPE_ARRAY_OPEN);
    {token_finish(state_main)}
    debug_variable_unused(c);
  }
  
  state_array_end: {
    state_start(TOKEN_TYPE_ARRAY_CLOSE);
    {token_finish(state_main)}
    debug_variable_unused(c);
  }
  
  state_assigment: {
    state_start(TOKEN_TYPE_ASSIGMENT);
    {token_finish(state_main)}
    debug_variable_unused(c);
  }
  
  state_string: {
    state_start(TOKEN_TYPE_STRING);
    if(c=='\\')               {advance_to(state_string_screen)}
    else if(c=='"')           {advance_to(state_string_end)}
    else if(c==0)             goto error;
    else                      {advance_to(state_string)}
  }
  
  state_string_screen: {advance_to(state_string)}
  state_string_end:    {token_finish(state_main)}
  
  end:
  debug_variable_unused(list);
  
  return(true);
  error: {
    return(false);
  }
}

struct {
  t_setting_type type;
  t_string name;
  union {
    i32* value_i;
    r32* value_f;
    t_string* value_s;
    t_array_i* value_ai;
    t_array_f* value_af;
    t_array_s* value_as;
  };
} typedef t_setting_link;

struct {
  t_setting_link* v;
  u32 count;
  u32 alloc;
} typedef t_link_list;

internal void link_add(t_link_list* list, t_setting_link link) {
  if(list->count + 1 > list->alloc) {
    list->alloc *= 2;
    if(list->alloc == 0) list->alloc = 1;
    list->v = realloc(list->v, list->alloc * sizeof(t_setting_link));
  }
  list->v[list->count] = link;
  list->count += 1;
}

internal void link_create_i(t_link_list* list, t_string name, i32* value) {
  t_setting_link result = {0};
  result.type = TYPE_INTEGER;
  result.name = name;
  result.value_i = value;
  link_add(list, result);
}

internal void link_create_f(t_link_list* list, t_string name, r32* value) {
  t_setting_link result = {0};
  result.type = TYPE_FLOAT;
  result.name = name;
  result.value_f = value;
  link_add(list, result);
}

internal void link_create_s(t_link_list* list, t_string name, t_string* value) {
  t_setting_link result = {0};
  result.type = TYPE_STRING;
  result.name = name;
  result.value_s = value;
  link_add(list, result);
}

internal void link_create_ai(t_link_list* list, t_string name, t_array_i* value) {
  t_setting_link result = {0};
  result.type = TYPE_ARRAY_INTEGER;
  result.name = name;
  result.value_ai = value;
  link_add(list, result);
}

internal void link_create_af(t_link_list* list, t_string name, t_array_f* value) {
  t_setting_link result = {0};
  result.type = TYPE_ARRAY_FLOAT;
  result.name = name;
  result.value_af = value;
  link_add(list, result);
}

internal void link_create_as(t_link_list* list, t_string name, t_array_s* value) {
  t_setting_link result = {0};
  result.type = TYPE_ARRAY_STRING;
  result.name = name;
  result.value_as = value;
  link_add(list, result);
}

internal void config_load_settings(t_setting_list* settings, t_link_list* links) {
  for(u32 settingIndex = 0; settingIndex < settings->count; settingIndex += 1) {
    t_setting* currentSetting = settings->v + settingIndex;
    for(u32 linkIndex = 0; linkIndex < links->count; linkIndex += 1) {
      t_setting_link* link = links->v + linkIndex;
      
      if(string_compare(link->name, currentSetting->name)) {
        if(link->type == currentSetting->type) {
          switch(link->type) {
            case(TYPE_INTEGER): {
              *link->value_i = currentSetting->value_i;
            } break;
            case(TYPE_FLOAT): {
              *link->value_f = currentSetting->value_f;
            } break;
            case(TYPE_STRING): {
              *link->value_s = string_copy_mem(currentSetting->value_s);
            } break;
            case(TYPE_ARRAY_INTEGER): {
              link->value_ai->len = currentSetting->value_ai.len;
              if(link->value_ai->ptr) free(link->value_ai->ptr);
              link->value_ai->ptr = malloc(link->value_ai->len * sizeof(u32));
              for(u32 valueIndex = 0; valueIndex < currentSetting->value_ai.len; valueIndex += 1) {
                link->value_ai->ptr[valueIndex] = currentSetting->value_ai.ptr[valueIndex];
              }
            } break;
            case(TYPE_ARRAY_FLOAT): {
              link->value_af->len = currentSetting->value_af.len;
              if(link->value_af->ptr) free(link->value_af->ptr);
              link->value_af->ptr = malloc(link->value_af->len * sizeof(r32));
              for(u32 valueIndex = 0; valueIndex < currentSetting->value_af.len; valueIndex += 1) {
                link->value_af->ptr[valueIndex] = currentSetting->value_af.ptr[valueIndex];
              }
            } break;
            case(TYPE_ARRAY_STRING): {
              link->value_as->len = currentSetting->value_as.len;
              if(link->value_as->ptr) free(link->value_as->ptr);
              link->value_as->ptr = malloc(link->value_as->len * sizeof(u32));
              for(u32 valueIndex = 0; valueIndex < currentSetting->value_as.len; valueIndex += 1) {
                link->value_as->ptr[valueIndex] = string_copy_mem(currentSetting->value_as.ptr[valueIndex]);
              }
            } break;
          }
        }
        else {
          // TODO(bumbread): logging
        }
      }
      
    }
    
  }
}

internal void config_free_settings(t_setting_list* settings) {
  for(u32 settingIndex = 0; settingIndex < settings->count; settingIndex += 1) {
    t_setting* setting = settings->v + settingIndex;
    if(setting->type == TYPE_ARRAY_INTEGER) {
      if(setting->value_ai.ptr) free(setting->value_ai.ptr);
    }
    else if(setting->type == TYPE_STRING) {
      if(setting->value_s.ptr) free(setting->value_s.ptr);
    }
    else if(setting->type == TYPE_ARRAY_FLOAT) {
      if(setting->value_af.ptr) free(setting->value_af.ptr);
    }
    else if(setting->type == TYPE_ARRAY_STRING) {
      if(setting->value_as.ptr) {
        for(u32 stringIndex = 0; stringIndex < setting->value_as.len; stringIndex += 1) {
          free(setting->value_as.ptr[stringIndex].ptr);
        }
        free(setting->value_as.ptr);
      }
    }
  }
  free(settings->v);
}

internal void app_load_config(t_app_config* appConfig, t_string16 filename) {
  debug_variable_unused(appConfig);
  
  config_load_default(appConfig);
  // TODO(bumbread): this code is for testing.
  // remove this later.
  
  t_setting_list settings = {0};
  t_link_list links = {0};
  
  //t_file_data configData = platform_load_file(filename);
  t_file_data configData = {0};
  configData.ptr = "color_test_count = 2;\ncolor_array= {2,4,test,\"wh\\\"at?\"\n}\nstring=\"test\"";
  
  bool shouldWriteNewConfig = (configData.ptr == false);
  if(configData.ptr) {
    parse_config_file(&settings, configData);
  }
  
#if 0  
  t_setting colorCycleSetting = {0};
  colorCycleSetting.name = char_copy("color_cycle");
  colorCycleSetting.type = TYPE_ARRAY_INTEGER;
  colorCycleSetting.value_ai.len = 2;
  colorCycleSetting.value_ai.ptr = malloc(2 * sizeof(u32));
  colorCycleSetting.value_ai.ptr[0] = 0xff222222;
  colorCycleSetting.value_ai.ptr[1] = 0xffeeeeee;
  settings_add(&settings, colorCycleSetting);
#endif
  
  link_create_ai(&links, char_count("color_cycle"), &appConfig->colorCycle);
  
  config_load_settings(&settings, &links);
  config_free_settings(&settings);
  
  if(shouldWriteNewConfig) {
    app_write_config_to_file(appConfig, filename);
  }
}
