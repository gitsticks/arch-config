#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <cstdio>
#include <algorithm>

static std::string trim(std::string s) {
    while (!s.empty() && (s.back()=='\n' || s.back()=='\r' || s.back()==' ' || s.back()=='\t')) s.pop_back();
    size_t i = 0;
    while (i < s.size() && (s[i]==' ' || s[i]=='\t')) i++;
    return s.substr(i);
}

static std::string q(const std::string& s) { return std::string("'") + s + "'"; }

static std::string runCapture(const std::string& cmd) {
    std::string out;
    FILE* p = popen(cmd.c_str(), "r");
    if (!p) return out;
    char buf[4096];
    while (fgets(buf, sizeof(buf), p)) out += buf;
    pclose(p);
    return out;
}

static long long parseLL(const std::string& s) {
    try {
        size_t idx = 0;
        long long v = std::stoll(trim(s), &idx, 10);
        if (idx == 0) return -1;
        return v;
    } catch (...) {
        return -1;
    }
}

int main() {
    std::string url;
    std::cout << "Playlist URL: ";
    std::getline(std::cin, url);
    url = trim(url);
    if (url.empty()) return 1;

    const char* home = std::getenv("HOME");
    if (!home) return 1;

    const std::string cacheDir = std::string(home) + "/.cache";
    const std::string archivePath = cacheDir + "/yt-dlp-download-archive.txt";
    const std::string outTemplate = "%(playlist_title)s/%(playlist_index)03d - %(title)s.%(ext)s";
    const int batch = 50;

    std::system((std::string("mkdir -p ") + q(cacheDir)).c_str());

    std::ostringstream countCmd;
    countCmd
        << "yt-dlp --flat-playlist --yes-playlist "
        << "--cookies-from-browser firefox "
        << "--print " << q("%(playlist_count)s") << " "
        << q(url) << " 2>/dev/null";

    long long total = parseLL(runCapture(countCmd.str()));
    if (total <= 0) {
        std::cerr << "Could not determine playlist size.\n";
        return 2;
    }

    for (long long start = 1; start <= total; start += batch) {
        long long end = std::min(start + (long long)batch - 1, total);

        std::cout << "Batch " << start << "-" << end << " / " << total << std::endl;

        std::ostringstream cmd;
        cmd << "yt-dlp --yes-playlist --cookies-from-browser firefox"
            << " --ignore-errors"                              // IMPORTANT
            << " --download-archive " << q(archivePath)
            << " --continue"
            << " --playlist-start " << start
            << " --playlist-end " << end
            << " --retries 10 --fragment-retries 10 --file-access-retries 3 --retry-sleep 5"
            << " --sleep-interval 2 --max-sleep-interval 20"
            << " -f " << q("bestaudio/best")
            << " -x --audio-format mp3 --audio-quality 0"
            << " -o " << q(outTemplate)
            << " --embed-metadata --embed-thumbnail "
            << q(url);

        int rc = std::system(cmd.str().c_str());

        if (rc != 0) {
            std::cerr << "yt-dlp returned non-zero (" << rc << "), continuing to next batch.\n";
            continue;  // tolerate transient failures

        }
    }

    return 0;
}
