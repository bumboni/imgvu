
internal void clear_screen_gdi(u32 color) {
  for(u32 row = 0; row < g_window.clientHeight; row += 1) {
    for(u32 column = 0; column < g_window.clientWidth; column += 1) {
      g_window.pixels[row*g_window.clientWidth + column] = color;
    }
  }
}

internal void draw_image_gdi(t_location* loc, t_image* image) {
  i32 maxWidth = (i32)g_window.clientWidth;
  i32 maxHeight = (i32)g_window.clientHeight;
  
  i32 xPosition = (i32)loc->posX;
  i32 yPosition = (i32)loc->posY;
  i32 width = (i32)image->width;
  i32 height = (i32)image->height;
  
  if(xPosition >= maxWidth) return;
  if(yPosition >= maxHeight) return;
  if(xPosition + maxWidth <= 0) return;
  if(yPosition + maxHeight <= 0) return;
  
  if(xPosition < 0) { 
    width += xPosition;
    xPosition = 0;
  }
  if(yPosition < 0) {
    height += yPosition;
    yPosition = 0;
  }
  if(xPosition + width > maxWidth) {
    i32 over = xPosition + width - maxWidth;
    assert(over > 0);
    width -= over;
  }
  if(yPosition + height > maxHeight) {
    i32 over = yPosition + height - maxHeight;
    assert(over > 0);
    height -= over;
  }
  
  assert(xPosition >= 0);
  assert(yPosition >= 0);
  assert(xPosition + width <= maxWidth);
  assert(yPosition + height <= maxHeight);
  
  u32* targetRow = g_window.pixels + (u32)yPosition*g_window.clientWidth + (u32)xPosition;
  u32* sourceRow = image->pixels;
  for(i32 row = 0; row < height; row += 1) {
    u32* targetPixel = targetRow;
    u32* sourcePixel = sourceRow;
    for(i32 column = 0; column < width; column += 1) {
      *targetPixel = *sourcePixel;
      targetPixel += 1;
      sourcePixel += 1;
    }
    sourceRow += image->width;
    targetRow += g_window.clientWidth;
  }
}

internal void image_show_gdi(void) {
  assert(g_window.pixels);
  
  BITMAPINFO bitmapInfo = {0};
  bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
  bitmapInfo.bmiHeader.biWidth = (LONG)g_window.clientWidth;
  bitmapInfo.bmiHeader.biHeight = (LONG)g_window.clientHeight;
  bitmapInfo.bmiHeader.biPlanes = 1;
  bitmapInfo.bmiHeader.biBitCount = 32;
  bitmapInfo.bmiHeader.biCompression = BI_RGB;
  
  StretchDIBits(g_window.deviceContext, 
                0, 0, (LONG)g_window.clientWidth, (LONG)g_window.clientHeight,
                0, 0, (LONG)g_window.clientWidth, (LONG)g_window.clientHeight,
                g_window.pixels, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

internal void clear_screen_gl(u32 color) {
  u32 r = (color >> 0)  & 0xff;
  u32 g = (color >> 8)  & 0xff;
  u32 b = (color >> 16) & 0xff;
  glClearColor((r32)r / 255.0f, (r32)g/255.0f, (r32)b/255.0f, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
}

internal void draw_image_gl(t_location* loc, t_image* image) {
  debug_variable_unused(loc);
  debug_variable_unused(image);
  glColor4f(1.0, 1.0, 1.0, 1.0);
  glLoadIdentity();
  glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
  glBegin(GL_POLYGON);
  glVertex3f (0.25, 0.25, 0.0);
  glVertex3f (0.75, 0.25, 0.0);
  glVertex3f (0.75, 0.75, 0.0);
  glVertex3f (0.25, 0.75, 0.0);
  glEnd();
}

internal void image_show_gl(void) {
  glFlush();
  SwapBuffers(g_window.deviceContext);
}
