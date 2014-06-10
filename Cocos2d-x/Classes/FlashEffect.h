#ifndef __FLASH_EFFECT_H__
#define __FLASH_EFFECT_H__

#include "cocos2d.h"
#include "tinyxml2/tinyxml2.h"
#include "DisplayParameter.h"
#include "FlashEffectDefine.h"

USING_NS_CC;

//特效延时创建动作
class EffectCreateAction : public ActionInterval
{
protected:
	bool init(float delay_start, float duration);
	virtual EffectCreateAction* clone() const override;
	virtual EffectCreateAction* reverse() const override;
	virtual void update(float time) override;
	virtual void startWithTarget(Node *target) override;
	virtual bool doCreate() = 0;
	virtual bool doDestory() = 0;

protected:
	EffectCreateAction():
		m_bCreated(false),
		m_fDelayStart(0),
		m_fDuration(0)
	{};
	virtual ~EffectCreateAction()
	{};

private:
	float								m_fDelayStart;				// 起始延时
	float								m_fDuration;				// 持续时长
	bool								m_bCreated;					// 是否已创建
};

//声音特效延时创建
class SoundCreateAction : public EffectCreateAction
{
public:
	static SoundCreateAction * create(const char * sound, float duration,
		float delay_start, bool loop, float pitch, float pan, float gain);

	//override
	virtual bool doCreate() override;
	virtual bool doDestory() override;

protected:
	SoundCreateAction():
		m_szSound(nullptr),
		m_fPitch(false),
		m_fPan(false),
		m_fGain(false),
		m_bLoop(false),
		m_nSoundID(0)
	{};
	virtual ~SoundCreateAction()
	{
		doDestory();
		m_szSound = nullptr;
		m_nSoundID = 0;
	};

private:
	const char *						m_szSound;					//声音文件名
	float								m_fPitch;					//
	float								m_fPan;
	float								m_fGain;
	bool								m_bLoop;					//是否循环播放
	unsigned int						m_nSoundID;					// 对应的动画对象
};

class ImageCreateAction : public EffectCreateAction
{
public:
	static ImageCreateAction * create(const char * image, DisplayParameter param,
		float duration, float delay_start);

	//override
	virtual bool doCreate() override;
	virtual bool doDestory() override;

protected:
	ImageCreateAction() :
		m_pSprite(nullptr),
		m_szImageName(nullptr)
	{};
	virtual ~ImageCreateAction()
	{
		doDestory();
		m_szImageName = nullptr;
	};

private:
	const char *						m_szImageName;				// 图片名
	DisplayParameter					m_DisplayParameter;			// 显示参数
	Sprite *							m_pSprite;					// 图片显示对象
};

class NodeCreateAction : public EffectCreateAction
{
public:
	//创建
	static NodeCreateAction * create(const char * effect_id, DisplayParameter param,
		ActionInterval * action, float duration, float delay_start);

	//override
	virtual bool doCreate() override;
	virtual bool doDestory() override;

protected:
	NodeCreateAction():
		m_szEffectID(nullptr),
		m_pAction(nullptr)
	{};
	~NodeCreateAction()
	{
		doDestory();

		m_szEffectID = nullptr;
	};

private:
	const char *						m_szEffectID;				//特效ID
	DisplayParameter					m_DisplayParameter;			//初始显示参数
	Action *							m_pAction;					//运动
	Node *								m_pNode;					//被创建对象
};

class FlashEffect : public Node
{
public:
	static FlashEffect * create(const char * effect_id);
	// 子动画结束回调
	void onSubEffectFinish();

protected:
	bool init(const char * effect_id);
	// 从XML获取Action
	ActionInterval * parseXmlToAction(tinyxml2::XMLElement * action);
	// 创建抖动特效动作
	ActionInterval * createShakeAction(float duration, Point range=Point(2,2), float rate=10);

private:
	FlashEffect() :
		m_nSubEffectRuningCount(0)
	{};

private:
	int									m_nSubEffectRuningCount;	//在运行的子特效个数
};

class FlashEffectManager
{
public:
	static FlashEffectManager * getInstance()
	{
		if (nullptr == s_Instance)
		{
			s_Instance = new FlashEffectManager;
		}
		return s_Instance;
	};
	//从文件加载特效XML文档
	bool loadConfig(const char * xml_file);
	//通过ID创建特效
	FlashEffect * createEffect(const char * effect_id);

protected:
	//通过特效ID获取特效XML节点
	tinyxml2::XMLElement * getEffectXmlNode(const char * effect_id);

private:
	FlashEffectManager():
		m_pXmlDocument(nullptr)
	{};
	~FlashEffectManager()
	{
		if (m_pXmlDocument != nullptr)
		{
			delete m_pXmlDocument;
		}
	};

private:
	static FlashEffectManager *				s_Instance;				//单例实例
	tinyxml2::XMLDocument *					m_pXmlDocument;			//特效XML文档

	friend class FlashEffect;
};

#endif // __FLASH_EFFECT_H__
