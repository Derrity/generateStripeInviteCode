#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <string>
#include <fstream>
#include <curl/curl.h>
#include <boost/algorithm/string.hpp>

int successCount = 0;
int failCount = 0;

std::string generate_random_string(size_t length) {
    auto randchar = []() -> char {
        const char charset[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        static thread_local std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<size_t> distribution(0, max_index);
        return charset[distribution(generator)];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *s) {
    size_t newLength = size * nmemb;
    size_t oldLength = s->size();
    try {
        s->resize(oldLength + newLength);
    } catch (std::bad_alloc &e) {
        return 0;
    }
    std::copy((char *) contents, (char *) contents + newLength, s->begin() + oldLength);
    return newLength;
}

size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
    size_t realSize = size * nitems;
    std::string *headerStr = static_cast<std::string *>(userdata);
    headerStr->append(buffer, realSize);
    return realSize;
}


void check_code(std::ofstream &out) {
    while (true) {
        std::string code = generate_random_string(6);
        CURL *curl;
        CURLcode res;
        //std::string readBuffer;
        std::string headerBuffer;
        curl = curl_easy_init();
        if (curl) {
            std::string url = "https://dashboard.stripe.com/referral/" + code;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            //curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_PROXY, "http://127.0.0.1:10809");
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerBuffer);
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            std::string target = "https://dashboard.stripe.com/register?invite=" + code;
            if (boost::icontains(headerBuffer, target)) {
                successCount++;
                out << code << std::endl;
            } else {
                failCount++;
            }
            printf("\033c");
            printf("Result: Success✅: %d, Fail❌: %d\n", successCount, failCount);
        }
    }
}

int main() {
    int threadCount;
    std::cout << "Enter the number of threads: ";
    std::cin >> threadCount;
    std::vector<std::thread> threads;
    std::ofstream out("codes.txt", std::ios::app);
    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back(check_code, std::ref(out));
    }
    for (auto &thread: threads) {
        thread.join();
    }
    out.close();
    return 0;
}
