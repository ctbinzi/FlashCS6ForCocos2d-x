#include "FlashEffect.h"
#include "SimpleAudioEngine.h"

//////////////////////////////////////////////////////////////////////////

bool EffectCreateAction::init( float delay_start, float duration )
{
	m_fDuration = duration;
	m_fDelayStart = delay_start;

	initWithDuration(delay_start + duration);

	return true;
}

EffectCreateAction* EffectCreateAction::clone() const 
{
	CCASSERT(false, "EffectCreateAction can't clone.");
	return nullptr;
}

EffectCreateAction* EffectCreateAction::reverse() const 
{
	CCASSERT(false, "NodeCreateAction can't reverse.");
	return nullptr;
}

void EffectCreateAction::update( float time )
{
	if(m_bCreated && _elapsed > _duration)
	{
		this->doDestory();

		m_bCreated = false;
	}
	else if(_elapsed >= m_fDelayStart && !m_bCreated)
	{
		this->doCreate();

		m_bCreated = true;
	}
}

void EffectCreateAction::startWithTarget( Node *target )
{
	CCASSERT(dynamic_cast<FlashEffect*>(target) != nullptr, "EffectCreateAction only supports FlashEffect as target.");
	ActionInterval::startWithTarget(target);
}

//////////////////////////////////////////////////////////////////////////

SoundCreateAction * SoundCreateAction::create( const char * sound, float duration, float delay_start, bool loop, float pitch, float pan, float gain )
{
	CCASSERT(sound, "sound is null.");
	if(nullptr == sound) return nullptr;
	if(duration < FLT_EPSILON) duration = FLT_MAX;

	SoundCreateAction * pAction = new SoundCreateAction;
	if (nullptr == pAction) return nullptr;
	pAction->autorelease();

	if (!pAction->init(delay_start, duration))
	{
		CC_SAFE_DELETE(pAction);
		return nullptr;
	}

	pAction->m_szSound = sound;
	pAction->m_fGain = gain;
	pAction->m_fPan = pan;
	pAction->m_fPitch = pitch;

	return pAction;
}

bool SoundCreateAction::doCreate()
{
	CocosDenshion::SimpleAudioEngine * pAudioEngine = CocosDenshion::SimpleAudioEngine::getInstance();
	if (nullptr == pAudioEngine)
	{
		return false;
	}

	m_nSoundID = pAudioEngine->playEffect(m_szSound, m_bLoop, m_fPitch, m_fPan, m_fGain);
	return true;
}

bool SoundCreateAction::doDestory()
{
	CocosDenshion::SimpleAudioEngine * pAudioEngine = CocosDenshion::SimpleAudioEngine::getInstance();
	if (nullptr == pAudioEngine)
	{
		return false;
	}

	pAudioEngine->stopEffect(m_nSoundID);

	m_nSoundID = 0;

	return true;
}

//////////////////////////////////////////////////////////////////////////

ImageCreateAction * ImageCreateAction::create( const char * image, DisplayParameter param, float duration, float delay_start )
{
	CCASSERT(image, "image is null.");
	if(nullptr == image) return nullptr;
	if(duration < FLT_EPSILON) duration = FLT_MAX;

	ImageCreateAction * pAction = new ImageCreateAction;
	if (!pAction) return nullptr;
	pAction->autorelease();

	if(!pAction->init(delay_start, duration))
	{
		CC_SAFE_DELETE(pAction);
	}

	pAction->m_szImageName = image;
	pAction->m_DisplayParameter = param;

	return pAction;
}

bool ImageCreateAction::doCreate()
{
	m_pSprite = Sprite::createWithSpriteFrameName(m_szImageName);
	if (nullptr == m_pSprite) return false;
	m_pSprite->retain();
	m_pSprite->setCascadeColorEnabled(true);
	m_pSprite->setCascadeOpacityEnabled(true);
	m_pSprite->setAnchorPoint(Point(0,1));

	m_DisplayParameter.apply(m_pSprite);

	if (_target)
	{
		_target->addChild(m_pSprite);
	}

	return true;
}

bool ImageCreateAction::doDestory()
{
	if (m_pSprite != nullptr)
	{
		m_pSprite->removeFromParent();
		m_pSprite->release();

		m_pSprite = nullptr;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////

bool NodeCreateAction::doCreate()
{
	m_pNode = FlashEffect::create(m_szEffectID);
	if(nullptr == m_pNode) return false;
	m_pNode->retain();
	m_pNode->setCascadeColorEnabled(true);
	m_pNode->setCascadeOpacityEnabled(true);

	m_DisplayParameter.apply(m_pNode);

	if (nullptr != _target)
	{
		_target->addChild(m_pNode);
	}

	if(nullptr != m_pAction)
	{
		m_pNode->runAction(m_pAction);
	}

	return true;
}

bool NodeCreateAction::doDestory()
{
	if (m_pAction)
	{
		m_pAction->release();
		m_pAction = nullptr;
	}

	if (nullptr != m_pNode)
	{
		if (nullptr != _target)
		{
			_target->removeChild(m_pNode);

			FlashEffect * pParentEffect = dynamic_cast<FlashEffect*>(_target);
			if (nullptr != pParentEffect)
			{
				pParentEffect->onSubEffectFinish();
			}
		}

		m_pNode->release();
		m_pNode = nullptr;
	}
	return true;
}

NodeCreateAction * NodeCreateAction::create( const char * effect_id, DisplayParameter param, ActionInterval * action, float duration, float delay_start )
{
	CCASSERT(effect_id, "effect_id is null.");
	if(effect_id == nullptr) return nullptr;

	NodeCreateAction * pAction = new NodeCreateAction;
	if(pAction == nullptr) return nullptr;
	pAction->autorelease();

	if (action != nullptr)
	{
		duration = action->getDuration();
	}
	if (duration < FLT_EPSILON)
	{
		duration = FLT_MAX;
	}
	if(!pAction->init(delay_start, duration))
	{
		CC_SAFE_DELETE(pAction);
	}

	pAction->m_szEffectID = effect_id;
	pAction->m_DisplayParameter = param;
	pAction->m_pAction = action;
	if(action)
	{
		pAction->m_pAction->retain();
	}

	return pAction;
}

//////////////////////////////////////////////////////////////////////////

FlashEffect * FlashEffect::create( const char * effect_id )
{
	FlashEffect * effect = new FlashEffect;
	if (effect && effect->init(effect_id))
	{
		effect->autorelease();
		return effect;
	}

	return nullptr;
}

bool FlashEffect::init( const char * effect_id )
{
	if (!Node::init()) return false;

	tinyxml2::XMLElement * pEffectElement = FlashEffectManager::getInstance()->getEffectXmlNode(effect_id);
	CCASSERT(pEffectElement, "getEffectXmlNode fail.");
	if (nullptr == pEffectElement) return false;

	tinyxml2::XMLElement * pSubEffect = pEffectElement->FirstChildElement();
	CCASSERT(pSubEffect, "pSubEffect is null.");
	if (nullptr == pSubEffect) return false;

	do 
	{
		float fStart = 0.0f;
		float fDuration = 0.0f;
		pSubEffect->QueryFloatAttribute("t", &fStart);
		pSubEffect->QueryFloatAttribute("d", &fDuration);

		if (strcmp(pSubEffect->Name(), "p") == 0)
		{
			const char * szImage = pSubEffect->Attribute("img");

			DisplayParameter param;
			param.readXml(pSubEffect);

			this->runAction(ImageCreateAction::create(szImage, param, fDuration, fStart));
		}
		else if (strcmp(pSubEffect->Name(), "n") == 0)
		{
			const char * szID = pSubEffect->Attribute("id");

			DisplayParameter param;
			param.readXml(pSubEffect);

			ActionInterval * pAction = this->parseXmlToAction(pSubEffect->FirstChildElement());

			this->runAction(NodeCreateAction::create(szID, param, pAction, fDuration, fStart));

			m_nSubEffectRuningCount ++;
		}
		else if (strcmp(pSubEffect->Name(), "s") == 0)
		{
			const char * szSound = pSubEffect->Attribute("file");
			bool bLoop = false;
			pSubEffect->QueryBoolAttribute("loop", &bLoop);
			float fPitch = 1.0f;
			pSubEffect->QueryFloatAttribute("pitch", &fPitch);
			float fPan = 0.5f;
			pSubEffect->QueryFloatAttribute("pan", &fPan);
			float fGain = 1.0f;
			pSubEffect->QueryFloatAttribute("gain", &fGain);

			this->runAction(SoundCreateAction::create(szSound, fDuration, fStart, bLoop, fPitch, fPan, fGain));
		}


	} while (pSubEffect = pSubEffect->NextSiblingElement());

	return true;
}

void FlashEffect::onSubEffectFinish()
{
	m_nSubEffectRuningCount --;
	if (m_nSubEffectRuningCount <= 0)
	{
		this->removeFromParent();
	}
}

ActionInterval * FlashEffect::parseXmlToAction(tinyxml2::XMLElement * action)
{
	if (nullptr == action)
	{
		return nullptr;
	}

	const char * DELAY			= "DelayTime";	//延时
	const char * EASE_IN		= "EaseIn";		//缓动
	const char * EASE_OUT		= "EaseOut";
	const char * FADE_IN		= "FadeIn";		//Alpha
	const char * FADE_OUT		= "FadeOut";
	const char * MOVE_BY		= "MoveBy";		//移动
	const char * MOVE_TO		= "MoveTo";
	const char * ROTATE_BY		= "RotateBy";	//旋转
	const char * ROTATE_TO		= "RotateTo";
	const char * ROTATE_X_TO	= "RotateXTo";	//绕X轴旋转
	const char * ROTATE_Y_TO	= "RotateYTo";	//绕Y轴旋转
	const char * SCALE_BY		= "ScaleBy";	//缩放
	const char * SCALE_TO		= "ScaleTo";
	const char * SEQUENCE		= "Sequence";	//链接
	const char * SHAKE			= "Shake";		//抖动
	const char * SKEW_BY		= "SkewBy";		//扭曲
	const char * SKEW_TO		= "SkewTo";
	const char * SPAWN			= "Spawn";		//合并
	const char * TINT_BY		= "TintBy";		//RGB
	const char * TINT_TO		= "TintTo";

	ActionInterval * pAction = nullptr;

	const char * type = action->Attribute("type");

	if (type == nullptr)
	{
		return nullptr;
	}
	else if (strcmp(type, DELAY) == 0)
	{
		float d;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("d", &d))
		{
			return nullptr;
		}

		pAction = DelayTime::create(d);
	}
	else if (strcmp(type, SHAKE) == 0)
	{
		float d, x, y, r;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("d", &d))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("x", &x))
		{
			x = 2;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("y", &y))
		{
			y = 2;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("r", &r))
		{
			r = 10;
		}

		pAction = createShakeAction(d, Point(x,y), r);
	}
	else if (strcmp(type, MOVE_BY) == 0)
	{
		float d, x, y;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("d", &d))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("x", &x))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("y", &y))
		{
			return nullptr;
		}

		pAction = MoveBy::create(d, Point(x,y));
	}
	else if (strcmp(type, MOVE_TO) == 0)
	{
		float d, x, y;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("d", &d))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("x", &x))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("y", &y))
		{
			return nullptr;
		}

		pAction = MoveTo::create(d, Point(x,y));
	}
	else if (strcmp(type, ROTATE_BY) == 0)
	{
		float d, x, y;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("d", &d))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("x", &x))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("y", &y))
		{
			return nullptr;
		}

		pAction = RotateBy::create(d, x, y);
	}
	else if (strcmp(type, ROTATE_TO) == 0)
	{
		float d, x, y;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("d", &d))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("x", &x))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("y", &y))
		{
			return nullptr;
		}

		pAction = RotateTo::create(d, x, y);
	}
	else if (strcmp(type, SCALE_BY) == 0)
	{
		float d, x, y;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("d", &d))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("x", &x))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("y", &y))
		{
			return nullptr;
		}

		pAction = ScaleBy::create(d, x, y);
	}
	else if (strcmp(type, SCALE_TO) == 0)
	{
		float d, x, y;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("d", &d))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("x", &x))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("y", &y))
		{
			return nullptr;
		}

		pAction = ScaleTo::create(d, x, y);
	}
	else if (strcmp(type, SKEW_BY) == 0)
	{
		float d, x, y;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("d", &d))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("x", &x))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("y", &y))
		{
			return nullptr;
		}

		pAction = SkewBy::create(d, x, y);
	}
	else if (strcmp(type, SKEW_TO) == 0)
	{
		float d, x, y;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("d", &d))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("x", &x))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("y", &y))
		{
			return nullptr;
		}

		pAction = SkewTo::create(d, x, y);
	}
	else if (strcmp(type, TINT_BY) == 0)
	{
		float d, r, g, b;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("d", &d))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("r", &r))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("g", &g))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("b", &b))
		{
			return nullptr;
		}

		pAction = TintBy::create(d, r, g, b);
	}
	else if (strcmp(type, TINT_TO) == 0)
	{
		float d, r, g, b;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("d", &d))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("r", &r))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("g", &g))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("b", &b))
		{
			return nullptr;
		}

		pAction = TintTo::create(d, r, g, b);
	}
	else if (strcmp(type, EASE_IN) == 0)
	{
		float d;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("r", &d))
		{
			return nullptr;
		}

		tinyxml2::XMLElement * sub = action->FirstChildElement();
		if (nullptr == sub)
		{
			return nullptr;
		}

		pAction = parseXmlToAction(sub);
		if (nullptr == pAction)
		{
			return nullptr;
		}

		pAction = EaseIn::create(pAction, d);
	}
	else if (strcmp(type, EASE_OUT) == 0)
	{
		float d;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("r", &d))
		{
			return nullptr;
		}

		tinyxml2::XMLElement * sub = action->FirstChildElement();
		if (nullptr == sub)
		{
			return nullptr;
		}

		pAction = parseXmlToAction(sub);
		if (nullptr == pAction)
		{
			return nullptr;
		}

		pAction = EaseOut::create(pAction, d);
	}
	else if (strcmp(type, FADE_IN) == 0)
	{
		float d;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("d", &d))
		{
			return nullptr;
		}

		pAction = FadeIn::create(d);
	}
	else if (strcmp(type, FADE_OUT) == 0)
	{
		float d;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("d", &d))
		{
			return nullptr;
		}

		pAction = FadeOut::create(d);
	}
	else if (strcmp(type, ROTATE_X_TO) == 0)
	{
		float d, a;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("d", &d))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("a", &a))
		{
			return nullptr;
		}

		/************************************************************************/
		/* 该动作目前还未实现                                                   */
		/************************************************************************/
		pAction = DelayTime::create(d);
	}
	else if (strcmp(type, ROTATE_Y_TO) == 0)
	{
		float d, a;
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("d", &d))
		{
			return nullptr;
		}
		if (tinyxml2::XML_NO_ERROR != action->QueryFloatAttribute("a", &a))
		{
			return nullptr;
		}

		/************************************************************************/
		/* 该动作目前还未实现                                                   */
		/************************************************************************/
		pAction = DelayTime::create(d);
	}
	else if (strcmp(type, SPAWN) == 0)
	{
		cocos2d::Vector<FiniteTimeAction *> list;

		tinyxml2::XMLElement * sub = action->FirstChildElement();
		if (nullptr == sub)
		{
			return nullptr;
		}

		do 
		{
			pAction = parseXmlToAction(sub);
			if (nullptr == pAction)
			{
				return nullptr;
			}

			list.pushBack(pAction);

		} while (nullptr != (sub = sub->NextSiblingElement()));

		pAction = Spawn::create(list);
	}
	else if (strcmp(type, SEQUENCE) == 0)
	{
		cocos2d::Vector<FiniteTimeAction *> list;

		tinyxml2::XMLElement * sub = action->FirstChildElement();
		if (nullptr == sub)
		{
			return nullptr;
		}

		do 
		{
			pAction = parseXmlToAction(sub);
			if (nullptr == pAction)
			{
				return nullptr;
			}

			list.pushBack(pAction);

		} while (nullptr != (sub = sub->NextSiblingElement()));

		pAction = Sequence::create(list);
	}
	else
	{
		return nullptr;
	}

	return pAction;
}

ActionInterval * FlashEffect::createShakeAction(float duration, Point range/*=Point(2,2)*/, float rate/*=10*/)
{
	//抖动效果
	return Repeat::create(Sequence::create(
		MoveBy::create((1.0f / rate) * 0.5f, range),
		MoveBy::create((1.0f / rate) * 0.5f, -range),
		NULL
		), duration * rate);
}


//////////////////////////////////////////////////////////////////////////

FlashEffectManager * FlashEffectManager::s_Instance = nullptr;

bool FlashEffectManager::loadConfig( const char * xml_file )
{
	CCASSERT(xml_file, "xml_file is null point.");
	CCASSERT(nullptr == m_pXmlDocument, "m_pXmlDocument is't null.");
	if (nullptr == xml_file)
	{
		return false;
	}

	//检查文件是否存在
	if (!FileUtils::getInstance()->isFileExist(xml_file))
	{
		CCASSERT(false, "xml_file is't exist.");
		return false;
	}

	//文件路径格式检查
	FileUtils * pFileUtils = FileUtils::getInstance();
	if (nullptr == pFileUtils)
	{
		return false;
	}
	std::string strFileName = pFileUtils->fullPathForFilename(xml_file);


	//加载XML
	m_pXmlDocument = new tinyxml2::XMLDocument;
	if (m_pXmlDocument == nullptr)
	{
		CCASSERT(false, "m_pXmlDocument new fail.");
		return false;
	}

	if(tinyxml2::XML_NO_ERROR != m_pXmlDocument->LoadFile(strFileName.c_str()))
	{
		CCASSERT(false, "m_pXmlDocument load xml_file fail.");
		return false;
	}

	return true;
}

tinyxml2::XMLElement * FlashEffectManager::getEffectXmlNode( const char * effect_id )
{
	CCASSERT(effect_id, "effect_id is null.");
	CCASSERT(m_pXmlDocument, "m_pXmlDocument is null.");

	tinyxml2::XMLElement * pRoot = m_pXmlDocument->FirstChildElement();
	CCASSERT(pRoot, "pRoot is null.");
	if (nullptr == pRoot)
	{
		return nullptr;
	}

	tinyxml2::XMLElement * pNodeElement = pRoot->FirstChildElement();
	CCASSERT(pNodeElement, "pNodeElement is null.");
	if (pNodeElement == nullptr)
	{
		return nullptr;
	}

	do 
	{
		if (pNodeElement->Attribute("id", effect_id))
		{
			return pNodeElement;
		}

	} while (pNodeElement = pNodeElement->NextSiblingElement());

	CCASSERT(false, "can't find effect by effect_id.");
	return nullptr;
}

FlashEffect * FlashEffectManager::createEffect( const char * effect_id )
{
	return FlashEffect::create(effect_id);
}
