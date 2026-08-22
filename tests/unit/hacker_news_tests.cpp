#include "techpulse/sources/hacker_news.hpp"

#include <iostream>
#include <map>

int main() {
    const std::map<std::string, techpulse::sources::HttpResponse> responses{
        {"https://hacker-news.firebaseio.com/v0/topstories.json", {200, "[1,2,3]"}},
        {"https://hacker-news.firebaseio.com/v0/beststories.json", {500, ""}},
        {"https://hacker-news.firebaseio.com/v0/showstories.json", {200, "[]"}},
        {"https://hacker-news.firebaseio.com/v0/jobstories.json", {200, "[4]"}},
        {"https://hacker-news.firebaseio.com/v0/item/1.json", {200, R"({"id":1,"title":"  C++\ttool  ","url":"HTTPS://EXAMPLE.COM/a?utm_source=hn","time":1787392800,"score":10,"descendants":2})"}},
        {"https://hacker-news.firebaseio.com/v0/item/2.json", {200, R"({"id":2,"deleted":true})"}},
        {"https://hacker-news.firebaseio.com/v0/item/3.json", {200, R"({"id":3,"title":""})"}},
        {"https://hacker-news.firebaseio.com/v0/item/4.json", {200, R"({"id":4,"title":"C++ role","time":1787392800})"}},
    };
    const auto get = [&](const std::string& url) { const auto found = responses.find(url); return found == responses.end() ? techpulse::sources::HttpResponse{404, {}} : found->second; };
    const auto result = techpulse::sources::HackerNewsSource{3}.fetch(get);
    if (result.items.size() != 2 || result.errors.size() != 1 || result.items[0].title != "C++ tool" ||
        result.items[0].url != "https://example.com/a" || result.items[1].source_type != "job") {
        std::cerr << "HN adapter fixture test failed\n";
        return 1;
    }
    return 0;
}
