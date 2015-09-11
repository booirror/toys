#include "BlogRank.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"
#include "network/HttpClient.h"
#include <iostream>

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

	ui::EditBox *text = ui::EditBox::create(Size(size.width / 2, 80), ui::Scale9Sprite::create("yellow_edit.png"));
	text->setPosition(Vec2(size.width/2, size.height - 120));
	text->setReturnType(ui::EditBox::KeyboardReturnType::DONE);
	text->setPlaceHolder("url");
	text->setDelegate(this);
	this->addChild(text);

	ui::ScrollView *view = ui::ScrollView::create();
	view->setDirection(ui::ScrollView::Direction::VERTICAL);
	view->setTouchEnabled(true);
	view->setContentSize(Size(size.width*0.8, size.height*0.7));

	return true;
}

void BlogRank::editBoxReturn(ui::EditBox* editBox)
{
	log("editboxReturn call------------------------------!!!");
	HttpRequest *req = new (std::nothrow) HttpRequest();
	req->setUrl(editBox->getText());
	req->setRequestType(HttpRequest::Type::GET);
	req->setResponseCallback([](HttpClient* client, HttpResponse* response)->void{
		if (!response) {
			return;
		}
		long code = response->getResponseCode();
		if (!response->isSucceed()){
			log("error:%s", response->getErrorBuffer());
		}
		std::vector<char> *buff = response->getResponseData();
		std::string buffstr(buff->begin(), buff->end());
		
	});
	HttpClient::getInstance()->send(req);
	req->release();
}