#include <stdio.h>
#include <string.h>
#include <urlencode.h>

int main(int argc, const char** argv) {
  if (argc < 2) {
    fprintf(stderr, "CLI needs a string\n");
    return 1;
  }

  const char* input = argv[1];
  size_t input_size = strlen(input);
  char output1[input_size * 3 + 1];
  char output2[input_size * 3 + 1];
  char output3[input_size + 1];
  char output4[input_size + 1];
  urlencode::Encode(input, input_size, output1, true);
  urlencode::Encode(input, input_size, output2, false);
  urlencode::Decode(output1, strlen(output1), output3, true);
  urlencode::Decode(output2, strlen(output2), output4, false);

  printf("Encoded (space to plus): %s\n", output1);
  printf("Decoded: %s\n\n", output3);

  printf("Encoded: %s\n", output2);
  printf("Decoded: %s\n", output4);

  return 0;
}
