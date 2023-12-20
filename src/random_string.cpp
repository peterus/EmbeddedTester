#include <algorithm>
#include <string.h>

#include <esp_random.h>

#include "random_string.h"

void get_random_string(String &output, unsigned int len) {
  char eligible_chars[] = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz1234567890";

  for (unsigned int i = 0; i < len - 1; i++) {
    unsigned int rand_int     = esp_random();
    unsigned int random_index = rand_int % (strlen(eligible_chars));

    output += eligible_chars[random_index];
  }
}
