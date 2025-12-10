#ifndef SD_CHECK_H_
#define SD_CHECK_H_

#include <stddef.h>

class Print;

namespace sd_check {

void begin(int csPin);
void update();
bool fileExists(const char* path);
bool readFile(const char* path, Print& out, size_t maxBytes = 0);

}  // namespace sd_check

#endif  // SD_CHECK_H_
