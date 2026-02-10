#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>

static std::string trim(std::string s) {
    while (!s.empty() && (s.back()=='\n' || s.back()=='\r' || s.back()==' ' || s.back()=='\t')) s.pop_back();
    size_t i = 0;
    while (i < s.size() && (s[i]==' ' || s[i]=='\t')) i++;
    return s.substr(i);
}

static bool looksLikeCookieExpired(const std::string& line) {
    return line.find("cookies are no longer valid") != std::string::npos ||
           line.find("Sign in to confirm you’re not a bot") != std::string::npos ||
           line.find("Sign in to confirm you're not a bot") != std::string::npos;
}

static int run_ytdlp_batch_and_watch(const std::vector<std::string>& args, bool& cookieProblem) {
    cookieProblem = false;

    int pipefd[2];
    if (pipe(pipefd) != 0) return 1;

    pid_t pid = fork();
    if (pid < 0) return 1;

    if (pid == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        std::vector<char*> argv;
        argv.reserve(args.size() + 1);
        for (auto& s : args) argv.push_back(const_cast<char*>(s.c_str()));
        argv.push_back(nullptr);

        execvp(argv[0], argv.data());
        _exit(127);
    }
    
    close(pipefd[1]);

    std::string buf;
    buf.reserve(4096);
    char chunk[1024];

    bool sentInt = false;

    for (;;) {
        ssize_t n = read(pipefd[0], chunk, sizeof(chunk));
        if (n <= 0) break;
        buf.append(chunk, chunk + n);

        size_t pos;
        while ((pos = buf.find('\n')) != std::string::npos) {
            std::string line = buf.substr(0, pos + 1);
            buf.erase(0, pos + 1);

            std::cout << line << std::flush;

            if (!sentInt && looksLikeCookieExpired(line)) {
                cookieProblem = true;
                sentInt = true;
                kill(pid, SIGINT); //least hacky cli wrapper
            }
        }
    }

    close(pipefd[0]);

    int status = 0;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
    return 1;
}

int main() {
    std::string url;
    std::cout << "Playlist URL: ";
    std::getline(std::cin, url);
    url = trim(url);
    if (url.empty()) return 1;

    const char* home = getenv("HOME");
    if (!home) return 1;

    const std::string archivePath = std::string(home) + "/.cache/yt-dlp-download-archive.txt";
    const std::string outTemplate = "%(playlist_title)s/%(playlist_index)03d - %(title)s.%(ext)s";
    const int batch = 50;

    {
        std::string cmd = std::string("mkdir -p ") + std::string(home) + "/.cache";
        std::system(cmd.c_str());
    }

    for (;;) {
        std::vector<std::string> args = {
            "yt-dlp",
            "--yes-playlist",
            "--cookies-from-browser", "firefox",
            "--download-archive", archivePath,
            "--continue",
            "--max-downloads", std::to_string(batch),
            "--retries", "10",
            "--fragment-retries", "10",
            "--file-access-retries", "3",
            "--retry-sleep", "5",
            "--sleep-interval", "2",
            "--max-sleep-interval", "20",
            "-f", "bestaudio/best",
            "-x",
            "--audio-format", "mp3",
            "--audio-quality", "0",
            "-o", outTemplate,
            "--embed-metadata",
            "--embed-thumbnail",
            url
        };

        bool cookieProblem = false;
        int rc = run_ytdlp_batch_and_watch(args, cookieProblem);
	// genuinely kys youtube 
        if (cookieProblem) continue;
        if (rc != 0) return rc;
    }
}
