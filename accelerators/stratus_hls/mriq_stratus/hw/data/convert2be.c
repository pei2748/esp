#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

int main (int argc, char* argv[]) {

  if (argc != 3) {
    printf("usage: ./write_be in_file out_file\n");
    return 1;
  }

  FILE* in_file = fopen(argv[1], "r");
  FILE* out_file = fopen(argv[2], "w");

  uint8_t rd_data[4];
  int i;

  // in while, read from in_file to rd_data
  // if 10 00 00 00
  // then rd_data[0] = 00, rd_data[3] = 10
  while(fread(rd_data, sizeof(uint8_t), 4, in_file) == 4){
    for (i = 3; i>= 0; i--)
      fwrite(&rd_data[i], sizeof(uint8_t), 1, out_file);
  }



  fclose(in_file);
  fclose(out_file);
  return 0;
}





