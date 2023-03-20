#include <string>
#include <sstream>
#include <vector>
#include <map>
#include "thetvdbscraper.h"

using namespace std;

cTVDBScraper::cTVDBScraper(string baseDir, cTVScraperDB *db) {
    baseURL4 = "https://api4.thetvdb.com/v4/";
    baseURL4Search = "https://api4.thetvdb.com/v4/search?type=series&query=";
    this->baseDir = baseDir;
    this->db = db;
}

cTVDBScraper::~cTVDBScraper() {
}

bool cTVDBScraper::Connect(void) {
  return true;
}

bool cTVDBScraper::GetToken(void) {
// return true on success
  if (time(0) - tokenHeaderCreated < 24*60*60) return true; // call only once a day
  const char *url = "https://api4.thetvdb.com/v4/login";
  std::string buffer;

  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "accept: application/json");
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, "charset: utf-8");
  bool result =  CurlPostUrl(url, "{\"apikey\": \"5476e702-85aa-45fd-a8da-e74df3840baf\"}", buffer, headers);
  curl_slist_free_all(headers);
  if (!result) {
    esyslog("tvscraper: ERROR cTVDBScraper::GetToken, calling %s", url);
    return false;
  }
// now read the tocken
  if (!GetToken(buffer)) {
    esyslog("tvscraper: ERROR cTVDBScraper::GetToken, parsing json result %s", buffer.erase(40).c_str() );
    return false;
  }
  return true;
}

bool cTVDBScraper::GetToken(const std::string &jsonResponse) {
  json_t *root = json_loads(jsonResponse.c_str(), 0, NULL);
  if (!root) return false;
  if (!json_is_object(root)) { json_decref(root); return false; }
  if (json_string_value_validated(root, "status").compare("success") != 0) {
    esyslog("tvscraper: ERROR getting thetvdb token, status = %s", json_string_value_validated(root, "status").c_str() );
    json_decref(root);
    return false;
  }
  json_t *jData = json_object_get(root, "data");
  if(!json_is_object(jData)) { json_decref(root); return false; }
  tokenHeader = "Authorization: Bearer ";
  tokenHeader.append(json_string_value_validated(jData, "token") );
  tokenHeaderCreated = time(0);
  json_decref(root);
  return true;
}

void getCharacters(json_t *jData, cTVDBSeries &series) {
  json_t *jCharacters = json_object_get(jData, "characters");
  if (json_is_array(jCharacters)) {
    size_t index;
    json_t *jCharacter;
    json_array_foreach(jCharacters, index, jCharacter) {
      series.ParseJson_Character(jCharacter);
    }
  }
}

int cTVDBScraper::StoreSeriesJson(int seriesID, bool onlyEpisodes) {
// return 0 in case of error
// return -1 if the object does not exist in external db
// otherwise, return seriesID > 0
// only update if not yet in db
// we ignore onlyEpisodes, there is no real performance improvement, as episode related information like gouest stars is in "main" series
// except for the check if we need to update: if onlyEpisodes == true, we update, even if we have aready data
  if (seriesID == 0) return 0;
  int ns, ne;
  if (!onlyEpisodes && db->TvGetNumberOfEpisodes(seriesID * (-1), ns, ne)) return seriesID; // already in db

  cTVDBSeries series(db, this, seriesID);
// this call gives also episode related information like actors, images, ...
// Change: season images type 6 ("Banner") & 7 ("Poster") still included
// Episode Guest stars, writer, ... NOT included any more
// Episode Guest stars, writer, ... NOT available in https://api4.thetvdb.com/v4/seasons/1978231/extended
//
  CONCATENATE(url, baseURL4, "series/", seriesID, "/extended?meta=translations&short=false");
  int error;
  cLargeString buffer("cTVDBScraper::StoreSeriesJson", 15000);
  json_t *jSeries = CallRestJson(url, buffer, &error);
  if (!jSeries) {
    if (error == -1) return -1; // object does not exist
    return 0;
  }
  json_t *jSeriesData = json_object_get(jSeries, "data");
  if (!jSeriesData) { json_decref(jSeries); return 0;}
  series.ParseJson_Series(jSeriesData);
// episodes
  string urlE = concatenate(baseURL4, "series/", seriesID, "/episodes/default/", series.m_language, "?page=0");
  while (!urlE.empty() ) {
    json_t *jEpisodes = CallRestJson(urlE.c_str(), buffer);
    urlE = "";
    if (!jEpisodes) break;
    json_t *jEpisodesData = json_object_get(jEpisodes, "data");
    if (jEpisodesData) {
// parse episodes
      json_t *jEpisodesDataEpisodes = json_object_get(jEpisodesData, "episodes");
      if (json_is_array(jEpisodesDataEpisodes)) {
        size_t index;
        json_t *jEpisode;
        json_array_foreach(jEpisodesDataEpisodes, index, jEpisode) {
          series.ParseJson_Episode(jEpisode);
/*
// dont't read episode character data here, this is too often called just to figure out if we have the right series
          int epidodeID = series.ParseJson_Episode(jEpisode);
          if (epidodeID != 0) {
            string urlEp = baseURL4 + "episodes/" + std::to_string(epidodeID) + "/extended";
            json_t *jEpisode = CallRestJson(urlEp, buffer, NULL, true);
            json_t *jEpisodeData = json_object_get(jEpisode, "data");
            getCharacters(jEpisodeData, series);
            json_decref(jEpisode);
          }
*/
        }
      }
    }
    json_t *jLinks = json_object_get(jEpisodes, "links");
    if (json_is_object(jLinks)) urlE = json_string_value_validated(jLinks, "next");
    json_decref(jEpisodes);
  }
// characters / actors
// we also add characters to episodes. Therefore, we do this after parsing the episodes
  getCharacters(jSeriesData, series);
// we also add season images. Therefore, we do this after parsing the episodes
  series.ParseJson_Artwork(jSeriesData);
// store series here, as here information (incl. episode runtimes, poster URL, ...) is complete
  series.StoreDB();
  db->TvSetEpisodesUpdated(seriesID * (-1) );
  if (jSeries) json_decref(jSeries);
  return seriesID;
}

int cTVDBScraper::StoreSeriesJson(int seriesID, const cLanguage *lang) {
// return 0 in case of error
// only update if required (there are less episodes in this language than episodes in default language
  if (seriesID == 0) return 0;
  if (!db->episodeNameUpdateRequired(seriesID * (-1), lang->m_id)) return seriesID;

  cTVDBSeries series(db, this, seriesID);
// episodes
  cLargeString buffer("cTVDBScraper::StoreSeriesJson lang", 10000);
  string urlE = concatenate(baseURL4, "series/", seriesID, "/episodes/default/", lang->m_thetvdb, "?page=0");
  while (!urlE.empty() ) {
    json_t *jEpisodes = CallRestJson(urlE.c_str(), buffer);
    urlE = "";
    if (!jEpisodes) break;
    json_t *jEpisodesData = json_object_get(jEpisodes, "data");
    if (jEpisodesData) {
// parse episodes
      json_t *jEpisodesDataEpisodes = json_object_get(jEpisodesData, "episodes");
      if (json_is_array(jEpisodesDataEpisodes)) {
        size_t index;
        json_t *jEpisode;
        json_array_foreach(jEpisodesDataEpisodes, index, jEpisode) {
          series.ParseJson_Episode(jEpisode, lang);
        }
      }
    }
    json_t *jLinks = json_object_get(jEpisodes, "links");
    if (json_is_object(jLinks)) urlE = json_string_value_validated(jLinks, "next");
    json_decref(jEpisodes);
  }
  return seriesID;
}

json_t *cTVDBScraper::CallRestJson(const char *url, cLargeString &buffer, int *error, bool disableLog) {
// return NULL in case of errors
// if error is given, it will be set to -1 if an object is requested which does not exist
// otherwise, the caller must ensure to call json_decref(...); on the returned reference
  if (error) *error = 0;
  if (!GetToken() ) return NULL;
  buffer.clear();
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "accept: application/json");
  headers = curl_slist_append(headers, tokenHeader.c_str() );
  headers = curl_slist_append(headers, "charset: utf-8");
  if (config.enableDebug && !disableLog) esyslog("tvscraper: calling %s", url);
  bool result = CurlGetUrl(url, buffer, headers);
  curl_slist_free_all(headers);
  if (!result) {
    esyslog("tvscraper: ERROR calling %s", url);
    return NULL;
  }
  json_t *jRoot = json_loads(buffer.c_str(), 0, NULL);
  if (!jRoot) {
    esyslog("tvscraper: ERROR cTVDBScraper::CallRestJson, url %s, buffer %s", url, buffer.erase(50).c_str());
    return NULL;
  }
  std::string status = json_string_value_validated(jRoot, "status");
  if (status != "success") {
    if (status == "failure" && json_string_value_validated(jRoot, "message") == "Not Found") {
      if (error) *error = -1;
    }
    json_decref(jRoot);
// note: if "error" is provided, the program will handle this situation. No need to report an error in syslog
    if (error) esyslog("tvscraper: cTVDBScraper::CallRestJson, url %s, buffer %s, status = %s", url, buffer.erase(50).c_str(), status.c_str() );
    else esyslog("tvscraper: ERROR cTVDBScraper::CallRestJson, url %s, buffer %s, status = %s", url, buffer.erase(50).c_str(), status.c_str() );
    return NULL;
  }
  if (!json_object_get(jRoot, "data")) {
    json_decref(jRoot);
    esyslog("tvscraper: ERROR cTVDBScraper::CallRestJson, data is NULL, url %s, buffer %s", url, buffer.erase(50).c_str());
    return NULL;
  }
  return jRoot;
}

// methods to download / store media

const char *cTVDBScraper::prefixImageURL1 = "https://artworks.thetvdb.com/banners/";
const char *cTVDBScraper::prefixImageURL2 = "https://artworks.thetvdb.com";

const char *cTVDBScraper::getDbUrl(const char *url) {
  if (!url || !*url) return url;
  const char *s = removePrefix(url, cTVDBScraper::prefixImageURL1);
  if (s) return s;
  s = removePrefix(url, "/banners/");
  if (s) return s;
  return url;
}

std::string cTVDBScraper::getFullDownloadUrl(const char *url) {
// return std::string("https://thetvdb.com/banners/") + imageUrl;
  if (!url || !*url) return "";
  if (strncmp(url, cTVDBScraper::prefixImageURL2, strlen(cTVDBScraper::prefixImageURL2) ) == 0) return url;
  if (url[0] == '/') return concatenate(cTVDBScraper::prefixImageURL2, url);
  return concatenate(cTVDBScraper::prefixImageURL1, url);  // for URLs returned by APIv3
}

void cTVDBScraper::Download4(const char *url, const std::string &localPath) {
  Download(cTVDBScraper::getFullDownloadUrl(url), localPath);
}

void cTVDBScraper::StoreActors(int seriesID) {
  std::string destDir = concatenate(baseDir, "/", seriesID);
//  if (config.enableDebug) esyslog("tvscraper: cTVDBScraper::StoreActors, seriesID %i destDir %s", seriesID, destDir.c_str());
  if (!CreateDirectory(destDir)) return;
  destDir += "/actor_";
  const char *sql = "select actor_id, actor_path from actor_download where movie_id = ? and is_movie = ?";
  for (cSql &stmt: cSql(db, sql, seriesID * -1, false)) {
    const char *actor_path = stmt.getCharS(1);
    if (!actor_path || !*actor_path) continue;
    Download4(actor_path, concatenate(destDir, stmt.getInt(0), ".jpg"));
  }
  db->DeleteActorDownload (seriesID * -1, false);
}
void cTVDBScraper::StoreStill(int seriesID, int seasonNumber, int episodeNumber, const string &episodeFilename) {
    if (episodeFilename.empty() ) return;
    std::string destDir = concatenate(baseDir, "/", seriesID, "/");
    bool ok = CreateDirectory(destDir);
    if (!ok) return;
    std::string destDir2 = concatenate(destDir, seasonNumber, "/");
    ok = CreateDirectory(destDir2);
    if (!ok) return;
    std::string pathStill = concatenate(destDir2, "still_", episodeNumber, ".jpg");
    Download4(episodeFilename.c_str(), pathStill);
}
void cTVDBScraper::UpdateTvRuntimes(int seriesID) {
  StoreSeriesJson(seriesID, true);
}
int cTVDBScraper::GetTvScore(int seriesID) {
  const char *sql = "select tv_score from tv_score where tv_id = ?";
  int score = db->queryInt(sql, seriesID * -1);
  if (score != 0) return score;
  if (config.enableDebug) esyslog("tvscraper: cTVDBScraper::GetTvScore, score %i, seriesID %i", score, seriesID);
  StoreSeriesJson(seriesID, true);
  score = db->queryInt(sql, seriesID * -1);
  if (score == 0) esyslog("tvscraper: ERROR cTVDBScraper::GetTvScore, score %i, seriesID %i", score, seriesID);
  return score;
}

void cTVDBScraper::DownloadMedia (int tvID) {
//stringstream destDir;
//destDir << baseDir << "/" << tvID << "/";
  std::string destDir = concatenate(baseDir, "/", tvID, "/");
  if (!CreateDirectory(destDir)) return;

  DownloadMedia (tvID, mediaPoster, destDir + "poster_");
  DownloadMedia (tvID, mediaFanart, destDir + "fanart_");
  DownloadMedia (tvID, mediaSeason, destDir + "season_poster_");
  DownloadMediaBanner (tvID, destDir + "banner.jpg");
  db->deleteTvMedia (tvID * -1, false, true);
}

void cTVDBScraper::DownloadMedia (int tvID, eMediaType mediaType, const string &destDir) {
//  if (config.enableDebug) esyslog("tvscraper: cTVDBScraper::DownloadMedia, tvID %i mediaType %i destDir %s", tvID, mediaType, destDir.c_str());
  const char *sql = "select media_path, media_number from tv_media where tv_id = ? and media_type = ? and media_number >= 0";
  for (cSql &stmt: cSql(db, sql, tvID * -1, (int)mediaType)) {
//    if (config.enableDebug) esyslog("tvscraper: cTVDBScraper::DownloadMedia, media[0] %s media[1] %s", media[0].c_str(), media[1].c_str() );
    const char *media_path = stmt.getCharS(0);
    if (!media_path || !*media_path ) continue;
    Download4(media_path, concatenate(destDir, stmt.getInt(1), ".jpg"));
  }
}

void cTVDBScraper::DownloadMediaBanner (int tvID, const string &destPath) {
  const char *sql = "select media_path from tv_media where tv_id = ? and media_type = ? and media_number >= 0";
  for (cSql &stmt: cSql(db, sql, tvID * -1, (int)mediaBanner)) {
    const char *media_path = stmt.getCharS(0);
    if (!media_path || !*media_path ) continue;
    Download4(media_path, destPath);
    return;
  }
}

// Search series
bool cTVDBScraper::AddResults4(vector<searchResultTvMovie> &resultSet, std::string_view SearchString, const cLanguage *lang) {
// search for tv series, add search results to resultSet
// return true if a result matching searchString was found, and no splitting of searchString was required
// otherwise, return false. Note: also in this case some results might have been added

  string_view searchString1, searchString2, searchString3, searchString4;
  std::vector<cNormedString> normedStrings = getNormedStrings(SearchString, searchString1, searchString2, searchString3, searchString4);
  cLargeString buffer("cTVDBScraper::AddResults4", 500, 5000);
  size_t size0 = resultSet.size();
  AddResults4(buffer, resultSet, SearchString, normedStrings, lang);
  if (resultSet.size() > size0) {
    for (size_t i = size0; i < resultSet.size(); i++)
      if (sentence_distance_normed_strings(normedStrings[0].m_normedString, resultSet[i].normedName) < 600) return true;
    return false;
  }
  if (!searchString1.empty() ) AddResults4(buffer, resultSet, searchString1, normedStrings, lang);
  if (resultSet.size() > size0) return false;
  if (!searchString2.empty() ) AddResults4(buffer, resultSet, searchString2, normedStrings, lang);
  if (resultSet.size() > size0) return false;
  if (searchString4.empty() ) return false;
// searchString3 is the string before ' - '. We will search for this later, as part of the <title> - <episode> pattern
  AddResults4(buffer, resultSet, searchString4, normedStrings, lang);
  return false;
}

bool cTVDBScraper::AddResults4(cLargeString &buffer, vector<searchResultTvMovie> &resultSet, std::string_view SearchString, const std::vector<cNormedString> &normedStrings, const cLanguage *lang) {
//  string SearchString_rom = removeRomanNum(SearchString.data(), SearchString.length());
  CONVERT(SearchString_rom, SearchString, removeRomanNumC);
  if (*SearchString_rom == 0) {
    esyslog("tvscraper: ERROR cTVDBScraper::AddResults4, SearchString_rom == empty");
    return false;
  }
  CURLESCAPE(url_e, SearchString_rom);
  CONCATENATE(url, baseURL4Search, url_e);
  json_t *root = CallRestJson(url, buffer);
  if (!root) return false;
  int seriesID = 0;
  bool result = ParseJson_search(root, resultSet, normedStrings, lang);
  json_decref(root);
  if (!result) {
    esyslog("tvscraper: ERROR cTVDBScraper::AddResults4, !result, url %s", url);
    return false;
  }
  if(seriesID == 0) return false;
  return true;
}

bool cTVDBScraper::ParseJson_search(json_t *root, vector<searchResultTvMovie> &resultSet, const std::vector<cNormedString> &normedStrings, const cLanguage *lang) {
  if (root == NULL) return false;
  json_t *jData = json_object_get(root, "data");
  if(!json_is_array(jData))  {
    esyslog("tvscraper: ERROR cTVDBScraper::ParseJson_search, parsing thetvdb search result, jData is not an array");
    return false;
  }
  size_t index;
  json_t *jElement;
  json_array_foreach(jData, index, jElement) {
    ParseJson_searchSeries(jElement, resultSet, normedStrings, lang);
  }
  return true;
}

int minDist(int dist, const json_t *jString, const string &SearchStringStripExtraUTF8, std::string *normedName = NULL) {
// compare string in jString with SearchStringStripExtraUTF8
// make sanity checks first
  if (!jString || !json_is_string(jString)) return dist;
  const char *name = json_string_value(jString);
  if (!name || !*name) return dist;

  if (normedName) {
    *normedName = normString(name);
    dist = std::min(dist, sentence_distance_normed_strings(*normedName, SearchStringStripExtraUTF8) );
  } else
    dist = std::min(dist, sentence_distance_normed_strings(normString(name), SearchStringStripExtraUTF8) );
  int len = StringRemoveLastPartWithP(name, (int)strlen(name) );
  if (len != -1) {
    if (normedName) *normedName = normString(name, len);
    dist = std::min(dist, sentence_distance_normed_strings(normString(name, len), SearchStringStripExtraUTF8) );
  }
  return dist;
}

void cTVDBScraper::ParseJson_searchSeries(json_t *data, vector<searchResultTvMovie> &resultSet, const std::vector<cNormedString> &normedStrings, const cLanguage *lang) {// add search results to resultSet
  if (!data) return;
  std::string objectID = json_string_value_validated(data, "objectID");
  if (objectID.length() < 8) {
    esyslog("tvscraper: ERROR cTVDBScraper::ParseJson_searchSeries, objectID.length() < 8, %s", objectID.c_str() );
    return;
  }
  if (objectID.compare(0, 7, "series-") != 0) {
    esyslog("tvscraper: ERROR cTVDBScraper::ParseJson_searchSeries, objectID does not start with series-, %s", objectID.c_str() );
    return;
  }
  int seriesID = atoi(objectID.c_str() + 7);
  if (seriesID == 0) {
    esyslog("tvscraper: ERROR cTVDBScraper::ParseJson_searchSeries, seriesID = 0, %s", objectID.c_str() );
    return;
  }
// is this series already in the list?
  for (const searchResultTvMovie &sRes: resultSet ) if (sRes.id() == seriesID * (-1) ) return;

// create new result object sRes
  searchResultTvMovie sRes(seriesID * (-1), false, json_string_value_validated(data, "year"));
  sRes.setPositionInExternalResult(resultSet.size() );

// distance == deviation from search text
  int dist_a = 1000;
// if search string is not in original language, consider name (== original name) same as alias
  if (lang) dist_a = minDistanceNormedStrings(dist_a, normedStrings, json_string_value_validated_c(data, "name") );
  json_t *jAliases = json_object_get(data, "aliases");
  if (json_is_array(jAliases) ) {
    size_t index;
    json_t *jElement;
    json_array_foreach(jAliases, index, jElement) {
      dist_a = minDistanceNormedStrings(dist_a, normedStrings, json_string_value(jElement) );
    }
  }
// in search results, aliases don't have language information
// in series/<id>/extended, language information for aliases IS available
// still, effort using that seems to be too high
// we give a malus for the missing language information, similar to movies
  dist_a = std::min(dist_a + 50, 1000);
  int requiredDistance = 600; // "standard" require same text similarity as we required for episode matching
  if (lang) {
    json_t *jTranslations = json_object_get(data, "translations");
    if (json_is_object(jTranslations) ) {
      json_t *langVal = json_object_get(jTranslations, lang->m_thetvdb);
      if (langVal) {
        dist_a = minDistanceNormedStrings(dist_a, normedStrings, json_string_value(langVal), &sRes.normedName);
        requiredDistance = 700;  // translation in EPG language is available. Reduce requirement somewhat
      }
    }
  } else {
// name is the name in original / primary language
    dist_a = minDistanceNormedStrings(dist_a, normedStrings, json_string_value_validated_c(data, "name") );
  }
  if (dist_a < requiredDistance) {
    sRes.setMatchText(dist_a);
    resultSet.push_back(sRes);
  }
}
