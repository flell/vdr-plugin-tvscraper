#include <iterator>
#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <math.h>
#include <set>
 
std::set<std::string> ignoreWords = {"der", "die", "das", "the", "I", "-"};
// see https://stackoverflow.com/questions/15416798/how-can-i-adapt-the-levenshtein-distance-algorithm-to-limit-matches-to-a-single#15421038
template<typename T, typename C>
size_t
seq_distance(const T& seq1, const T& seq2, const C& cost,
             const typename T::value_type& empty = typename T::value_type()) {
  const size_t size1 = seq1.size();
  const size_t size2 = seq2.size();
 
  std::vector<size_t> curr_col(size2 + 1);
  std::vector<size_t> prev_col(size2 + 1);
 
  // Prime the previous column for use in the following loop:
  prev_col[0] = 0;
  for (size_t idx2 = 0; idx2 < size2; ++idx2) {
    prev_col[idx2 + 1] = prev_col[idx2] + cost(empty, seq2[idx2]);
  }
 
  curr_col[0] = 0;
  for (size_t idx1 = 0; idx1 < size1; ++idx1) {
    curr_col[0] = curr_col[0] + cost(seq1[idx1], empty);
 
    for (size_t idx2 = 0; idx2 < size2; ++idx2) {
      curr_col[idx2 + 1] = std::min(std::min(
        curr_col[idx2] + cost(empty, seq2[idx2]),
        prev_col[idx2 + 1] + cost(seq1[idx1], empty)),
        prev_col[idx2] + cost(seq1[idx1], seq2[idx2]));
    }
 
    curr_col.swap(prev_col);
    curr_col[0] = prev_col[0];
  }
 
  return prev_col[size2];
}
 
size_t
letter_distance(char letter1, char letter2) {
  return letter1 != letter2 ? 1 : 0;
}
 
size_t
word_distance(const std::string& word1, const std::string& word2) {
  return seq_distance(word1, word2, &letter_distance);
}
 
std::vector<std::string> word_list(const std::string &sentence, int &len) {
// return list of words
//std::cout << "sentence = \"" << sentence << "\" ";
  len = 0;
  std::vector<std::string> words;
  std::istringstream iss(sentence);
  for(std::istream_iterator<std::string> it(iss), end; ; ) {
  if (ignoreWords.find(*it) == ignoreWords.end()  )
    {
      words.push_back(*it);
      len += it->length();
//    std::cout << "word = \"" << *it << "\" ";
    }
    if(++it == end) break;
  }
//std::cout << std::endl;
/*
  std::copy(std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>(),
            std::back_inserter(words));
*/
  return words;
}

size_t
sentence_distance_int(const std::string& sentence1, const std::string& sentence2, int &len1, int &len2) {
  return seq_distance(word_list(sentence1, len1), word_list(sentence2, len2), &word_distance);
}
std::string stripExtra(const std::string &in) {
  std::string out;
  out.reserve(in.length() );
  for (const char &c: in) {
    if ( (c > 0 && c < 46) || c == 46 || c == 47 || ( c > 57 && c < 65 ) || ( c > 90 && c < 97 ) || ( c > 122  && c < 128) ) {
      out.append(1, ' ');
    } else out.append(1, c);
  }
  return out;
}

// find longest common substring
// https://iq.opengenus.org/longest-common-substring/
int lcsubstr( const std::string &s1, const std::string &s2)
{
  int len1=s1.length(),len2=s2.length();
  int dp[2][len2+1];
  int curr=0, res=0;
//  int end=0;
  for(int i=0; i<=len1; i++) {
    for(int j=0; j<=len2; j++) {
      if(i==0 || j==0) dp[curr][j]=0;
      else if(s1[i-1] == s2[j-1]) {
        dp[curr][j] = dp[1-curr][j-1] + 1;
        if(res < dp[curr][j]) {
          res = dp[curr][j];
//          end = i-1;
        }
      } else {
          dp[curr][j] = 0;
      }
    }
    curr = 1-curr;
  }
//  std::cout << s1 << " " << s2 << " length= " << res << " substr= " << s1.substr(end-res+1,res) << std::endl;
  return res;
}

float normMatch(float x) {
// input:  number between 0 and infinity
// output: number between 0 and 1
// normMatch(1) = 0.5
// you can call normMatch(x/n), which will return 0.5 for x = n
  if (x <= 0) return 0;
  if (x <  1) return sqrt (x)/2;
  return 1 - 1/(x+1);
}
int normMatch(int i, int n) {
// return number between 0 and 1000
// for i == n, return 500
  if (n == 0) return 1000;
  return normMatch((float)i / (float)n) * 1000;
}

void resizeStringWordEnd(std::string &str, int len) {
  if ((int)str.length() <= len) return;
  const size_t found = str.find_first_of(" .,;:!?", len);
  if (found == std::string::npos) return;
  str.resize(found);
}

int dist_norm_fuzzy(std::string &str_longer, const std::string &str_shorter, int max_length) {
// return 0 - 1000
// 0:    identical
// 1000: no match
// truncate str_longer to max_length (+ characters to ensure complete word end)
  if ((int)str_longer.length() > max_length) resizeStringWordEnd(str_longer, max_length);
  int l1, l2;
  int dist = sentence_distance_int(str_longer, str_shorter, l1, l2);
//  std::cout << "l1 = " << l1 << " l2 = " << l2 << " dist = " << dist << std::endl;
  int max_dist = std::max(std::max(l1, l2), dist);
  if (max_dist == 0) return 1000; // no word found -> no match
  return 1000 * dist / max_dist;
}

int sentence_distance(const std::string& sentence1a, const std::string& sentence2a) {
// return 0-1000
// 0: Strings are equal
  std::string sentence1 = stripExtra(sentence1a);
  std::string sentence2 = stripExtra(sentence2a);
  if (sentence1.length() < sentence2.length() ) sentence1.swap(sentence2); // now sentence1 is longer than (or equal to ) sentence2
//  std::cout << "sentence1 = " << sentence1 << " sentence2 = " << sentence2 << std::endl;
  int s1l = sentence1.length();
  int s2l = sentence2.length();
  if (s2l == 0) return 1000;

  int slengt = lcsubstr(sentence1, sentence2);
  int upper = 10; // match of more than upper characters: good
  if (slengt == s2l) upper = std::min(std::max(s2l/3, 2), 9);  // if the shorter string is part of the longer string: reduce upper
  if (slengt == s1l) slengt *= 2; // the strings are identical, add some points for this
  int max_dist_lcsubstr = 1000;
  int dist_lcsubstr = 1000 - normMatch(slengt, upper);
  int dist_lcsubstr_norm = 1000 * dist_lcsubstr / max_dist_lcsubstr;

  int max1 = std::max(20, 2*s2l);
  int max2 = std::max(15, s2l + s2l/2);
  int max3 = std::max( 6, s2l + s2l/2);
  int dist_norm = dist_norm_fuzzy(sentence1, sentence2, max1);
  if (s1l > max2 && max1 != max2) dist_norm = std::min(dist_norm, dist_norm_fuzzy(sentence1, sentence2, max2));
  if (s1l > max3 && max2 != max3) dist_norm = std::min(dist_norm, dist_norm_fuzzy(sentence1, sentence2, max3));
//std::cout << "dist_lcsubstr_norm = " << dist_lcsubstr_norm << " dist_norm = " << dist_norm << std::endl;
  return dist_norm / 2 + dist_lcsubstr_norm / 2;
}
