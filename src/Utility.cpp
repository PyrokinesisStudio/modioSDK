#include "Utility.h"

namespace modio
{
//String methods
std::string toString(i32 number)
{
  if (number == 0)
    return "0";

  if (number < 0)
    return "-" + toString(-number);

  std::string temp = "";
  std::string returnvalue = "";
  while (number > 0)
  {
    temp += number % 10 + 48;
    number /= 10;
  }
  for (int i = 0; i < (int)temp.length(); i++)
    returnvalue += temp[temp.length() - i - 1];
  return returnvalue;
}

std::string toString(u32 number)
{
  return toString((i32)number);
}

std::string toString(double number)
{
  return std::to_string(number);
}

std::string replaceSubstrings(const std::string& str, const std::string& from, const std::string& to)
{
  std::string return_value = str;

  if (from == "")
    return return_value;

  size_t start_pos = 0;
  while ((start_pos = return_value.find(from, start_pos)) != std::string::npos)
  {
    return_value.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
  return return_value;
}

std::string addSlashIfNeeded(const std::string& directory_path)
{
  std::string return_value = directory_path;

  if (return_value != "" && return_value[return_value.size() - 1] != '/')
    return_value += "/";

  return return_value;
}

// Log methods

void writeLogLine(const std::string& text, u32 debug_level)
{
  // NOTE(@jackson): Lower value is higher severity (error == 0)
  if (DEBUG_LEVEL < debug_level)
    return;

  std::ofstream log_file(getModIODirectory() + "log", std::ios::app);
  log_file << "[" << modio::getCurrentTime() << "] ";
  if (debug_level == MODIO_DEBUGLEVEL_ERROR)
  {
    log_file << "[Error] ";
  }
  else if (debug_level == MODIO_DEBUGLEVEL_WARNING)
  {
    log_file << "[WARNING] ";
  }
  else if (debug_level == MODIO_DEBUGLEVEL_LOG)
  {
    log_file << "[LOG] ";
  }
  log_file << text.c_str() << "\n";
  log_file.close();
}

void clearLog()
{
  std::ofstream log_file(getModIODirectory() + "log");
  log_file.close();
}

// Time methods

u32 getCurrentTime()
{
  return (u32)std::time(nullptr);
}

// Json methods

bool hasKey(nlohmann::json json_object, const std::string& key)
{
  return json_object.find(key) != json_object.end() && !json_object[key].is_null();
}

nlohmann::json toJson(const std::string& json_str)
{
  if (json_str == "")
    return "{}"_json;

  nlohmann::json response_json;
  try
  {
    response_json = nlohmann::json::parse(json_str);
  }
  catch (nlohmann::json::parse_error &e)
  {
    writeLogLine(std::string("Error parsing json: ") + e.what(), MODIO_DEBUGLEVEL_ERROR);
    response_json = "{}"_json;
  }
  return response_json;
}

nlohmann::json openJson(const std::string& file_path)
{
  std::ifstream ifs(file_path);
  nlohmann::json cache_file_json;
  if (ifs.is_open())
  {
    try
    {
      ifs >> cache_file_json;
    }
    catch (nlohmann::json::parse_error &e)
    {
      modio::writeLogLine(std::string("Error parsing json: ") + e.what(), MODIO_DEBUGLEVEL_ERROR);
      cache_file_json = {};
    }
  }
  ifs.close();
  return cache_file_json;
}

void writeJson(const std::string& file_path, nlohmann::json json_object)
{
  std::ofstream ofs(file_path);
  ofs << std::setw(4) << json_object << std::endl;
  ofs.close();
}

// Filesystem methods

#ifdef MODIO_WINDOWS_DETECTED
void writeLastErrorLog(const std::string& error_function)
{
  //Get the error message, if any.
  DWORD errorMessageID = ::GetLastError();
  if (errorMessageID == 0)
    return; //No error message has been recorded

  if (errorMessageID == 183)
  {
    modio::writeLogLine("The directory already exists.", MODIO_DEBUGLEVEL_LOG);
    return;
  }

  LPSTR messageBuffer = nullptr;
  size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

  std::string message(messageBuffer, size);

  modio::writeLogLine("Error while using " + error_function + ": " + message, MODIO_DEBUGLEVEL_ERROR);

  //Free the buffer.
  LocalFree(messageBuffer);
}
#endif

void removeEmptyDirectory(const std::string& path)
{
#if defined(MODIO_LINUX_DETECTED) || defined(MODIO_OSX_DETECTED)
  if (remove(path.c_str()))
    writeLogLine(path + " removed", MODIO_DEBUGLEVEL_LOG);
  else
    writeLogLine("Could not remove " + path, MODIO_DEBUGLEVEL_ERROR);
#endif

#ifdef MODIO_WINDOWS_DETECTED
  if (!RemoveDirectory(path.c_str()))
    writeLastErrorLog("RemoveDirectory");
#endif
}

#ifdef MODIO_WINDOWS_DETECTED
int deleteDirectoryWindows(const std::string &refcstrRootDirectory)
{
  HANDLE hFile;                    // Handle to directory
  std::string strFilePath;         // Filepath
  std::string strPattern;          // Pattern
  WIN32_FIND_DATA FileInformation; // File information

  strPattern = refcstrRootDirectory + "\\*.*";
  hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
  if (hFile != INVALID_HANDLE_VALUE)
  {
    do
    {
      if (FileInformation.cFileName[0] != '.')
      {
        strFilePath.erase();
        strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;

        if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
          // Delete subdirectory
          int iRC = deleteDirectoryWindows(strFilePath);
          if (iRC)
            return iRC;
        }
        else
        {
          // Set file attributes
          if (::SetFileAttributes(strFilePath.c_str(),
                                  FILE_ATTRIBUTE_NORMAL) == FALSE)
            return ::GetLastError();

          // Delete file
          if (::DeleteFile(strFilePath.c_str()) == FALSE)
            return ::GetLastError();
        }
      }
    } while (::FindNextFile(hFile, &FileInformation) == TRUE);

    // Close handle
    ::FindClose(hFile);

    DWORD dwError = ::GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
      return dwError;
    else
    {
      // Set directory attributes
      if (::SetFileAttributes(refcstrRootDirectory.c_str(),
                              FILE_ATTRIBUTE_NORMAL) == FALSE)
        return ::GetLastError();

      // Delete directory
      if (::RemoveDirectory(refcstrRootDirectory.c_str()) == FALSE)
        return ::GetLastError();
    }
  }
  return 0;
}
#endif

std::string getModIODirectory()
{
  return modio::addSlashIfNeeded(ROOT_PATH) + ".modio/";
}

bool isDirectory(const std::string& directory)
{
  return opendir(modio::addSlashIfNeeded(directory).c_str());
}

bool fileExists(const std::string& directory)
{
  return check_file_exists(directory.c_str());
}

std::vector<std::string> getFilenames(const std::string& directory)
{
  std::string directory_with_slash = modio::addSlashIfNeeded(directory);

  std::vector<std::string> filenames;

  struct dirent *ent;
  DIR *dir;
  if ((dir = opendir(directory_with_slash.c_str())) != NULL)
  {
    while ((ent = readdir(dir)) != NULL)
    {
      DIR *current_dir;
      std::string current_file_path = directory_with_slash + ent->d_name;
      if ((current_dir = opendir(current_file_path.c_str())) != NULL && strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
      {
        std::vector<std::string> subdirectories_filenames = getFilenames(directory_with_slash + ent->d_name);
        for (int i = 0; i < (int)subdirectories_filenames.size(); i++)
        {
          filenames.push_back(std::string(ent->d_name) + "/" + subdirectories_filenames[i]);
        }
        closedir(current_dir);
      }
      else if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
      {
        filenames.push_back(ent->d_name);
      }
    }
    closedir(dir);
  }
  return filenames;
}

void createDirectory(const std::string& directory)
{
  writeLogLine("Creating directory " + directory, MODIO_DEBUGLEVEL_LOG);
#if defined(MODIO_LINUX_DETECTED) || defined(MODIO_OSX_DETECTED)
  mkdir(directory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

#ifdef MODIO_WINDOWS_DETECTED
  if (!CreateDirectory((char *)directory.c_str(), NULL))
    writeLastErrorLog("CreateDirectory");
#endif
}

bool removeDirectory(const std::string& directory)
{
#ifdef MODIO_WINDOWS_DETECTED
  int error_code = deleteDirectoryWindows(directory);
  if (error_code != 0)
    modio::writeLogLine("Could not remove directory, error code: " + modio::toString(error_code), MODIO_DEBUGLEVEL_ERROR);
  return error_code == 0;
#endif

  DIR *dir;
  struct dirent *entry;
  char path[PATH_MAX];

  std::string directory_with_slash = modio::addSlashIfNeeded(directory);

  dir = opendir(directory_with_slash.c_str());
  if (dir == NULL)
  {
    writeLogLine("Error opendir()", MODIO_DEBUGLEVEL_LOG);
    return false;
  }

  while ((entry = readdir(dir)) != NULL)
  {
    if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
    {
      snprintf(path, (size_t)PATH_MAX, "%s%s", directory_with_slash.c_str(), entry->d_name);
      if (opendir(path) != NULL)
      {
        removeDirectory(path);
      }
      writeLogLine("Deleting: " + std::string(path), MODIO_DEBUGLEVEL_LOG);
      removeFile(path);
    }
  }
  closedir(dir);
  writeLogLine("Deleting: " + directory_with_slash, MODIO_DEBUGLEVEL_LOG);
  removeEmptyDirectory(directory_with_slash);

  return true;
}

void removeFile(const std::string& filename)
{
  if (remove(filename.c_str()) != 0)
    writeLogLine("Could not remove " + filename, MODIO_DEBUGLEVEL_ERROR);
  else
    writeLogLine(filename + " removed", MODIO_DEBUGLEVEL_LOG);
}

double getFileSize(const std::string& file_path)
{
  double file_size = 0;
  FILE *fp = fopen(file_path.c_str(), "rb");
  if (fp)
  {
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    file_size = ftell(fp);
    fclose(fp);
  }
  return file_size;
}

void createPath(const std::string& path)
{
  std::string current_path;
  std::string tokenized_path = path;
  u32 slash_position;

  while (tokenized_path.length())
  {
    slash_position = (int)tokenized_path.find('/');
    if (slash_position == (u32)std::string::npos)
      break;
    current_path += tokenized_path.substr(0, slash_position) + "/";
    tokenized_path.erase(tokenized_path.begin(), tokenized_path.begin() + slash_position + 1);
    createDirectory(current_path);
  }
}
} // namespace modio
