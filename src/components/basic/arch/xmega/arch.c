#include "arch.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "spi.h"
#include "ff.h"

  int
asprintf(char **str, const char *format, ...)
{
  va_list argp;
  va_start(argp, format);

  char one_char[1];
  int len = vsnprintf(one_char, 1, format, argp);
  if (len < 1){
    *str = NULL;
    return len;
  }
  va_end(argp);
  *str = malloc(len+1);
  if (!str) {
    return -1;
  }
  va_start(argp, format);
  vsnprintf(*str, len+1, format, argp);
  va_end(argp);
  return len;
}

  float
strtof(const char *restrict nptr, char **restrict endptr)
{
  float f;
  sscanf(nptr, "%f", &f);
  return f;
}

  char*
strndup(const char *s, size_t n)
{
  size_t len = strnlen (s, n);
  char *new = (char *) malloc (len + 1);
  if (new == NULL)
  {
    return NULL;
  }
  new[len] = '\0';
  return (char *) memcpy (new, s, len);
}

// -- SD Card specific

FATFS FatFs;

  int
arch_init(void)
{
  spi_init();
  f_mount(&FatFs, "", 0); 
  return 0;
}

  int
arch_load(char* name, arch_load_out_cb cb, void* context)
{
  char filename[13]; // 8 + '.' + 3 + '\0'
  char line[128];
  FIL fil;
  snprintf(filename, sizeof(filename), "%s.bas", name);
  f_open(&fil, filename, FA_READ);
  while (f_gets(line, sizeof line, &fil)){
    cb(line, context);
  }
  f_close(&fil);
  return 0;
}

  int
arch_save(char* name, arch_save_cb cb, void* context)
{
  char filename[13];
  char* line;
  char buffer[128];
  FIL fil;
  snprintf(filename, sizeof(filename), "%s.bas", name);
  f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS); 
  for(;;){
    uint16_t number = cb(&line, context);
    if (line == NULL){
      break;
    }
    snprintf(buffer, sizeof(buffer), "%d %s\n", number, line);
    f_puts(buffer, &fil);
  }
  f_close(&fil);
  return 0;
}

  int
arch_dir(arch_dir_out_cb cb, void* context)
{
  DIR dir;
  FRESULT res;
  FILINFO fno;
  char out[24];
  f_getlabel("", out, 0);
  cb(out, 0, true, context);
  res = f_findfirst(&dir, &fno, "", "*.bas");
  while (res == FR_OK && fno.fname[0]){
    fno.fname[strlen(fno.fname)-4] = '\0';
    cb(fno.fname, (size_t) fno.fsize, false, context);
    res = f_findnext(&dir, &fno);
  }
  f_closedir(&dir);
  return 0;
}

  int
arch_delete(char* name){
  char filename[13];
  snprintf(filename, sizeof(filename), "%s.bas", name);
  f_unlink(filename);
  return 0;
}


