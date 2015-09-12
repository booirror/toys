#include "BlogRank.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"
#include "network/HttpClient.h"
#include <iostream>
#include <regex>

using namespace cocos2d::network;

BlogRank* BlogRank::create()
{
	BlogRank *br = new(std::nothrow) BlogRank();
	if (br->init()) {
		br->autorelease();
		return br;
	}
	CC_SAFE_DELETE(br);
	return nullptr;
}

Scene* BlogRank::createScene()
{
	Scene* scene = Scene::create();
	BlogRank* layer = BlogRank::create();
	scene->addChild(layer);
	return scene;
}

bool BlogRank::init()
{
	if (!Layer::init()) {
		return false;
	}
	Size size = Director::getInstance()->getVisibleSize();

	ui::EditBox *text = ui::EditBox::create(Size(size.width / 2, 80), ui::Scale9Sprite::create("r2.png"));
	text->setPosition(Vec2(size.width/2, size.height - 120));
	text->setReturnType(ui::EditBox::KeyboardReturnType::DONE);
	text->setPlaceHolder("input url");
	text->setDelegate(this);
	this->addChild(text);

	ui::ScrollView *view = ui::ScrollView::create();
	view->setDirection(ui::ScrollView::Direction::VERTICAL);
	view->setTouchEnabled(true);
	view->setContentSize(Size(size.width*0.8, size.height*0.7));
	
	return true;
}



void BlogRank::regexCapture(std::string pattern, std::string src, std::function<void(std::smatch&)> func)
{
	std::regex regpattern(pattern);
	std::smatch matchResult;
	while (std::regex_search(src, matchResult, regpattern)) {
		func(matchResult);
		src = matchResult.suffix();
	}
}


void BlogRank::editBoxReturn(ui::EditBox* editBox)
{
	log("editboxReturn call------------------------------!!!");
	HttpRequest *req = new (std::nothrow) HttpRequest();
	req->setUrl(editBox->getText());
	req->setRequestType(HttpRequest::Type::GET);
	req->setResponseCallback([this](HttpClient* client, HttpResponse* response)->void{
		if (!response) {
			log("response is null");
			return;
		}
		long code = response->getResponseCode();
		if (!response->isSucceed()){
			log("error:%s", response->getErrorBuffer());
			return;
		}
		std::vector<char> *buff = response->getResponseData();
		std::string buffstr(buff->begin(), buff->end());
		
		std::string url_mid;
		std::string url_num;
		std::string url3;
		std::string url4;
		this->regexCapture(R"--(</a> <a href="([^<]+?)/(\d+?)">([^/]+?)</a>(\s+?\r?\n\s+?)</div>)--", buffstr, [&url_mid, &url_num,&url3,&url4](std::smatch& mResult){
			url_mid = mResult[1];
			url_num = mResult[2];
			url3 = mResult[3];
			url4 = mResult[4];
		});
		int cnt = atoi(url_num.c_str());
		vs.clear();
		for (int i = 1; i <= cnt; i++) {
			vs.push_back(concatURL(url_mid, i));
		}
		this->flag = 0;
		va.clear();
		this->size = vs.size();
		allArticleViews(vs);
	});
	HttpClient::getInstance()->send(req);
	req->release();
}


std::string BlogRank::concatURL(std::string mid, int cnt)
{
	char buff[512];
	if (cnt == -1) {
		sprintf(buff, "http://blog.csdn.net%s", mid.c_str());
	}
	else {
		sprintf(buff, "http://blog.csdn.net%s/%d", mid.c_str(), cnt);

	}
	return std::string(buff);
}

void BlogRank::regexCaptureEx(std::string pattern, std::string pattern2, std::string src, std::function<void(std::smatch&, std::smatch&)> func)
{
	std::regex regpattern(pattern);
	std::regex regp2(pattern2);
	std::smatch matchResult1, mathResult2;
	while (std::regex_search(src, matchResult1, regpattern)) {
		std::string temp = matchResult1.suffix();
		
		if (std::regex_search(temp, mathResult2, regp2)) {
			func(matchResult1, mathResult2);
			src = mathResult2.suffix();
		}
		else {
			log("false");
		}
	}
}

void BlogRank::allArticleViews(std::vector<std::string>& vs)
{
	if (true) {
		HttpRequest *req = new (std::nothrow) HttpRequest();
		req->setUrl(vs[flag].c_str());
		req->setRequestType(HttpRequest::Type::GET);
		req->setResponseCallback([this](HttpClient* client, HttpResponse* response)->void {
			if (!response) {
				log("response is null");
				return;
			}
			long code = response->getResponseCode();
			if (!response->isSucceed()){
				log("error:%s", response->getErrorBuffer());
				return;
			}
			std::vector<char> *buff = response->getResponseData();
			std::string buffstr(buff->begin(), buff->end());
			this->flag++;

			this->regexCaptureEx(R"--(<span class="link_title"><a href="(.+?)">\r?\n?\s+(.+?)\s+\r?\n?\s+</a>)--", 
				R"--(<span class="link_view" title=.+?</a>\((\d+)\))--", buffstr, [this](std::smatch& mRes1, std::smatch& mRes2){
				va.push_back(Article(std::string(mRes1[2]), concatURL(std::string(mRes1[1]), -1), atoi(std::string(mRes2[1]).c_str())));
			});
			showResult(va);
		});
		HttpClient::getInstance()->send(req);
		req->release();
		
	}
}

void BlogRank::showResult(std::vector<Article>& va)
{
	if (flag != size) {
		allArticleViews(vs);
		return;
	}
	if (va.size() == 0) return;
	std::sort(va.begin(), va.end(), [](Article& a, Article& b)->bool{
		return a.pageViews > b.pageViews;
	});
	int i = 0;
	for (Article& a : va) {
		log("%03d PageViews(%05d) [%s](%s)", ++i, a.pageViews, a.title.c_str(), a.url.c_str());
	}
}