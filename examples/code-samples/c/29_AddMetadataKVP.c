#include "modio_c.h"

void onAddMetadataKVP(void *object, ModioResponse response)
{
  bool *wait = object;
  printf("Add Metdata KVP response: %i\n", response.code);
  if (response.code == 201)
  {
    printf("Metadata KVP added successfully!\n");
  }
  *wait = false;
}

int main(void)
{
  modioInit(MODIO_ENVIRONMENT_TEST, 7, (char *)"e91c01b8882f4affeddd56c96111977b", NULL);

  bool wait = true;

  // Let's start by requesting a single mod
  printf("Please enter the mod id: \n");
  u32 mod_id;
  scanf("%i", &mod_id);

  printf("Adding metadata kvp...\n");

  char **metadata_kvp_array = (char **)malloc(1);
  metadata_kvp_array[0] = (char *)malloc(100);
  strcpy(metadata_kvp_array[0], "pistol-dmg:800\0");

  // We add metadata key value pairs to a mod by providing the key and the value on a string separated by a colon :
  modioAddMetadataKVP(&wait, mod_id, (char **)metadata_kvp_array, 1, &onAddMetadataKVP);

  while (wait)
  {
    modioProcess();
  }

  modioShutdown();

  printf("Process finished\n");

  return 0;
}
