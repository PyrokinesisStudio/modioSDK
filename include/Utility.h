#ifndef MODWORKS_UTILITY_H
#define MODWORKS_UTILITY_H

#include <iostream>
#include <vector>
#include <string.h>
#include <dirent.h>

#ifdef __linux__
#define LINUX
#endif

#ifdef __APPLE__
#define OSX
#endif
#ifdef __MACH__
#define OSX
#endif

#ifdef _WIN32
#define WINDOWS
#endif
#ifdef _WIN64
#define WINDOWS
#endif

#ifdef LINUX
#include <sys/stat.h>
#endif

#ifdef WINDOWS
#include <windows.h>
#endif

#include <fstream>

using namespace std;

namespace modworks
{
  string dataToJsonString(char* data, size_t size);
  string toString(int number);
  void createDirectory(string directory);
  bool writeLogLine(string text);
  vector<string> getFilenames(string directory);
}

#endif
