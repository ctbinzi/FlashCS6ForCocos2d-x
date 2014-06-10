#include "HelloWorldScene.h"
#include "FlashEffect.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Point origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    
	closeItem->setPosition(Point(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Point::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

	// 加载图片资源
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("flash_effects.plist");

	// 加载Flash特效动画配置文档
	FlashEffectManager::getInstance()->loadConfig("flash_effects.xml");

	// 侦听触屏事件
	EventListenerTouchOneByOne* m_touchListener = EventListenerTouchOneByOne::create();
	m_touchListener->setSwallowTouches(true);
	m_touchListener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
	m_touchListener->onTouchMoved = CC_CALLBACK_2(HelloWorld::onTouchMoved, this);
	m_touchListener->onTouchEnded = CC_CALLBACK_2(HelloWorld::onTouchEnded, this);
	m_touchListener->onTouchCancelled = CC_CALLBACK_2(HelloWorld::onTouchCancelled, this);

	_eventDispatcher->addEventListenerWithFixedPriority(m_touchListener, 1);
	
    // add a label shows "Hello World"
    // create and initialize a label
    
    //auto label = LabelTTF::create("Hello World", "Arial", 24);
    
    // position the label on the center of the screen
    //label->setPosition(Point(origin.x + visibleSize.width/2,
    //                        origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    //this->addChild(label, 1);

    // add "HelloWorld" splash screen"
    //auto sprite = Sprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    //sprite->setPosition(Point(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer
    //this->addChild(sprite, 0);
    
    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

bool HelloWorld::onTouchBegan(Touch *touch, Event * event)
{
	return true;
}

void HelloWorld::onTouchEnded(Touch *touch, Event * event)
{
	static int nCount = 0;

	Node * pEffect = nullptr;
	switch (nCount%3)
	{
	case 0:
		pEffect = FlashEffectManager::getInstance()->createEffect(EFFECT1);
		break;
	case 1:
		pEffect = FlashEffectManager::getInstance()->createEffect(EFFECT2);
		break;
	case 2:
		pEffect = FlashEffectManager::getInstance()->createEffect(EFFECT3);
		break;
	default:
		break;
	}

	CC_ASSERT(pEffect);
	if (nullptr != pEffect)
	{
		pEffect->setPosition(touch->getLocation());
		this->addChild(pEffect);
	}

	nCount ++;
}

void HelloWorld::onTouchCancelled(Touch *touch, Event * event)
{

}

void HelloWorld::onTouchMoved(Touch *touch, Event * event)
{

}
