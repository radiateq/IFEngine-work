#include <DrawText.h>

#include <android/bitmap.h>




//
//FT_Library  library;
//FT_Face     face;
//int32_t deviceDPI;
//FT_GlyphSlot  slot;
//FT_Matrix     matrix;              /* transformation matrix */
//FT_UInt       glyph_index;
//FT_Vector     pen;                 /* untransformed origin */
//FT_Byte      *buffer = NULL;
//int           n;
//int char_width = 0, char_height = 0;
//int char_width_px = 0, char_height_px = 0;
//struct android_app* android_app_state = NULL;
//
//bool InitFreeType(struct android_app* _state){
//
// if (buffer != NULL) {
//  DoneFreeType();  
// }
//
//
// if(_state!=NULL){  
//  android_app_state = _state;  
// }
// size_t size;
// //buffer = (FT_Byte*)RQNDKUtils::getAssetFileToBuffer(android_app_state, "RobotoMono-Regular.ttf", size);
// buffer = (FT_Byte*)RQNDKUtils::getAssetFileToBuffer(android_app_state, "Roboto-Thin.ttf", size);
// 
// if (buffer == NULL){
//  return false;
// }
//
// FT_Error error = FT_Init_FreeType(&library);
// if (error) { 
//  free(buffer);
//  return false;
// }
//
// error = FT_New_Memory_Face(library,
//  buffer,    
//  size,      
//  0,         
//  &face); 
// if (error) { 
//  free(buffer); 
//  return false; 
// }
//
// deviceDPI = RQNDKUtils::getDensityDpi(android_app_state);
//
// return true;
//}
//
//void DoneFreeType(){  
// if(buffer){
//  FT_Done_Face(face);
//  free(buffer), buffer = NULL;
// }
//}
//
//bool SetFontPixelSize(FT_UInt _width, FT_UInt _heigth ){
// if(_width!=0|| _heigth!=0){
//  char_width_px = _width;
//  char_height_px = _heigth;
// }
// char_width = char_height = 0;
// //If you want to specify the(integer) pixel sizes yourself, you can call FT_Set_Pixel_Sizes.
// FT_Error error = FT_Set_Pixel_Sizes(
//  face,   /* handle to face object */
//  char_width_px,      /* pixel_width           */
//  char_height_px);   /* pixel_height          */
// //This example sets the character pixel sizes to 16×16 pixels.As previously, a value of 0 for one of the dimensions means ‘same as the other’.
// if (error) { return false; }
// //Note that both functions return an error code.Usually, an error occurs with a fixed - size font format(like FNT or PCF) when trying to set the pixel size to a value that is not listed in the face->fixed_sizes array.
// return true;
//}
//
//bool SetFaceSize(FT_F26Dot6 _char_width, FT_F26Dot6 _char_height ) {
// if(_char_width!=0||_char_height!=0){
//  char_width = _char_width;
//  char_height = _char_height;
// }
// char_width_px = char_height_px = 0;
// if (FT_Set_Char_Size(
//  face,    /* handle to face object           */
//  char_width,       /* char_width in 1/64th of points  */
//  char_height,   /* char_height in 1/64th of points */
//  deviceDPI,     /* horizontal device resolution    */
//  deviceDPI)) {  /* vertical device resolution      */
//  return false;
// }
// return true;
//}
//
//
//
//void computeStringBBox(char *facestring, FT_BBox  *abbox, float angle, FT_Vector _pos )
//{
// FT_BBox  bbox;
// FT_BBox  glyph_bbox;
//
// FT_Matrix matrix;
// // set up matrix 
// matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
// matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
// matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
// matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);
//
// /* initialize string bbox to "empty" values */
// bbox.xMin = bbox.yMin = 32000;
// bbox.xMax = bbox.yMax = -32000;
// FT_Error error;
// int num_chars = strlen(facestring);
// bool use_kerning = FT_HAS_KERNING(face);
// FT_UInt previous = 0;
// //memset(&glyph,0,sizeof(glyph));
// for (size_t n = 0; n < num_chars; n++) {
//  error = FT_Load_Char(face, (unsigned char)facestring[n], FT_LOAD_DEFAULT);
//  if (error)
//   continue;  /* ignore errors */
//
//  if (use_kerning && previous && glyph_index)
//  {
//   FT_Vector  delta;
//   FT_Get_Kerning(face, previous, glyph_index,
//    FT_KERNING_DEFAULT, &delta);
//   _pos.x += delta.x>>6;
//   _pos.y += delta.y>>6;
//  }
//  previous = glyph_index;
//  {
//   FT_Glyph glyph;
//   error = FT_Get_Glyph(face->glyph, &glyph);
//   FT_Glyph_Get_CBox(glyph, ft_glyph_bbox_pixels, &glyph_bbox);
//   FT_Done_Glyph(glyph);
//  }
//
//  FT_Set_Transform(face, &matrix, &_pos);
//
//  _pos.x += face->glyph->advance.x>>6;
//  _pos.y += face->glyph->advance.y>>6;
//
//  glyph_bbox.xMin += _pos.x;
//  glyph_bbox.xMax += _pos.x;
//  glyph_bbox.yMin += _pos.y;
//  glyph_bbox.yMax += _pos.y;
//
//  if (glyph_bbox.xMin < bbox.xMin)
//   bbox.xMin = glyph_bbox.xMin;
//
//  if (glyph_bbox.yMin < bbox.yMin)
//   bbox.yMin = glyph_bbox.yMin;
//
//  if (glyph_bbox.xMax > bbox.xMax)
//   bbox.xMax = glyph_bbox.xMax;
//
//  if (glyph_bbox.yMax > bbox.yMax)
//   bbox.yMax = glyph_bbox.yMax;
//
//
// }
// 
//
// /* check that we really grew the string bbox */
// if (bbox.xMin > bbox.xMax){
//  bbox.xMin = 0;
//  bbox.yMin = 0;
//  bbox.xMax = 0;
//  bbox.yMax = 0;
// }
//
// /* return string bbox */
// *abbox = bbox;
//}
//
//
//struct IFSDrawText{
// //Input
// //text
// //target font width/heigth and unit enumerator either pixels or  /* char_width in 1/64th of points  */
// //target rendering start pos
// //target rotation angle
// //foreground bmp - pixels taken when font image is larger than 0
// //background bmp - pixels taken when font image is 0
// //Output - all parameters are optional
// //uv bottom, uv top
// //output image w/h
// //array holding texture - user must free
//};
////width: the width(in pixels) of the bitmap accessed via face->glyph->bitmap.width.
//// height : the height(in pixels) of the bitmap accessed via face->glyph->bitmap.rows.
//// bearingX : the horizontal bearing e.g.the horizontal position(in pixels) of the bitmap relative to the origin accessed via face->glyph->bitmap_left.
//// bearingY : the vertical bearing e.g.the vertical position(in pixels) of the bitmap relative to the baseline accessed via face->glyph->bitmap_top.
//// advance : the horizontal advance e.g.the horizontal distance(in 1 / 64th pixels) from the origin to the origin of the next glyph.Accessed via face->glyph->advance.x.
//
//
//
//   //                                                                                                                                                  
//   //                                    xmin                          xmax                              glyph metrics                                          
//   //                                    -:  /- .                      ./  ::                                                                           
//   //                               `:`-.:s`++./.:--              `:`-.-s./+`.:-:`--         .--.. ::         `/`     --    :.                          
//   //                               :-+-:o:+-o:/-/o.              --+--o-+-o:/s:-+:        .+-``-. y..`````.. .y`.    so  .o+  .` -o`.`.``+  `.  -.`    
//   //                               `````` `````` `               `````` ` `````` `        y.  -y-:o`y:-:h/`y`s:.y`  :.d`:.d`:o:o s:`m:/`h-`o./`-s/`    
//   //                                       -                               -              -/--/+`y- :s-/y-/::o`o+` -o s-.oo`o+-..y.+/  -y.:s-.-///     
//   //                           -.          .            width              .              --------::/-:y/-----------------------------------------`    
//   //                          +md:         -             `` `-  .`         -              ````````````````````````````````````````````````````````     
//   //                         +so+y:        -        -...`:`-::+-+-         -                                                                           
//   //                         ``+/``        -        /+/-://:o-//:+         -                                                                           
//   //                           +/          -  `     ..` .`.`..``.`      `  -                                                                           
//   //                           +/          -:::.........................-/::                                                        ymax               
//   //                           +/          -/:-`````````````````````````--/-                                                       `.`  .`             
//   //                           +/          - ``                         `` -                                                    ` ``o:`/+``.`` `       
//   //                           +/          /----------:/+oooo+/:-----------o``.`.-/ -`-`..```. -`- .`.`..`-`. -`.`.``.-:`- . -` .+-.:+/-:-/o./:-       
//   //                           +/          +      `:sdmdyoooydNmho:--------+   `://:`                                -:+-`     `-/./--.:-:---.-.       
//   //      bearingx             +/          +    `/dNMm:.     `/mMMNNNNNNNNNs   ``--``    bearingy                    ..:.`     ``                      
//   // -.         ``     `-- .-` +/          +   `yMMMN-         -mMMMMh++++/+     --  .                 .` `.          `:                               
//   //`+:..-`----::.:-.-- `/--`  +/```````--.+   yMMMMy           /MMMMN/    +     -- `+. .` .```.-``. .`.:`--          `:                               
//   //-///+./+:o./::/+:/:-:./.   ++.......-:+o  -MMMMM+           `mMMMMN    +     -- :/+:/.:o-o.+-+/+//- +.`           `:                               
//   //`` ```.``. .```-/:/.` `.   +/       .` +  /MMMMMo            dMMMMN    +     -- :.`:.--:..`:`--//:../`            `:                               
//   //               ``.`        +/          +  .NMMMMd`           dMMMMm    +     --                .-:`               `:                               
//   //                           +/          +   oNMMMM+          -NMMMm-    +     --                                   `:                               
//   //                           +/          +   `+mMMMN+`       .dMMNh-     +     --                        height     `:                               
//   //                           +/          +     .odNMNh+-...:smNmh:`      +     --                    /    ``  .- `. `:                               
//   //                           +/          +     `.omyosyhyyyys+:.         +     --                   :/:-/./::.+:.+/ `:                               
//   //                           +/          +    -ymh-`                     +     --                  .::-o-::+:::/./` `:                               
//   //                           +/          +   +NMMo.``````````            +     --                  ` ` ` `::+.````  `:                               
//   //                           +/          +  :MMMMMmhhhhhhhhhhssssss/:.`  +     --                          ``       `:                               
//   //                          `so.         +  .yNMMMMMMMMMMMMMMMMMMMMMMNy:`+     --       `:s/`        +/.            `:                               
//   //                   -:::::/hMMNo::::::::s:::/odMNmmmmmmmmmmmmmmmNNNMMMNss:::::++:::::::hMMMd::::::::+yms.          `:                               
//   //                   `````.-dNNN+````````+```.+md-```````````````.--/omMMy`````````````.ommNs````````/ys/`          `:                               
//   //                     ``.```s+-`        +  :dMy`                     -MMs               ``:         -.             `:                               
//   //         origin    -`.     +/          +`oNMN.                     `+Mh+                 .                        `:                               
//   //             `   ``        +/          ++MMMMs.`                 `.yNy`+                 -                        `:                               
//   //       `-`-.-:.-`:.:-.     +/          ++NMMMMNho/-..```````..-:ohNd/` +                 -                        `:            ymin               
//   //       +::o`+-o/:/:/+.     +/          + /yNMMMMMMNNNmddddddNNNNNy/`   +                 -                       ..:.`         `/. `/.`            
//   //       .``` -/:+.```.      +/          +```./shmNNNMMMMMMNNNmho:.``````+  `   `      `   -          ` ` `      ` -:+-`      ..- o/.//.:.-:`        
//   //             `..           +/          ---------:/+oooooo//:-----------:. -`. .`....`- . /`.`.``.`.`- .`-`..`.`. -./ .```.` `s.:-+/-:+-+/+`        
//   //                           +/                                                         `  -                                 `:.`..``... ```         
//   //                           +/`````````````````````````````````````````````````````````--`-                                                         
//   //                           ++----------------------------------------------------------/o:                                                         
//   //                           +/                        `                               `-` -                                                         
//   //                           +:                    `  `/.   ` ` ` ` `                      -                                                         
//   //                                                -+-::/o`:-+-o//-::+                      -                                                         
//   //                                               -:+::+-/-:-+.-/./.+.                      -                                                         
//   //                                                   advance                                                                                         
//   //                                                                                                                                                  
//
//GLuint DrawText(char *_text, FT_UInt _target_height, FT_Vector start_pos, double _angle, float *ub, float *vb, float *ut, float *vt) {
//
// InitFreeType();
// if(char_width){
//  SetFaceSize();
// }else{
//  SetFontPixelSize();
// }
//
// GLuint texture = GL_INVALID_VALUE;
//                                                                              //  _angle=0;
//
// slot = face->glyph;                /* a small shortcut */
//
// //glDisable(GL_TEXTURE_2D);
// glGenTextures(1, &texture);
// glActiveTexture(GL_TEXTURE0);
//
// FT_Matrix matrix;
// // set up matrix 
// matrix.xx = (FT_Fixed)(cos(_angle) * 0x10000L);
// matrix.xy = (FT_Fixed)(-sin(_angle) * 0x10000L);
// matrix.yx = (FT_Fixed)(sin(_angle) * 0x10000L);
// matrix.yy = (FT_Fixed)(cos(_angle) * 0x10000L);
//
//
// ///* the pen position in 26.6 cartesian space coordinates 
// ///* start at (300,200)                                   
// pen.x = start_pos.x>>6;
// pen.y = start_pos.y >> 6;
// //pen.x = 0 * 64;
// //pen.y = 0 * 64;
// size_t num_chars = strlen(_text);
// 
// unsigned int predicted_width;
// unsigned int predicted_height;
// int expanded_width = 0;
// int expanded_height = 0;
// GLubyte* expanded_data = NULL;
// struct SRGBA {
//  unsigned char r, g, b, a;
// };
// union URGBA {
//  SRGBA rgba;
//  unsigned int field : 32;
// };
// URGBA foreground, background;
// foreground.rgba.r = 25;
// foreground.rgba.g = 100;
// foreground.rgba.b = 250;
// foreground.rgba.a = 255;
// background.rgba.r = 10;
// background.rgba.g = 19;
// background.rgba.b = 135;
// background.rgba.a = 70;
//
//
// FT_BBox stringBBox;
// computeStringBBox(_text, &stringBBox, _angle, pen);
// predicted_width = stringBBox.xMax-stringBBox.xMin;
// predicted_height = stringBBox.yMax - stringBBox.yMin;
// expanded_width = next_p2(predicted_width);
// expanded_height = next_p2(predicted_height);
// // Allocate Memory For The Texture Data.
// //GLubyte* expanded_data = new GLubyte[2 * expanded_width * expanded_height];
// expanded_data = new GLubyte[4 * expanded_width * expanded_height];
//
//
// int j = 0, jb;
// int i = 0, ib;
// int k = 0, l=0;
// int k1 = INT_MIN, l1 = INT_MIN;
//
// for( j = 0; j < expanded_height; j++){
//  for (i = 0; i < expanded_width; i++) {
//   expanded_data[4 * (i + j * expanded_width) + 0] = background.rgba.r;
//   expanded_data[4 * (i + j * expanded_width) + 1] = background.rgba.g;
//   expanded_data[4 * (i + j * expanded_width) + 2] = background.rgba.b;
//   expanded_data[4 * (i + j * expanded_width) + 3] = background.rgba.a;
//  }
// }
//
//
// j = i = 0;
// unsigned int bmp_width, bmp_height;
// FT_UInt glyph_index;
// FT_Error error;
// bool use_kerning = FT_HAS_KERNING(face);
// FT_UInt previous = 0;
// for (size_t n = 0; n < num_chars; n++) {
//
//  //load glyph image into the slot (erase previous one) 
//  //error = FT_Load_Char(face, _text[n], FT_LOAD_RENDER);
//
//  glyph_index = FT_Get_Char_Index(face, _text[n]);
//
//  if (use_kerning && previous && glyph_index)
//  {
//   FT_Vector  delta;
//   FT_Get_Kerning(face, previous, glyph_index,
//    FT_KERNING_DEFAULT, &delta);
//   pen.x += delta.x >> 6;
//   pen.y += delta.y >> 6;
//  }
//  previous = glyph_index;
//
//  error = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
//  if (error)
//   continue;  /* ignore errors */
//
/////* now, draw to our target surface (convert position) */
//// my_draw_bitmap(&slot->bitmap,
////  slot->bitmap_left,
////  my_target_height - slot->bitmap_top);
//  // set transformation 
//  FT_Set_Transform(face, &matrix, &pen);
//
//
//  bmp_width = slot->bitmap.width, bmp_height = slot->bitmap.rows;
//  k = pen.y;
//  l = pen.x;
//  for (j = (k - bmp_height + slot->bitmap_top), jb = bmp_height-1; j < ((k ) + slot->bitmap_top); j++, jb--) {
//   for (i = (l + slot->bitmap_left), ib=0; i < ((l + slot->bitmap_left) +bmp_width); i++, ib++) {
//    {
//     if((i>= expanded_width)||(j>=expanded_height)||(i <0)||(j<0)){
//      continue;
//     }
//     if (k1 < j) k1 = j;
//     if( l1 < i ) l1 = i;
//     if ((slot->bitmap.buffer)[ib + bmp_width * jb] > 0) {
//      expanded_data[4 * (i + j * expanded_width) + 0] = (unsigned char)((float)(foreground.rgba.r + (slot->bitmap.buffer)[ib + bmp_width * jb])*0.5);
//      expanded_data[4 * (i + j * expanded_width) + 1] = (unsigned char)((float)(foreground.rgba.g + (slot->bitmap.buffer)[ib + bmp_width * jb])*0.5);
//      expanded_data[4 * (i + j * expanded_width) + 2] = (unsigned char)((float)(foreground.rgba.b + (slot->bitmap.buffer)[ib + bmp_width * jb])*0.5);
//      expanded_data[4 * (i + j * expanded_width) + 3] = (unsigned char)((float)(foreground.rgba.a + (slot->bitmap.buffer)[ib + bmp_width * jb])*0.5);
//     }
//    }    
// 
//   }
//  }
//
//  pen.x += (slot->advance.x >> 6);
//  pen.y += (slot->advance.y >> 6);
// }
//
//
// 
// if (ub) *ub = 0;
// if (vb) *vb = 0;
// if (ut) *ut = ((float)l1 + (float)(slot->advance.x >> 6)) / (float)expanded_width;
// if (vt) *vt = ((float)k1 + (float)(slot->advance.y >> 6)) / (float)expanded_height ;
//
// glBindTexture(GL_TEXTURE_2D, texture);
// //  glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, expanded_width, expanded_height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, (GLvoid*)(expanded_data) );
// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, expanded_width, expanded_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)(expanded_data));
// GLenum glerror = glGetError();
// if (glerror){
//  delete[] expanded_data;
//  return texture;
// }
//
// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//
//
// //free(PNG_image);
// delete[] expanded_data;
//
//
//return texture;
//}