#ifndef __TVSCRAPER_TVDBSCRAPER_H
#define __TVSCRAPER_TVDBSCRAPER_H

using namespace std;

// --- cTVDBScraper -------------------------------------------------------------

class cTVDBScraper {
    friend class cTVDBSeries;
private:
    const char *baseURL4 = "https://api4.thetvdb.com/v4/";
    const char *baseURL4Search = "https://api4.thetvdb.com/v4/search?type=series&query=";
    std::string tokenHeader;
    time_t tokenHeaderCreated = 0;
    cTVScraperDB *db;
    int CallRestJson(rapidjson::Document &document, const rapidjson::Value *&data, cLargeString &buffer, const char *url, bool disableLog = false);
    bool GetToken(std::string &jsonResponse);
    bool AddResults4(cLargeString &buffer, vector<searchResultTvMovie> &resultSet, std::string_view SearchString, const std::vector<cNormedString> &normedStrings, const cLanguage *lang);
    void ParseJson_searchSeries(const rapidjson::Value &data, vector<searchResultTvMovie> &resultSet, const std::vector<cNormedString> &normedStrings, const cLanguage *lang);

    static const char *prefixImageURL1;
    static const char *prefixImageURL2;
public:
    cTVDBScraper(cTVScraperDB *db);
    virtual ~cTVDBScraper(void);
    bool Connect(void);
    bool GetToken();
    int StoreSeriesJson(int seriesID, bool onlyEpisodes);
    int StoreSeriesJson(int seriesID, const cLanguage *lang);
    void StoreStill(int seriesID, int seasonNumber, int episodeNumber, const string &episodeFilename);
    void StoreActors(int seriesID);
    void UpdateTvRuntimes(int seriesID);
//    void GetTvVote(int seriesID, float &vote_average, int &vote_count);
    int GetTvScore(int seriesID);
    void DownloadMedia (int tvID);
    void DownloadMedia (int tvID, eMediaType mediaType, const string &destDir);
    void DownloadMediaBanner (int tvID, const string &destPath);
    bool AddResults4(vector<searchResultTvMovie> &resultSet, std::string_view SearchString, const cLanguage *lang);
    static const char *getDbUrl(const char *url);
    static std::string getFullDownloadUrl(const char *url);
    void Download4(const char *url, const std::string &localPath);
};
#endif //__TVSCRAPER_TVDBSCRAPER_H
