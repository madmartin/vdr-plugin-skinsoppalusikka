/*
 * logo.c: The 'soppalusikka' VDR skin
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#include "common.h"
#include "config.h"
#include "logo.h"
#include <vdr/tools.h>
#include <vdr/plugin.h>

cSoppalusikkaLogoCache SoppalusikkaLogoCache(0);

cSoppalusikkaLogoCache::cSoppalusikkaLogoCache(unsigned int cacheSizeP)
: cacheSizeM(cacheSizeP), bitmapM(NULL)
{
}

cSoppalusikkaLogoCache::~cSoppalusikkaLogoCache()
{
  // let's flush the cache
  Flush();
}

bool cSoppalusikkaLogoCache::Resize(unsigned int cacheSizeP)
{
  debug("cSoppalusikkaLogoCache::Resize(%d)", cacheSizeP);
  // flush cache only if it's smaller than before
  if (cacheSizeP < cacheSizeM) {
     Flush();
     }
  // resize current cache
  cacheSizeM = cacheSizeP;
  return true;
}

bool cSoppalusikkaLogoCache::Load(const char *fileNameP)
{
  char *fileName = strdup(fileNameP);
  if (!fileName)
     return false;
  // replace '/' characters with '~'
  strreplace(fileName, '/', '~');
  debug("cSoppalusikkaLogoCache::Load(%s)", fileName);
  // does the logo exist already in map
  std::map<std::string, cBitmap*>::iterator i = cacheMapM.find(fileName);
  if (i != cacheMapM.end()) {
     // yes - cache hit!
     debug("cSoppalusikkaLogoCache::Load() CACHE HIT!");
     // check if logo really exist
     if (i->second == NULL) {
        debug("cSoppalusikkaLogoCache::Load() EMPTY");
        // empty logo in cache
        free(fileName);
        return false;
        }
     bitmapM = i->second;
     }
  else {
     // no - cache miss!
     debug("cSoppalusikkaLogoCache::Load() CACHE MISS!");
     // try to load xpm logo
     LoadXpm(fileName);
     // check if cache is active
     if (cacheSizeM) {
        // update map
        if (cacheMapM.size() >= cacheSizeM) {
           // cache full - remove first
           debug("cSoppalusikkaLogoCache::Load() DELETE");
           if (cacheMapM.begin()->second != NULL) {
              // logo exists - delete it
              cBitmap *bmp = cacheMapM.begin()->second;
              DELETENULL(bmp);
              }
           // erase item
           cacheMapM.erase(cacheMapM.begin());
           }
        // insert logo into map
        debug("cSoppalusikkaLogoCache::Load() INSERT(%s)", fileName);
        cacheMapM.insert(std::make_pair(fileName, bitmapM));
        }
     // check if logo really exist
     if (bitmapM == NULL) {
        debug("cSoppalusikkaLogoCache::Load() EMPTY");
        // empty logo in cache
        free(fileName);
        return false;
        }
     }
  free(fileName);
  return true;
}

cBitmap& cSoppalusikkaLogoCache::Get(void)
{
  return *bitmapM;
}

bool cSoppalusikkaLogoCache::LoadXpm(const char *fileNameP)
{
  struct stat stbuf;
  cBitmap *bmp = new cBitmap(1, 1, 1);

  // create absolute filename
  cString filename = cString::sprintf("%s/%s.xpm", SoppalusikkaConfig.GetLogoDir(), fileNameP);
  debug("cSoppalusikkaLogoCache::LoadXpm(%s)", *filename);
  // check validity
  if ((stat(*filename, &stbuf) == 0) && bmp->LoadXpm(*filename)) {
     if ((bmp->Width() == ChannelLogoWidth) && (bmp->Height() == ChannelLogoHeight)) {
        debug("cSoppalusikkaLogoCache::LoadXpm() LOGO FOUND");
        // assign bitmap
        bitmapM = bmp;
        return true;
        }
     else
        error("Invalid logo resolution in '%s'", *filename);
     }
  // no valid xpm logo found - delete bitmap
  debug("cSoppalusikkaLogoCache::LoadXpm() LOGO NOT FOUND OR INVALID RESOLUTION");
  delete bmp;
  bitmapM = NULL;
  return false;
}

bool cSoppalusikkaLogoCache::Flush(void)
{
  debug("cSoppalusikkaLogoCache::Flush()");
  // check if map is empty
  if (!cacheMapM.empty()) {
     debug("cSoppalusikkaLogoCache::Flush() NON-EMPTY");
     // delete bitmaps and clear map
     for (std::map<std::string, cBitmap*>::iterator i = cacheMapM.begin(); i != cacheMapM.end(); ++i) {
         cBitmap *bmp = i->second;
         if (bmp)
            DELETENULL(bmp);
         cacheMapM.erase(i);
         }
     // nullify bitmap pointer
     bitmapM = NULL;
     }
  return true;
}

