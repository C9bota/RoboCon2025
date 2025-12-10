#ifndef SD_DIAGNOSTICS_H_
#define SD_DIAGNOSTICS_H_

#include <stddef.h>

class Print;
class SdFat;

namespace sd_diag {

void begin(int csPin);
void configureTestFile(const char* path, size_t previewBytes);
void update();

bool isReady();
bool fileExists(const char* path);
bool readFile(const char* path, Print& out, size_t maxBytes = 0);
bool testMidiLoad(const char* path);

SdFat* getSdFat();

}  // namespace sd_diag

#endif  // SD_DIAGNOSTICS_H_
