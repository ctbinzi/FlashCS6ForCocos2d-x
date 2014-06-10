#ifndef __DISPLAY_PARAMETER_H__
#define __DISPLAY_PARAMETER_H__

#include "cocos2d.h"
#include "tinyxml2/tinyxml2.h"

USING_NS_CC;

class DisplayParameter
{
public:
	DisplayParameter():
		m_ptPosition(0,0),
		m_ptAnchorPoint(0.5f,0.5f),
		m_ptScale(1,1),
		m_fRotation(0),
		m_Color(1,1,1,1),
		m_nZOrder(0)
	{};

public:
	// 从XML读取数值
	bool readXml(const tinyxml2::XMLElement * xml);
	// 应用于cocos::node
	void apply(Node * node);

public:
	Point						m_ptPosition;				// 动画位置
	Point						m_ptAnchorPoint;			// 动画锚点
	Point						m_ptScale;					// 缩放比例
	float						m_fRotation;				// 动画旋转角度
	Color4F						m_Color;					// 起始颜色
	int							m_nZOrder;					// 显示层级排序
};


#endif // __DISPLAY_PARAMETER_H__