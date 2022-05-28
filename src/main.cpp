#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <curl/curl.h>

#include <windows.h>

using namespace std;




string filename = "e:/documents/words.txt";
string url = "https://od-api.oxforddictionaries.com/api/v2/entries/en-gb/";
string app_id;
string app_key;




size_t writeCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}



int fetchword(const string& word, string& res)
{
    CURLcode ret;
    CURL *hnd;
    struct curl_slist *slist1;

    slist1 = NULL;
    slist1 = curl_slist_append(slist1, "Content-Type: application/json");
    slist1 = curl_slist_append(slist1, app_id.c_str());
    slist1 = curl_slist_append(slist1, app_key.c_str());

    hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(hnd, CURLOPT_URL,url.append(word).c_str());
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.79.1");
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
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
        if (!isalpha(c)) return false;
    }
    return true;
}

int main()
{
    const string whitespace = " \t";

    ifstream conf("conf");
    conf >> app_id >> app_key;


    string word, response;
    string jsonstr = "{app_id:"+app_id+",app_key:"+app_key+"}";
    cout << jsonstr <<endl;

    if (OpenClipboard(0)) {
        // get text from system clipboard
        HANDLE hData = GetClipboardData(CF_TEXT);
        string buffer = (char *)GlobalLock(hData);


        // choose only the first word
        stringstream ss(buffer);
        ss >> word;

        // lowercase it
        for (auto& c : word) {
            c = tolower(c);
        }

        cout << buffer << endl;
        cout << word << endl << endl;

        if (all_are_letters(word)) {
            // word = "prestidigitation";
            string url = "https://www.google.com/search?q=define+";
            url.append(word);
            system(string("open " + url).c_str());
        }


        CloseClipboard();
    }



    return 0;
}
