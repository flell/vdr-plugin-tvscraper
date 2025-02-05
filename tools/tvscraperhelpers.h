#ifndef __TVSCRAPERHELPERS_H
#define __TVSCRAPERHELPERS_H

// years and stringhelper functions specific for tvscraper

#include <cstdarg>
#include <string>
#include <string.h>
#include <vector>
#include <iostream>
#include <chrono>
#include "stringhelpers.h"

#define CONVERT(result, from, fn) \
char result[fn(NULL, from) + 1]; \
fn(result, from);

#define CV_VA_NUM_ARGS_HELPER(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...)    N
#define CV_VA_NUM_ARGS(...)      CV_VA_NUM_ARGS_HELPER(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define CAT2( A, B ) A ## B
#define SELECT( NAME, NUM ) CAT2( NAME ## _, NUM )
#define VA_SELECT( NAME, ... ) SELECT( NAME, CV_VA_NUM_ARGS(__VA_ARGS__) )(__VA_ARGS__)

// concatenate macro based  ===========================
// deprectaed, use cToSvConcat, which is almost as fast
#define CONCATENATE_START_2(result, s1, s2) \
int result##concatenate_lvls = 0; \
int result##concatenate_lvl1 = ns_concat::numChars(s1); \
result##concatenate_lvls += result##concatenate_lvl1; \
int result##concatenate_lvl2 = ns_concat::numChars(s2); \
result##concatenate_lvls += result##concatenate_lvl2;

#define CONCATENATE_START_3(result, s1, s2, s3) \
CONCATENATE_START_2(result, s1, s2) \
int result##concatenate_lvl3 = ns_concat::numChars(s3); \
result##concatenate_lvls += result##concatenate_lvl3;

#define CONCATENATE_START_4(result, s1, s2, s3, s4) \
CONCATENATE_START_3(result, s1, s2, s3) \
int result##concatenate_lvl4 = ns_concat::numChars(s4); \
result##concatenate_lvls += result##concatenate_lvl4;

#define CONCATENATE_START_5(result, s1, s2, s3, s4, s5) \
CONCATENATE_START_4(result, s1, s2, s3, s4) \
int result##concatenate_lvl5 = ns_concat::numChars(s5); \
result##concatenate_lvls += result##concatenate_lvl5;

#define CONCATENATE_START_6(result, s1, s2, s3, s4, s5, s6) \
CONCATENATE_START_5(result, s1, s2, s3, s4, s5) \
int result##concatenate_lvl6 = ns_concat::numChars(s6); \
result##concatenate_lvls += result##concatenate_lvl6;

#define CONCATENATE_START_7(result, s1, s2, s3, s4, s5, s6, s7) \
CONCATENATE_START_6(result, s1, s2, s3, s4, s5, s6) \
int result##concatenate_lvl7 = ns_concat::numChars(s7); \
result##concatenate_lvls += result##concatenate_lvl7;

#define CONCATENATE_START_8(result, s1, s2, s3, s4, s5, s6, s7, s8) \
CONCATENATE_START_7(result, s1, s2, s3, s4, s5, s6, s7) \
int result##concatenate_lvl8 = ns_concat::numChars(s8); \
result##concatenate_lvls += result##concatenate_lvl8;

#define CONCATENATE_START_9(result, s1, s2, s3, s4, s5, s6, s7, s8, s9) \
CONCATENATE_START_8(result, s1, s2, s3, s4, s5, s6, s7, s8) \
int result##concatenate_lvl9 = ns_concat::numChars(s9); \
result##concatenate_lvls += result##concatenate_lvl9;

#define CONCATENATE_END_ADDCHARS_B(result_concatenate_buf, lvl, s) \
ns_concat::addChars(result_concatenate_buf, lvl, s); \
result_concatenate_buf += lvl;

#define CONCATENATE_END_2(result, s1, s2) \
char *result##concatenate_buf = result; \
CONCATENATE_END_ADDCHARS_B(result##concatenate_buf, result##concatenate_lvl1, s1); \
CONCATENATE_END_ADDCHARS_B(result##concatenate_buf, result##concatenate_lvl2, s2);

#define CONCATENATE_END_3(result, s1, s2, s3) \
CONCATENATE_END_2(result, s1, s2) \
CONCATENATE_END_ADDCHARS_B(result##concatenate_buf, result##concatenate_lvl3, s3);

#define CONCATENATE_END_4(result, s1, s2, s3, s4) \
CONCATENATE_END_3(result, s1, s2, s3) \
CONCATENATE_END_ADDCHARS_B(result##concatenate_buf, result##concatenate_lvl4, s4);

#define CONCATENATE_END_5(result, s1, s2, s3, s4, s5) \
CONCATENATE_END_4(result, s1, s2, s3, s4) \
CONCATENATE_END_ADDCHARS_B(result##concatenate_buf, result##concatenate_lvl5, s5);

#define CONCATENATE_END_6(result, s1, s2, s3, s4, s5, s6) \
CONCATENATE_END_5(result, s1, s2, s3, s4, s5) \
CONCATENATE_END_ADDCHARS_B(result##concatenate_buf, result##concatenate_lvl6, s6);

#define CONCATENATE_END_7(result, s1, s2, s3, s4, s5, s6, s7) \
CONCATENATE_END_6(result, s1, s2, s3, s4, s5, s6) \
CONCATENATE_END_ADDCHARS_B(result##concatenate_buf, result##concatenate_lvl7, s7);

#define CONCATENATE_END_8(result, s1, s2, s3, s4, s5, s6, s7, s8) \
CONCATENATE_END_7(result, s1, s2, s3, s4, s5, s6, s7) \
CONCATENATE_END_ADDCHARS_B(result##concatenate_buf, result##concatenate_lvl8, s8);

#define CONCATENATE_END_9(result, s1, s2, s3, s4, s5, s6, s7, s8, s9) \
CONCATENATE_END_8(result, s1, s2, s3, s4, s5, s6, s7, s8) \
CONCATENATE_END_ADDCHARS_B(result##concatenate_buf, result##concatenate_lvl9, s9);

#define CONCATENATE(result, ...) \
SELECT( CONCATENATE_START, CV_VA_NUM_ARGS(__VA_ARGS__) )(result, __VA_ARGS__) \
char result[result##concatenate_lvls + 1]; \
result[result##concatenate_lvls] = 0; \
SELECT( CONCATENATE_END, CV_VA_NUM_ARGS(__VA_ARGS__) )(result, __VA_ARGS__) \
*result##concatenate_buf = 0;

// methods for CONCATENATE
// deprecated

namespace ns_concat {
  inline int numChars(std::string_view s) { return s.length(); }
  inline int numChars(const std::string &s) { return s.length(); }
  inline int numChars(const char *s) { return s?strlen(s):0; }
  inline int numChars(int i) {
    if (i == 0) return 1;
    if (i > 0 ) return stringhelpers_internal::numCharsUg0(i);
    return stringhelpers_internal::numCharsUg0(-i) + 1;
  }
  inline void addChars(char *b, int l, int i) { stringhelpers_internal::addCharsIbe(b+l, i); }
  inline void addChars(char *b, int l, const std::string_view &s) { memcpy(b, s.data(), l); }
  inline void addChars(char *b, int l, const std::string &s) { memcpy(b, s.data(), l); }
  inline void addChars(char *b, int l, const char *s) { if(s) memcpy(b, s, l); }
}

inline cSv textAttributeValue(const char *text, const char *attributeName) {
// if attributeName is empty or not found in text: return ""
// else, return text after attributeName to end of line
  if (!text || !attributeName || !*attributeName) return cSv();
  const char *found = strstr(text, attributeName);
  if (!found) return cSv();
  const char *avs = found + strlen(attributeName);
  const char *ave = strchr(avs, '\n');
  if (!ave) return cSv(avs);
  return cSv(avs, ave-avs);
}

// =========================================================
// methods for years =======================================
// =========================================================

class cYears {
  public:
    cYears() { m_years[0] = 0; }
    cYears(const cYears&) = delete;
    cYears &operator= (const cYears &) = delete;
// note: iterate first over exact matches, then near matches
    class iterator {
        const char *m_years;
      public:
//      using iterator_category = std::forward_iterator_tag;
        explicit iterator(const char *years): m_years(years) { }
        iterator& operator++() {
          if (*m_years) m_years++;
          return *this;
        }
        bool operator!=(iterator other) const { return m_years != other.m_years; }
        int operator*() const {
          return (*m_years) + 1900;
        }
      };
    iterator begin() const { return iterator(m_years); }
    const iterator end() const { return iterator(m_years + m_years_p); }
    void addYear(const char *year) { addYear(yearToInt(year)); }
    void addYear(int year) {
      if (m_explicitFound) return;
      if (year <= 1920 || year >= 2100) return;
      addYearInt(year-1900);
    }
    void addYears(const char *str) {
      if (!str || m_explicitFound) return;
      cSv year_sv = textAttributeValue(str, "Jahr: ");
      if (year_sv.length() == 4) {
        int y = yearToInt(year_sv.data() );
        if (!(y <= 1920 || y >= 2100)) {
          m_explicitFound = true;
          m_years_p = 0;
          m_years[m_years_p++] = y-1900;
          m_years[m_years_p] = 0;
          return;
        }
      }
      const char *last;
      for (const char *first = firstDigit(str); *first; first = firstDigit(last) ) {
        last = firstNonDigit(first);
        if (last - first  == 4) addYear(first);
      }
    }
    int find2(int year) const {
// 0 not found
// 1 near  match found
// 2 exact match found
      if (year <= 1920 || year >= 2100) return 0;
      const char *f = strchr(m_years, year-1900);
      if (!f) return 0;
      if (m_years + m_years_e > f) return 2;  // [m_years[0]; m_years[m_years_e]): exact matches
      return 1;
    }
    int size() const { return strlen(m_years); }
    static int yearToInt(const char *year) {
// if the first 4 characters of year are digits, return this number as int
// otherwise (or if year == NULL) return 0;
      if (!year) return 0;
      if (*year < '0' || *year > '9') return 0;
      int result = (*year - '0') * 1000;
      year++;
      if (*year < '0' || *year > '9') return 0;
      result += (*year - '0') * 100;
      year++;
      if (*year < '0' || *year > '9') return 0;
      result += (*year - '0') * 10;
      year++;
      if (*year < '0' || *year > '9') return 0;
      result += (*year - '0');
      return result;
    }
template<class T>
    static int yearToInt(const T &str) {
      if (str.length() < 4 || (str.length() > 4 && isdigit(str[4])) ) return 0;
      return yearToInt(str.data() );
    }
    void finalize() {
// this adds year +-1, at the end
// new: this adds year +1, at the end: EPG: Production year, ext.db: premiere
// back to +-1: Made a check, can also be -1; there seem to be several reasons for deviations ...
      if (m_years_e != -1) return;  // add this only once ...
      m_years_e = m_years_p;
      for (int i=0; i < m_years_e; i++) {
        addYearInt(m_years[i] + 1);
        addYearInt(m_years[i] - 1);
      }
    }

  private:
    void addYearInt(int year) {
// year must be in internal format !!! (real year - 1900)
      if (m_years_p > 18) return;
      if (strchr(m_years, year) != NULL) return;
      m_years[m_years_p++] = year;
      m_years[m_years_p] = 0;
    }
    static const char *firstDigit(const char *found) {
      for (; ; found++) if (isdigit(*found) || ! *found) return found;
    }
    static const char *firstNonDigit(const char *found) {
      for (; ; found++) if (!isdigit(*found) || ! *found) return found;
    }

    char m_years[20];
    int m_years_p = 0;
    int m_years_e = -1;
    bool m_explicitFound = false;
};

inline bool splitString(cSv str, char delimiter, size_t minLengh, cSv &first, cSv &second) {
  using namespace std::literals::string_view_literals;
  if (delimiter == '-') return splitString(str, " - "sv, minLengh, first, second);
  if (delimiter == ':') return splitString(str, ": "sv, minLengh, first, second);
  std::string delim(1, delimiter);
  return splitString(str, delim, minLengh, first, second);
}

inline int StringRemoveTrailingWhitespace(const char *str, int len) {
// return "new" len of string, without whitespaces at the end
  if (!str) return 0;
  for (; len; len--) if (!my_isspace(str[len - 1])) return len;
  return 0;
}

inline utf8_iterator utf8LastLetter(cSv s) {
// return position directly after the last letter
  utf8_iterator it = s.utf8_end();
  while (it != s.utf8_begin() ) {
    --it;
    wint_t cp = it.codepoint();
    if (isdigit(cp)) continue;
    if (std::iswalnum(cp)) { ++it; return it; }
  }
  return it;
}

inline int lenWithoutLastPartWithP(cSv sv) {
// search part with (...)
// return -1 if nothing was found
// otherwise length of sv without ()
  int len = StringRemoveTrailingWhitespace(sv.data(), sv.length() );
  if (len < 3) return -1;
  if (sv[len -1] != ')') return -1;
  for (int i = len -2; i; i--) {
    if (!isdigit(sv[i]) && sv[i] != '/') {
      if (sv[i] != '(') return -1;
      int len2 = lenWithoutLastPartWithP(sv.substr(0, i));
      if (len2 == -1 ) return StringRemoveTrailingWhitespace(sv.data(), i);
      return len2;
    }
  }
  return -1;
}

inline int lenWithoutPartToIgnoreInSearch(cSv sv) {
// we keep the last letter and all digits directly following this letter
  utf8_iterator it = utf8LastLetter(sv);
  size_t found = sv.find(": ", it.pos());
  if (found != std::string_view::npos) return found;
  found = sv.find(" #", it.pos());
  if (found != std::string_view::npos) return found;
  return lenWithoutLastPartWithP(sv);
}

inline bool StringRemoveLastPartWithP(std::string &str) {
// remove part with (...)
  int len = lenWithoutPartToIgnoreInSearch(str);
  if (len < 0) return false;
  str.erase(len);
  return true;
}
inline cSv removeLastPartWithP(cSv str) {
  int l = lenWithoutPartToIgnoreInSearch(str);
  if (l < 0) return str;
  return cSv(str.data(), l);
}

inline int NumberInLastPartWithPS(cSv str) {
// return number in last part with (./.), 0 if not found / invalid
  if (str.length() < 3 ) return 0;
  if (str[str.length() - 1] != ')') return 0;
  std::size_t found = str.find_last_of("(");
  if (found == std::string::npos) return 0;
  for (std::size_t i = found + 1; i < str.length() - 1; i ++) {
    if (!isdigit(str[i]) && str[i] != '/') return 0; // we ignore (asw), and return only number with digits only
  }
  return parse_unsigned_internal<int>(str.substr(found + 1));
}
inline int NumberInLastPartWithP(cSv str) {
// return number in last part with (...), 0 if not found / invalid
  if (str.length() < 3 ) return 0;
  if (str[str.length() - 1] == ')') {
    std::size_t found = str.find_last_of("(");
    if (found == std::string::npos) return 0;
    for (std::size_t i = found + 1; i < str.length() - 1; i ++) {
      if (!isdigit(str[i])) return 0; // we ignore (asw), and return only number with digits only
    }
    return parse_unsigned_internal<int>(str.substr(found + 1));
  } else {
    size_t pos = str.length() - 1;
    for (; pos > 0 && isdigit(str[pos]); --pos);
    if (str[pos] == '#') return parse_unsigned_internal<int>(str.substr(pos + 1));
    return 0;
  }
}

inline int Number2InLastPartWithP(cSv str) {
// return second number n in last part with (../n), 0 if not found / invalid
  if (str.length() < 3 ) return 0;
  if (str[str.length() - 1] != ')') return 0;
  std::size_t found = str.find_last_of("(");
  if (found == std::string::npos) return 0;
  found = str.find("/", found+1);
  if (found == std::string::npos) return 0;
  for (std::size_t i = found + 1; i < str.length() - 1; i ++) {
    if (!isdigit(str[i])) return 0; // we ignore (asw), and return only number with digits only
  }
  return parse_unsigned_internal<int>(str.substr(found + 1));
}

inline int seasonS(cSv description_part, const char *S) {
// return season, if found at the beginning of description_part
// otherwise, return -1
//  std::cout << "seasonS " << description_part << "\n";
  size_t s_len = strlen(S);
  if (description_part.length() <= s_len ||
     !isdigit(description_part[s_len])   ||
     description_part.compare(0, s_len, S)  != 0) return -1;
  return parse_unsigned<int>(description_part.substr(s_len));
}
inline bool episodeSEp(int &season, int &episode, cSv description, const char *S, const char *Ep) {
// search pattern S<digit> Ep<digits>
// return true if episode was found.
// set season = -1 if season was not found
// set episode = 0 if episode was not found
// find Ep[digit]
  season = -1;
  episode = 0;
  size_t Ep_len = strlen(Ep);
  size_t ep_pos = 0;
  do {
    ep_pos = description.find(Ep, ep_pos);
    if (ep_pos == std::string_view::npos || ep_pos + Ep_len >= description.length() ) return false;  // no Ep[digit]
    ep_pos += Ep_len;
//  std::cout << "ep_pos = " << ep_pos << "\n";
  } while (!isdigit(description[ep_pos]));
// Ep[digit] found
//  std::cout << "ep found at " << description.substr(ep_pos) << "\n";
  episode = parse_unsigned<int>(description.substr(ep_pos));
  if (ep_pos - Ep_len >= 3) season = seasonS(description.substr(ep_pos - Ep_len - 3), S);
  if (season < 0 && ep_pos - Ep_len >= 4) season = seasonS(description.substr(ep_pos - Ep_len - 4), S);
  return true;
}

// =========================================================
// =========== search in char*
// =========================================================

inline const char* removePrefix(const char *s, const char *prefix) {
// if s starts with prefix, return s + strlen(prefix)  (string with prefix removed)
// otherwise, return NULL
  if (!s || !prefix) return NULL;
  size_t len = strlen(prefix);
  if (strncmp(s, prefix, len) != 0) return NULL;
  return s+len;
}

inline const char *strnstr(const char *haystack, const char *needle, size_t len) {
// if len >  0: use only len characters of needle
// if len == 0: use all (strlen(needle)) characters of needle

  if (len == 0) return strstr(haystack, needle);
  for (;(haystack = strchr(haystack, needle[0])); haystack++)
    if (!strncmp(haystack, needle, len)) return haystack;
  return 0;
}

inline const char *strstr_word (const char *haystack, const char *needle, size_t len = 0) {
// as strstr, but needle must be a word (surrounded by non-alphanumerical characters)
// if len >  0: use only len characters of needle
// if len == 0: use strlen(needle) characters of needle
  if (!haystack || !needle || !(*needle) ) return NULL;
  size_t len2 = (len == 0) ? strlen(needle) : len;
  if (len2 == 0) return NULL;
  for (const char *f = strnstr(haystack, needle, len); f && *(f+1); f = strnstr (f + 1, needle, len) ) {
    if (f != haystack   && isalpha(*(f-1) )) continue;
    if (f[len2] != 0 && isalpha(f[len2]) ) continue;
    return f;
  }
  return NULL;
}
// =========================================================
// special container:
//   delimiter is '|'
//   if delimiter is missing at start of string: take complete string
// =========================================================

template<class T>
void push_back_new(std::vector<T> &vec, cSv str) {
// add str to vec, but only if str is not yet in vec and if str is not empty
  if (str.empty() ) return;
  if (find (vec.begin(), vec.end(), str) != vec.end() ) return;
  vec.emplace_back(str);
}

template<class T>
void stringToVector(std::vector<T> &vec, const char *str) {
// delimiter is '|', and must be available at start of str.
// if str does not start with '|', don't split, just add str to vec
// otherwise, split str at '|', and add each part to vec
  if (!str || !*str) return;
  if (str[0] != '|') { vec.push_back(str); return; }
  const char *lDelimPos = str;
  for (const char *rDelimPos = strchr(lDelimPos + 1, '|'); rDelimPos != NULL; rDelimPos = strchr(lDelimPos + 1, '|') ) {
    push_back_new<T>(vec, cSv(lDelimPos + 1, rDelimPos - lDelimPos - 1));
    lDelimPos = rDelimPos;
  }
}

inline std::string vectorToString(const std::vector<std::string> &vec) {
  if (vec.size() == 0) return std::string();
  if (vec.size() == 1) return vec[0];
  std::string result("|");
  for (const std::string &str: vec) { result.append(str); result.append("|"); }
  return result;
}

// =========================================================
// special container:
//   set, can contain string or integer or channel
//        or any other object wher methods to convert from/to string are available
// =========================================================

inline std::string objToString(const int &i) { return std::string(cToSvInt(i)); }
inline std::string objToString(const std::string &i) { return i; }
inline std::string objToString(const tChannelID &i) { return std::string(cToSvChannel(i)); }

template<class T, class C=std::set<T>>
std::string getStringFromSet(const C &in, char delim = ';') {
  if (in.size() == 0) return std::string();
  std::string result;
  for (const T &i: in) {
    result.append(objToString(i));
    result.append(1, delim);
  }
  return result;
}

template<class T> T stringToObj(const char *s, size_t len) {
  esyslog("tvscraper: ERROR: template<class T> T stringToObj called");
  return 5; }
template<> inline int stringToObj<int>(const char *s, size_t len) { return parse_unsigned<int>(cSv(s, len)); }
template<> inline std::string stringToObj<std::string>(const char *s, size_t len) { return std::string(s, len); }
template<> inline cSv stringToObj<cSv>(const char *s, size_t len) { return cSv(s, len); }
template<> tChannelID stringToObj<tChannelID>(const char *s, size_t len) {
  char buf[len+1];
  buf[len] = 0;
  memcpy(buf, s, len);
  return tChannelID::FromString(buf);
}

template<class T> void insertObject(std::vector<T> &cont, const T &obj) { cont.push_back(obj); }
template<class T> void insertObject(std::set<T> &cont, const T &obj) { cont.insert(obj); }

template<class T, class C=std::set<T>>
C getSetFromString(const char *str, char delim = ';') {
// split str at delim, and add each part to result
  C result;
  if (!str || !*str) return result;
  const char *lStartPos = str;
  if (delim == '|' && lStartPos[0] == delim) lStartPos++;
  for (const char *rDelimPos = strchr(lStartPos, delim); rDelimPos != NULL; rDelimPos = strchr(lStartPos, delim) ) {
    insertObject<T>(result, stringToObj<T>(lStartPos, rDelimPos - lStartPos));
    lStartPos = rDelimPos + 1;
  }
  const char *rDelimPos = strchr(lStartPos, 0);
  if (rDelimPos != lStartPos) insertObject<T>(result, stringToObj<T>(lStartPos, rDelimPos - lStartPos));
  return result;
}

class cConcatenate
// deprecated. Use function concatenate or stringAppend
{
  public:
    cConcatenate(size_t buf_size = 0) { m_data.reserve(buf_size>0?buf_size:200); }
    cConcatenate(const cConcatenate&) = delete;
    cConcatenate &operator= (const cConcatenate &) = delete;
  template<typename T>
    cConcatenate & operator<<(const T &i) { stringAppend(m_data, i); return *this; }
    std::string moveStr() { return std::move(m_data); }
    operator cSv() const { return cSv(m_data.data(), m_data.length()); }
    const char *getCharS() { return m_data.c_str(); }
  private:
    std::string m_data;
};

#endif // __TVSCRAPERHELPERS_H
