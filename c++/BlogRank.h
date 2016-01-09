#ifndef _BLOG_RANK_H
#define _BLOG_RANK_H
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <regex>
#include <functional>
USING_NS_CC;


struct Article{
	std::string title;
	std::string url;
	int pageViews;
	Article(const std::string& t, const std::string& u, int pv) :title(t), url(u), pageViews(pv) {}
};

class BlogRank : public Layer , public ui::EditBoxDelegate{
public:
	static BlogRank* create();
	static Scene* createScene();
	virtual bool init() override;
	virtual void editBoxReturn(ui::EditBox* editBox) override;

	void regexCapture(std::string pattern, std::string src, std::function<void(std::smatch&)> func);
	void regexCaptureEx(std::string pattern, std::string pattern2, std::string src, std::function<void(std::smatch&, std::smatch&)> func);
	void allArticleViews(std::vector<std::string>& vs);
	void showResult(std::vector<Article>& va);
	std::string concatURL(std::string mid, int cnt);
	int flag;
	std::vector<std::string> vs;
	std::vector<Article> va;
	int size;
};

#endif