#include <stdio.h>

int main(int argc, char *argv[])
{
  const char *output_format;

  union
  {
    unsigned char  bytes[sizeof(int)];
    unsigned int   i32val;
    struct
    {
      unsigned short a;
      unsigned short b;
    } i16val;
  } test;

  if (argc == 2)
    output_format = argv[1];
  else
    output_format = "-D%s";

  if (sizeof(int) != sizeof(test))
  {
    fprintf(stderr, "Error: Size of int is %i; size of union is %i!\n", sizeof(int), sizeof(test));
    exit(1);
  }

  if (sizeof(int) != 4)
  {
    fprintf(stderr, "Error: Size of int is %i; expecting 4\n", sizeof(int));
    exit(2);
  }

  test.bytes[0] = (char)0x12;
  test.bytes[1] = (char)0x34;
  test.bytes[2] = (char)0x56;
  test.bytes[3] = (char)0x78;

  switch(test.i32val)
  {
    case 0x12345678:
      printf(output_format, "BIG_ENDIAN");
      break;
    case 0x56781234:
      printf(output_format, "-DLITTLE_ENDIAN");
      break;
    default:
      fprintf(stderr, "Error: Unknown endianness; %02x,%02x,%02x,%02x = %08x\n", 
	test.bytes[0], test.bytes[1], test.bytes[2], test.bytes[3], test.i32val);
      exit(3);
  }
  
  return 0;
}

