#ifndef _BLOG_RANK_H
#define _BLOG_RANK_H
#include "cocos2d.h"
#include "ui/CocosGUI.h"
USING_NS_CC;
class BlogRank : public Layer , public ui::EditBoxDelegate{
public:
	static BlogRank* create();
	static Scene* createScene();
	virtual bool init() override;
	virtual void editBoxReturn(ui::EditBox* editBox) override;
};

#endif