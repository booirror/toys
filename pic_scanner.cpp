#pragma warning(disable:4996)
#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <stdlib.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#define DISABLE_LOGGER
#include "avhttp.hpp"
namespace fs = boost::filesystem;
#define _SDEBUG

static int filenum = 0;

bool readUrl(const char* url, std::string& outstr)
{
	boost::asio::io_service baio;
	avhttp::http_stream avhttp(baio);
	boost::system::error_code error;

	avhttp.open(url, error);
	if (error) {
		std::cout << "ReadURL Error:" << error.message() << ",url=" << url << std::endl;
		return false;
	}
	std::stringstream buffer;
	buffer << &avhttp;
	outstr = buffer.str();
	return true;
}

void regexCapture(std::string pattern, std::string src, std::function<void(std::smatch&)> func)
{
	std::regex regpattern(pattern);
	std::smatch matchResult;
	while (std::regex_search(src, matchResult, regpattern)) {
		func(matchResult);
		src = matchResult.suffix();
	}
}

bool parseWebPage(std::string src, std::vector<std::string>& outvs)
{
	regexCapture(R"(href=\"(.*?)\")", src, [&outvs](std::smatch &mResult){outvs.push_back(mResult[1]); });
	return true;
}

void createDir(const char* fpath)
{
	fs::path filepath(fpath);
	if (!fs::exists(filepath)) {
		fs::create_directory(filepath);
	}
}

void replaceAll(std::string& filename, char s, std::string n)
{
	auto pos = filename.find(s);
	while (pos != std::string::npos) {
		filename.replace(pos, 1, n);
		pos = filename.find(s, pos + 1);
	}
}

void downloadFile(const char* url, const char* fname, const char* pathname)
{
	try {
		std::string path("others"), filename(url);
		regexCapture(R"(http://(.*)/(.*))", url, [&path, &filename](std::smatch& mResult){
			path = mResult[1];
			filename = mResult[2];
		});
		if (pathname) {
			path = pathname;
		}

		replaceAll(path, '/', "_");
		replaceAll(path, ':', "_");
		createDir(path.c_str());
		auto fullpath = path + "/" + filename;
		fs::path fpath(fullpath);
		if (fs::exists(fpath)) {
			goto NUMCNT;
		}
		boost::asio::io_service io;
		avhttp::http_stream http(io);
		http.open(url);
		std::ofstream outfile;
		outfile.open(fullpath, std::ios::binary | std::ios::out);
		if (outfile) {
			outfile << &http;
			outfile.close();
		}
	}
	catch (std::exception& e) {
		std::cerr << "Error:" << e.what() << std::endl;
	}
NUMCNT:
	filenum++;
	std::cout << "The num of files that has donwloaded: " << filenum << std::endl;
}

void parseCurrPagePic(std::string src, std::map<std::string, std::string>& picvec)
{
	regexCapture(R"(href=\"(.*?)\" title=\"(.+?)\")", src, [&picvec](std::smatch& mResult) {
		picvec[mResult[1]] = mResult[2];
	});
}

void parseCurrPicUrl(std::string src, std::vector<std::string>& picvec)
{
	regexCapture(R"(<img.+?src=\"(/.*?\.jpg)\")", src, [&picvec](std::smatch& mResult) {
		picvec.push_back(mResult[1]);
	});
}

void parseOtherPage(std::string src, std::vector<std::string>& picvec)
{
	regexCapture(R"(<a href=\"(.+?\.html)\">\d</a>)", src, [&picvec](std::smatch& mResult){
		picvec.push_back(mResult[1]);
	});
}

void parseOtherPic(std::string src, std::vector<std::string>& picvec)
{
	regexCapture(R"(<a href='(\w+?\.html)'>\d</a>)", src, [&picvec](std::smatch& mResult){
		picvec.push_back(mResult[1]);
	});
}

void spiderFiles(const char* url, bool downtoup)
{
	std::string buff;
	std::vector<std::string> urls;
	std::cout << "reading url..." << std::endl;
	readUrl(url, buff);
#ifdef _SDEBUG
	std::cout << buff << std::endl;
#endif
	std::cout << "spider running..." << std::endl;
	parseOtherPage(buff, urls);
	urls.insert(urls.begin(), url);
#ifdef _SDEBUG
	for (auto a : urls) {
		std::cout << a << std::endl;
	}
#endif
	auto mainUrl = url;
	std::string rootUrl;
	regexCapture(R"((http://.+?)/.*)", url, [&rootUrl](std::smatch& mResult){
		rootUrl = mResult[1];
	});
	if (rootUrl.empty()) {
		std::cerr << "get root url failure" << std::endl;
		std::abort();
	}
	std::string directory;
	regexCapture(R"(http://.+?/(.+))", url, [&directory](std::smatch& mResult){
		directory = mResult[1];
		replaceAll(directory, '/', "_");
	});
	if (directory.empty()) {
		std::cerr << "get directory failure" << std::endl;
		std::abort();
	}
	auto dirIndex = 1;
	for (auto pageurl : urls) {
		char temps[1024];
		sprintf(temps, "%s%d", directory.c_str(), dirIndex);
		dirIndex++;
		std::string storeDir(temps);
		if (pageurl.find("http://") == std::string::npos) {
			pageurl = std::string(mainUrl).append(pageurl);
		}
		std::map<std::string, std::string> picNames;
		std::string pageBuff;
		readUrl(pageurl.c_str(), pageBuff);
		parseCurrPagePic(pageBuff, picNames);

		for (auto it = picNames.begin(); it != picNames.end(); ++it) {
			auto picURL = it->first;
			auto fileName = it->second;
			std::string urlbuff;
			std::vector<std::string> totalURL;

			readUrl(picURL.c_str(), urlbuff);
			parseOtherPic(urlbuff, totalURL);
			std::cout << "total URL.size=" << totalURL.size() << std::endl;
			totalURL.insert(totalURL.begin(), picURL);
			for (auto u : totalURL) {
				if (u.find("http://") == std::string::npos) {
					u = std::string(mainUrl).append(u);
				}
				std::vector<std::string> picfileurl;
				std::string picurlbuff;
				readUrl(u.c_str(), picurlbuff);
				parseCurrPicUrl(picurlbuff, picfileurl);
				for (auto furl : picfileurl) {
					if (furl.find("http://") == std::string::npos) {
						furl = std::string(rootUrl).append(furl);
					}
					downloadFile(furl.c_str(), fileName.c_str(), storeDir.c_str());
				}
			}
		}
	}
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		std::cout << "usage:" << argv[0] << " <url> [-d]" << std::endl;
		std::exit(0);
	}
	bool downtoup = false;
	if (argc == 3 && strcmp("-d", argv[2]) == 0) {
		downtoup = true;
	}
	spiderFiles(argv[1], downtoup);
	return 0;
}