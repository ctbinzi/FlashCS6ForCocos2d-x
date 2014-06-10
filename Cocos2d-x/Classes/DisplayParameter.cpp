#include "DisplayParameter.h"
#include "CCVector.h"

USING_NS_CC;


bool DisplayParameter::readXml( const tinyxml2::XMLElement * xml )
{
	xml->QueryFloatAttribute("x", &(m_ptPosition.x));
	xml->QueryFloatAttribute("y", &(m_ptPosition.y));
	xml->QueryFloatAttribute("ax", &(m_ptAnchorPoint.x));
	xml->QueryFloatAttribute("ay", &(m_ptAnchorPoint.y));
	xml->QueryFloatAttribute("sx", &(m_ptScale.x));
	xml->QueryFloatAttribute("sy", &(m_ptScale.y));
	xml->QueryFloatAttribute("ca", &(m_Color.a));
	xml->QueryFloatAttribute("cr", &(m_Color.r));
	xml->QueryFloatAttribute("cg", &(m_Color.g));
	xml->QueryFloatAttribute("cb", &(m_Color.b));
	xml->QueryFloatAttribute("r", &(m_fRotation));
	xml->QueryIntAttribute("z", &m_nZOrder);
	return true;
}

void DisplayParameter::apply(Node * node)
{
	CCASSERT(node, "DisplayParameter::apply node is null.");
	if (nullptr == node)
	{
		return;
	}	

	node->setCascadeColorEnabled(true);
	node->setCascadeOpacityEnabled(true);
	node->setPosition(m_ptPosition);
	node->setAnchorPoint(m_ptAnchorPoint);
	node->setOpacity((GLubyte)(m_Color.a * 255));
	node->setColor((Color3B)m_Color);
	node->setRotation(m_fRotation);
	node->setScale(m_ptScale.x, m_ptScale.y);
	node->setLocalZOrder(m_nZOrder);
}