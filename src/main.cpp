#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <curl/curl.h>

#include <windows.h>

#include "json.hpp"

using namespace std;
using json = nlohmann::json;

#define OPEN_IN_BROWSER 0

string fnout;
string fnconf = "conf.json";

string url = "https://od-api.oxforddictionaries.com/api/v2/";  // entries/en-gb/";
string lang = "en/";
string query_meaning = "entries/";
string query_baseword = "lemmas/";

string app_id;
string app_key;

// #ifdef DEBUG
// bool DEBUG = true;
// #else
// bool DEBUG = false;
// #endif

#ifdef DEBUG
# define DLOG(fmt, ...) \
  do { \
    fprintf(stderr, fmt, __VA_ARGS__); \
  } while (0)
#else
# define DLOG(fmt, ...) \
  do { } while (0)
#endif

size_t writeCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

int getresponse(const string& word, const string& res, const string& query)
{
  CURLcode ret;
  CURL *hnd;
  struct curl_slist *slist1;

  string fullurl = url + query + lang + word;
  DLOG("%s\n", fullurl.c_str());

  slist1 = NULL;
  slist1 = curl_slist_append(slist1, "Content-Type: application/json");
  slist1 = curl_slist_append(slist1, string("app_id:" + app_id).c_str());
  slist1 = curl_slist_append(slist1, string("app_key:" + app_key).c_str());

  hnd = curl_easy_init();
  curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
  curl_easy_setopt(hnd, CURLOPT_URL, fullurl.c_str());
  curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
  curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.79.1");
  curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
  curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
  // curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
  curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &res);

  ret = curl_easy_perform(hnd);

  curl_easy_cleanup(hnd);
  hnd = NULL;
  curl_slist_free_all(slist1);
  slist1 = NULL;

  return ret;
}

bool all_are_letters(const string& str)
{
  for (auto& c: str) {
    if (!isalpha(c)) {
      return false;
    }
  }
  return true;
}

string parse_baseword(const string& res)
{
  json j = json::parse(res);
  return j["results"][0]["lexicalEntries"][0]["inflectionOf"][0]["id"];
}

void remQuotes(string& s)
{
  s.erase(remove(s.begin(), s.end(), '"'), s.end());
}

string parse_meaning(const string& res)
{
  json j = json::parse(res);
  json _j = j["results"][0]["lexicalEntries"][0]["entries"][0]["senses"];
  DLOG("%d\n", _j.size());

  string s;
  stringstream ss;

  for (json i : j["results"]) {
    for (json k : i["lexicalEntries"]) {
      s = k["lexicalCategory"]["text"];
      remQuotes(s);
      ss << string(2, ' ') << s << "\n";
      for (json m : k["entries"]) {
        int _i = 0;
        for (json n : m["senses"]) {
          s = n["definitions"][0];
          remQuotes(s);
          ss << string(4, ' ') << _i++ + 1 << ". " << s << "\n";
          for (json p : n["subsenses"]) {
            s = p["definitions"][0];
            remQuotes(s);
            ss << string(8, ' ') << "* " << s << "\n";
          }
          // ss << "\n";
        }
      }
    }
  }

  return ss.str();
}

void print_usage()
{
  cout << "Usage:\t worden.exe <word>\n";
}

int main(int argc, char **argv)
{
  if (!argc) {
    print_usage();
    return 0;
  }

  const string whitespace = " \t";
  json j;

  ifstream fconf(fnconf);
  json jconf;
  fconf >> jconf;

  string str;
  fconf >> str;
  DLOG("%s\n", str.c_str());

  app_id = jconf["app_id"];
  app_key = jconf["app_key"];
  fnout = jconf["outfile"];

  ofstream fout(fnout, ios::app);

  string word, response;
  string jsonstr = "{app_id:" + app_id + ",app_key:" + app_key + "}";
  DLOG("%s\n", jsonstr.c_str());

  string buffer = argv[1];

  // choose only the first word
  stringstream ss(buffer);
  ss >> word;

  // lowercase it
  for (auto& c : word) {
    c = tolower(c);
  }

  DLOG("%s --> %s\n", buffer.c_str(), word.c_str());

  // word = "mice";

  if (all_are_letters(word) && OPEN_IN_BROWSER) {
    string url = "https://www.google.com/search?q=define+";
    url.append(word);
    system(string("open " + url).c_str());
  }

  // ifstream fjson("res.json");
  // fjson >> j;
  // fjson.close();
  // response = j.dump();

  int ret = 0;
  if (all_are_letters(word)) {
    ret = getresponse(word, response, query_baseword);
    // cout << response << endl;

    if (!ret) {
      string baseword;
      baseword = parse_baseword(response);

      response.clear();
      ret = getresponse(baseword, response, query_meaning);
      // cout << response << endl;

      if (!ret) {
        string result;
        result = parse_meaning(response);
        cout << baseword << "\n" << result << "\n\n";
      }
    }
  }

  fout.close();
  fconf.close();

  return 0;
}
