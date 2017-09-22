#include "ModworksSDK.h"

namespace modworks
{
  void init(int game_id, string api_key)
  {
    clearLog();

    initCurl();

    writeLogLine("Initializing SDK", verbose);
    modworks::game_id = game_id;
    modworks::api_key = api_key;
    access_token = "";

    std::ifstream token_file(getModworksDirectory() + "token.json");
    if(token_file.is_open())
    {
      json token_file_json;
      token_file >> token_file_json;
      if(token_file_json.find("access_token") != token_file_json.end())
      {
        access_token = token_file_json["access_token"];
      }
    }

    createDirectory(getModworksDirectory());
    createDirectory(getModworksDirectory() + "images/");
    createDirectory(getModworksDirectory() + "tmp/");
    writeLogLine("SDK Initialized", verbose);
  }

  void init(int game_id, string api_key, string root_path)
  {
    ROOT_PATH = root_path;
    init(game_id, api_key);
  }

  void setDebugMode(DebugMode debug_mode)
  {
    DEBUG_LEVEL = debug_mode;
  }

  void pauseCurrentDownload()
  {
    curlPauseCurrentDownload();
  }
}
