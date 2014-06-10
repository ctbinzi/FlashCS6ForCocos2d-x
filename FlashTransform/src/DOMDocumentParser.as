package
{
	import flash.display.Sprite;
	import flash.filesystem.File;
	import flash.filesystem.FileMode;
	import flash.filesystem.FileStream;
	import flash.geom.ColorTransform;
	import flash.geom.Matrix;
	import flash.utils.Dictionary;

	public class DOMDocumentParser
	{
		private var _vSymbols:Vector.<SymbolInfo>;
		private var _fDuationPerframe:Number;
		
		/*解析DOMDocument文件*/
		private const DELAY:String = "DelayTime"; //延时
		private const EASE_IN:String = "EaseIn"; //缓动
		private const EASE_OUT:String = "EaseOut";
		private const FADE_IN:String = "FadeIn"; //Alpha
		private const FADE_OUT:String = "FadeOut";
		private const MOVE_BY:String = "MoveBy"; //移动
		private const MOVE_TO:String = "MoveTo";
		private const ROTATE_BY:String = "RotateBy"; //旋转
		private const ROTATE_TO:String = "RotateTo";
		private const ROTATE_X_TO:String = "RotateXTo"; //绕X轴旋转
		private 	const ROTATE_Y_TO:String = "RotateYTo"; //绕Y轴旋转
		private const SCALE_BY:String = "ScaleBy"; //缩放
		private const SCALE_TO:String = "ScaleTo";
		private const SEQUENCE:String = "Sequence"; //链接
		private const SKEW_BY:String = "SkewBy"; //扭曲
		private const SKEW_TO:String = "SkewTo";
		private const SPAWN:String = "Spawn"; //合并
		private const TINT_BY:String = "TintBy"; //RGB
		private const TINT_TO:String = "TintTo";
		
		public function DOMDocumentParser()
		{
			this._vSymbols = new Vector.<SymbolInfo>;
		}
		
		//保存Cocos脚本
		public function save(cocos_file:String, define_file:String):Boolean{
			
			var xmlCocos:XML = <nodes/>;
			var strDefine:String = "";
			
			for each(var si:SymbolInfo in this._vSymbols){
				xmlCocos.appendChild(si._out_xml);
				if(si._link.length > 0){
					strDefine += "#define " + si._link + "		\"" + si._item_id + "\"\n";
				}
			}
			
			// 保存XML
			var file:File = new File(cocos_file);
			var fs:FileStream = new FileStream;
			fs.open(file, FileMode.WRITE);
			fs.writeUTFBytes(xmlCocos.toXMLString());
			fs.close();
			
			//保存定义文件
			file = new File(define_file);
			fs = new FileStream;
			fs.open(file, FileMode.WRITE);
			fs.writeUTFBytes(strDefine);
			fs.close();
			return true;
		}
		
		//解析FlashDOM文档
		public function parse(xml_file:String):Boolean
		{
			if(xml_file == null) return false;
			
			//文件检查
			var file:File = new File(xml_file);
			if(!file.exists) return false;
			
			/*加载DOMDocument文件*/
			var fs:FileStream = new FileStream();
			fs.open(file, FileMode.READ);
			var strXML:String = fs.readUTFBytes(fs.bytesAvailable);
			fs.close();
			
			strXML = strXML.replace("xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://ns.adobe.com/xfl/2008/\"", "");
			
			var xmlDOM:XML = new XML(strXML);
			
			//获取影片帧频率
			this._fDuationPerframe = 1.0/Number(xmlDOM.@frameRate);
			
			//获取所有库原件
			if(xmlDOM..symbols.Include.length() > 0)
			{
				// 读取所有MovieClip的标识信息
				for each (var symbol:XML in xmlDOM..symbols.Include)
				{
					if(symbol.@href.length() > 0)
					{
						var symbol_info:SymbolInfo = new SymbolInfo;
						symbol_info._href = symbol.@href;
						symbol_info._item_id = symbol.@itemID;
						
						this._vSymbols.push(symbol_info);
					}
				}
				
				// 转换所有MovieClip为CocosNode
				for each (var si:SymbolInfo in this._vSymbols)
				{
					var moviclipFile:File = file.parent.resolvePath("LIBRARY/" + si._href);
					si._out_xml = transformMovieClip(moviclipFile);
					if(si._out_xml){
						si._out_xml.@id = si._item_id;
						
						if(si._out_xml.@link.length()){
							si._link = si._out_xml.@link;
						}
					}
				}
			}
			return true;
		}
		
		private function transformMovieClip(file:File):XML
		{
			//文件检查
			if(!file) return null;
			if(!file.exists) return null;
			
			/*加载DOMDocument文件*/
			var fs:FileStream = new FileStream();
			fs.open(file, FileMode.READ);
			var strXML:String = fs.readUTFBytes(fs.bytesAvailable);
			fs.close();
			
			strXML = strXML.replace("xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://ns.adobe.com/xfl/2008/\"", "");
			
			//构建资源XML
			var xmlMovieClip:XML = new XML(strXML);
			
			//构建目标
			var xmlCocosNode:XML = <n/>;
			
			//获取link值
			if(xmlMovieClip.@linkageClassName.length()){
				xmlCocosNode.@link = xmlMovieClip.@linkageClassName;
			}
			
			//获取MovieClip总帧数
			var nTotalFrameCount:int = getTotalFrameCount(xmlMovieClip);
			
			//遍历图层
			var nZOrder:int = xmlMovieClip..DOMLayer.length();
			for each(var xmlLayer:XML in xmlMovieClip..DOMLayer){
				
				var xmlPrevFrame:XML = null;
				for each(var xmlFrame:XML in xmlLayer..DOMFrame){
					//跳过动画帧
					if(this.isMotionFrame(xmlFrame)){
						xmlPrevFrame = xmlFrame;
						continue;
					}
					
					//提取所有图片帧信息
					this.parseImageFame(xmlFrame, xmlCocosNode, nZOrder, nTotalFrameCount);
					
					//提取声音
					this.parseSoundFrame(xmlFrame, xmlCocosNode, nTotalFrameCount);
					
					//提取静态MovieClip
					if(!this.isMotionFrame(xmlPrevFrame)
						|| (xmlFrame.@duration.length() > 0 && parseInt(xmlFrame.@duration) > 1)){
						
						this.parseMovieChilpFrame(xmlFrame, xmlCocosNode, nZOrder, nTotalFrameCount);
					}
					
					xmlPrevFrame = xmlFrame;
				}
				
				//提取动画
				parseMotion(xmlLayer, xmlCocosNode, nZOrder);
				
				nZOrder --;
			}
			
			return xmlCocosNode;
		}
		
		private function getTotalFrameCount(xml_mc:XML):int{
			var nTotalFrameCount:int = 0;
			for each(var xmlLayer:XML in xml_mc..DOMLayer){
				for each(var xmlFrame:XML in xmlLayer..DOMFrame){
					var nIndex:int = xmlFrame.@index;
					var nDuration:int = 1;
					if(xmlFrame.@duration.length() > 0){
						nDuration = xmlFrame.@duration;
					}
					
					if(nIndex + nDuration > nTotalFrameCount){
						nTotalFrameCount = nIndex + nDuration;
					}
				}
			}
			
			return nTotalFrameCount;
		}
		
		private function getIdByName(name:String):String
		{
			for each (var symbol:SymbolInfo in this._vSymbols)
			{
				if(symbol._href.substr(0, symbol._href.length - 4) == name)
				{
					return symbol._item_id;
				}
			}
			
			throw(new Error("can't find symbol by name : " + name));
			return "";
		}
		
		private function isMotionFrame(xmlFrame:XML):Boolean{
			
			if(xmlFrame == null) return false;
			
			if(xmlFrame.@tweenType.length() > 0 && xmlFrame.@tweenType.toString() == "motion") return true;
			
			if(xmlFrame.@name.length() > 0) return true;
			
			return false;
		}
		
		private function parseMotion(xmlLayer:XML, xmlCocosNode:XML, nZOrder:int):void{
			
			var xmlPrevFrame:XML = null;
			var xmlNode:XML = null;
			var xmlActionSequence:XML = null;
			var strSymbol:String = null;
			var spTemp:Sprite = null;
			for each(var xmlFrame:XML in xmlLayer..DOMFrame){
				
				//是不是运动帧判断
				if(!this.isMotionFrame(xmlFrame) && !this.isMotionFrame(xmlPrevFrame)){
					
					this.attachAction(xmlNode, xmlActionSequence);
					
					xmlActionSequence = null;
					xmlNode = null;
					strSymbol = null;
					continue;
				}
				
				//一个运动帧下有且只有一个MovieClip实例
				var xmlSymbol:XML = xmlFrame..DOMSymbolInstance[0];
				if(xmlSymbol == null){
					continue;
				}
				
				//动画首帧判断 
				if(strSymbol == null){
					
					strSymbol = xmlSymbol.@libraryItemName.toString();
					
					xmlNode = <n/>;
					xmlNode.@id = this.getIdByName(strSymbol);
					xmlNode.@t = Number(xmlFrame.@index * this._fDuationPerframe).toFixed(3);
					
					// 读取初始属性
					spTemp = this.parseDisplayAttribut(xmlSymbol, xmlNode);
					
					xmlCocosNode.appendChild(xmlNode);
					
					xmlActionSequence = <a/>;
					xmlActionSequence.@type = SEQUENCE;
					
				}else{
					
					//解析运动动作
					var xmlAction:XML = this.parseDisplayMotion(xmlPrevFrame, xmlFrame);
					
					xmlAction = this.filterAction(xmlAction);
					if(xmlAction != null){
						xmlActionSequence.appendChild(xmlAction);
					}
				}
				
				xmlPrevFrame = xmlFrame;
			}
			
			this.attachAction(xmlNode, xmlActionSequence);
		}
		
		private function parseDisplayMotion(xmlOldFrame:XML, xmlNewFrame:XML):XML{
			
			if(xmlOldFrame == null
				|| xmlNewFrame == null){
				return null;
			}
			
			var fDuration:Number = (parseInt(xmlNewFrame.@index.toString()) - parseInt(xmlOldFrame.@index.toString())) * this._fDuationPerframe;
			
			//提取属性变换信息
			var spOld:Sprite = this.parseDisplayAttribut(xmlOldFrame);
			var spNew:Sprite = this.parseDisplayAttribut(xmlNewFrame);
			var xmlActionSpawn:XML = <a/>;
			xmlActionSpawn.@type = SPAWN;
			var xmlActionTemp:XML = null;
			
			//位置变换
			if(Math.abs(spOld.x - spNew.x) > 0.001
				|| Math.abs(spOld.y - spNew.y) > 0.001){
				xmlActionTemp = <a/>;
				xmlActionTemp.@type = MOVE_BY;
				xmlActionTemp.@d = fDuration.toFixed(3);
				xmlActionTemp.@x = (spNew.x - spOld.x).toFixed(0);
				xmlActionTemp.@y = (spOld.y - spNew.y).toFixed(0);
				xmlActionSpawn.appendChild(xmlActionTemp);
			}
			
			//角度变换
			{
				var fRotation:Number = spNew.rotation >= spOld.rotation ? spNew.rotation - spOld.rotation : 360 - (spNew.rotation - spOld.rotation);
				
				//自动旋转
				var autoRotation:Number = 0;
				if (xmlOldFrame.@motionTweenRotateTimes.length() > 0
				&& xmlOldFrame.@motionTweenRotateTimes.length() > 0)
				{
					autoRotation = xmlOldFrame.@motionTweenRotateTimes * 360;
				}
				else
				{
					if (fRotation > 180)
					{
						fRotation = 360 - fRotation;
					}
				}
				
				if (xmlOldFrame.@motionTweenRotate.length() && "counter-clockwise" == xmlOldFrame.@motionTweenRotate.toString())
				{
					fRotation = -autoRotation - (360 - fRotation);
				}
				else
				{
					fRotation += autoRotation;
				}
				
				if(Math.abs(fRotation) > 0.001){
					xmlActionTemp = <a/>;
					xmlActionTemp.@type = ROTATE_BY;
					xmlActionTemp.@d = fDuration.toFixed(3);
					xmlActionTemp.@x = fRotation.toFixed(1);
					xmlActionTemp.@y = fRotation.toFixed(1);
					xmlActionSpawn.appendChild(xmlActionTemp);
				}
			}
			
			//缩放变换
			if(Math.abs(spNew.scaleX - spOld.scaleX) > 0.001
				|| Math.abs(spNew.scaleY - spOld.scaleY) > 0.001){
				
				xmlActionTemp = <a/>;
				xmlActionTemp.@type = SCALE_TO;
				xmlActionTemp.@d = fDuration.toFixed(3);
				xmlActionTemp.@x = spNew.scaleX.toFixed(3);
				xmlActionTemp.@y = spNew.scaleY.toFixed(3);
				xmlActionSpawn.appendChild(xmlActionTemp);
			}
			
			//颜色变换
			{
				//Alpha变换
				var nNewAlpha:int = spNew.transform.colorTransform.alphaMultiplier * 255 + spNew.transform.colorTransform.alphaOffset;
				var nOldAlpha:int = spOld.transform.colorTransform.alphaMultiplier * 255 + spOld.transform.colorTransform.alphaOffset;
				if(nNewAlpha != nOldAlpha){
					xmlActionTemp = <a/>;
					xmlActionTemp.@type = nNewAlpha == 255 ? FADE_IN : FADE_OUT;
					xmlActionTemp.@d = fDuration.toFixed(3);
					xmlActionSpawn.appendChild(xmlActionTemp);
				}
				
				//RGB变换
				var nNewRed:int = spNew.transform.colorTransform.redMultiplier * 255 + spNew.transform.colorTransform.redOffset;
				var nNewGreen:int = spNew.transform.colorTransform.greenMultiplier * 255 + spNew.transform.colorTransform.greenOffset;
				var nNewBlue:int = spNew.transform.colorTransform.blueMultiplier * 255 + spNew.transform.colorTransform.blueOffset;
				var nOldRed:int = spOld.transform.colorTransform.redMultiplier * 255 + spOld.transform.colorTransform.redOffset;
				var nOldGreen:int = spOld.transform.colorTransform.greenMultiplier * 255 + spOld.transform.colorTransform.greenOffset;
				var nOldBlue:int = spOld.transform.colorTransform.blueMultiplier * 255 + spOld.transform.colorTransform.blueOffset;
				if(nNewRed != nOldRed
					|| nNewGreen != nOldGreen
					|| nNewBlue != nOldBlue){
					
					xmlActionTemp = <a/>;
					xmlActionTemp.@type = TINT_TO;
					xmlActionTemp.@d = fDuration.toFixed(3);
					xmlActionTemp.@r = nNewRed;
					xmlActionTemp.@g = nNewGreen;
					xmlActionTemp.@b = nNewBlue;
					xmlActionSpawn.appendChild(xmlActionTemp);
				}
			}
			
			//缓动效果解析
			var xmlActionEase:XML = null;
			if (xmlOldFrame.@acceleration.length()){
				
				var nAcceleration:int = xmlOldFrame.@acceleration;
				
				if (nAcceleration > 0)
				{
					xmlActionEase = <a/>
					xmlActionEase.@type = EASE_IN;
					xmlActionEase.@r = Number(nAcceleration * 2.5 / 100).toFixed(1);
				}
				else if (nAcceleration < 0)
				{
					xmlActionEase = <a/>
					xmlActionEase.@type = EASE_OUT;
					xmlActionEase.@r = Number(-nAcceleration * 2.5 / 100).toFixed(1);
				}
			}
			
			//帧标签动作解析
			if(xmlOldFrame.@name.length() > 0){
				
				var strFrameName:String = xmlOldFrame.@name;
				
				var strActions:Array = strFrameName.split(";");
				for each (var strAction:String in strActions)
				{
					var strActionName:String = strAction;
					var strParams:String = "";
					if (strAction.indexOf(":") > 0)
					{
						strActionName = strAction.substr(0, strAction.indexOf(":"));
						strParams = strAction.substr(strActionName.length + 1);
					}
					
					var strParamName:String, strParamValue:String;
					if (strActionName == EASE_IN
						|| strActionName == EASE_OUT)
					{
						// 缓动处理
						strParamName = strParams.substr(0, strParams.indexOf("="));
						strParamValue = strParams.substr(strParams.indexOf("=") + 1);
						
						if (strParamName == "r"){
							xmlActionEase.@type = strActionName
							xmlActionEase.@r = Number(strParamValue).toFixed(1);
						}
					}
					else
					{
						xmlActionTemp =     <a/>;
						xmlActionTemp.@type = strActionName;
						xmlActionTemp.@d = fDuration;
						
						var arrParams:Array = strParams.split(",");
						for each (var param:String in arrParams)
						{
							strParamName = param.substr(0, param.indexOf("="));
							strParamValue = param.substr(param.indexOf("=") + 1);
							
							if (strParamName.length > 0)
								xmlActionTemp.@[strParamName] = strParamValue;
						}
						
						xmlActionSpawn.appendChild(xmlActionTemp);
					}
				}
			}
			
			//动作有效性判断
			xmlActionSpawn = this.filterAction(xmlActionSpawn);
			if(xmlActionSpawn == null){
				return null;
			}
			
			//缓动效果判断
			if(xmlActionEase != null){
				xmlActionEase.appendChild(xmlActionSpawn);
				return xmlActionEase;
			}
			
			return xmlActionSpawn;
		}
		
		private function filterAction(xmlAction:XML):XML{
			if(xmlAction == null) return null;
			
			while(xmlAction.children().length() == 1
				&&  (/*xmlAction.@type == this.EASE_IN
					|| xmlAction.@type == this.EASE_OUT
					|| */xmlAction.@type == this.SPAWN
					|| xmlAction.@type == this.SEQUENCE)){
				
				xmlAction = xmlAction.children()[0];
			}
			
			// 空动作过滤
			if(xmlAction.children().length() == 0
				&& (xmlAction.@type == this.EASE_IN
					|| xmlAction.@type == this.EASE_OUT
					|| xmlAction.@type == this.SPAWN
					|| xmlAction.@type == this.SEQUENCE)){
				return null;
			}
			
			return xmlAction;
		}
		
		private function attachAction(xmlNode:XML, xmlAction:XML):void{
			
			if(xmlNode == null || xmlAction == null){
				return;
			}
			
			xmlAction = this.filterAction(xmlAction);
			if(xmlAction != null){
				xmlNode.appendChild(xmlAction);
			}
		}
		
		private function parseMovieChilpFrame(xmlFrame:XML, xmlCocosNode:XML, nZOrder:int, nTotalFrameCount:int):void{
			
			if(!xmlFrame || !xmlCocosNode) return;
			
			if(xmlFrame..DOMSymbolInstance.length() <= 0) return;
			
			for each(var xmlMovieClip:XML in xmlFrame..DOMSymbolInstance){
				var strItemID:String = this.getIdByName(xmlMovieClip.@libraryItemName);
				if(strItemID.length <= 0) continue;
				
				// 中心点检查
				if(xmlMovieClip..Point.length() > 0 && xmlMovieClip..Point[0].attributes().length() > 0)
				{
					throw(new Error("image's center point is't left top."))
					return;
				}
				
				//加入图片节点
				var n:XML = <n/>;
				n.@id = strItemID;
				n.@t = Number(xmlFrame.@index * this._fDuationPerframe).toFixed(3);
				var nDurationFrameCount:int = 1;
				if(xmlFrame.@duration.length() > 0){
					nDurationFrameCount = xmlFrame.@duration;
				}
				if(nDurationFrameCount == nTotalFrameCount){
					n.@d = 0;
				}else{
					n.@d = (nDurationFrameCount * this._fDuationPerframe).toFixed(3);
				}
				
				//状态矩阵
				parseDisplayAttribut(xmlMovieClip, n);
				
				n.@ax = 0.5;
				n.@ay = 0.5;
				n.@z = nZOrder;
				xmlCocosNode.appendChild(n);
			}
		}
		
		private function parseSoundFrame(xmlFrame:XML, xmlCocosNode:XML, nTotalFrameCount:int):void{
			
			if(!xmlFrame || !xmlCocosNode) return;
			
			if(xmlFrame..SoundEnvelopePoint.length() <= 0) return;
			
			for each(var xmlSound:XML in xmlFrame..SoundEnvelopePoint){
				var nVolumeLeft:int = 0, nVolumeRight:int = 0;
				if(xmlSound.@level0.length() >0 ) nVolumeLeft = xmlSound.@level0;
				if(xmlSound.@level1.length() > 0) nVolumeRight = xmlSound.@level1;
				
				// 加入声音节点
				var s:XML = <s/>;
				s.@file = xmlFrame.@soundName;
				s.@t = Number(xmlFrame.@index * this._fDuationPerframe).toFixed(3);
				var nDurationFrameCount:int = 1;
				if(xmlFrame.@duration.length() > 0){
					nDurationFrameCount = xmlFrame.@duration;
				}
				if(nDurationFrameCount == nTotalFrameCount){
					s.@d = 0;
				}else{
					s.@d = (nDurationFrameCount * this._fDuationPerframe).toFixed(3);
				}
				s.@loop = xmlFrame.@soundLoopMode.length() > 0 ? xmlFrame.@soundLoopMode.toString() == "loop" : false;
				s.@gain = Number(Math.max(nVolumeLeft, nVolumeRight) / 32767.0).toFixed(3);
				s.@pan = Number(0.5 + 0.5 * (nVolumeRight - nVolumeLeft) / Number(s.@gain)).toFixed(3);
				
				xmlCocosNode.appendChild(s);
			}
		}
		
		private function parseImageFame(xmlFrame:XML, xmlCocosNode:XML, nZOrder:int, nTotalFrameCount:int):void
		{
			if(!xmlFrame || !xmlCocosNode) return;
			
			if(xmlFrame..DOMBitmapInstance.length() <= 0) return;
		
			for each(var xmlImage:XML in xmlFrame..DOMBitmapInstance)
			{
				var strImage:String = xmlImage.@libraryItemName;
				if(strImage.length <= 0) continue;
				strImage = strImage.substr(strImage.lastIndexOf("/") + 1);
				
				// 中心点检查
				if(xmlImage..Point.length() > 0 && xmlImage..Point[0].attributes().length() > 0)
				{
					throw(new Error("image's center point is't left top."))
					return;
				}
				
				//加入图片节点
				var p:XML = <p/>;
				p.@img = strImage;
				p.@t = Number(xmlFrame.@index * this._fDuationPerframe).toFixed(3);
				var nDurationFrameCount:int = 1;
				if(xmlFrame.@duration.length() > 0){
					nDurationFrameCount = xmlFrame.@duration;
				}
				if(nDurationFrameCount == nTotalFrameCount){
					p.@d = 0;
				}else{
					p.@d = (nDurationFrameCount * this._fDuationPerframe).toFixed(3);
				}
				
				//状态矩阵
				parseDisplayAttribut(xmlImage, p);
				
				p.@ax = 0;
				p.@ay = 1;
				p.@z = nZOrder;
				xmlCocosNode.appendChild(p);
			}
		}
		
		private function parseDisplayAttribut(xmlSrc:XML, xmlCocosNode:XML=null):Sprite
		{
			var spResult:Sprite = new Sprite;
			
			if(!xmlSrc) return spResult;
						
			//位置、旋转、缩放等信息
			var mt:Matrix = new Matrix;
			if(xmlSrc..Matrix.length() > 0)
			{
				var xmlMt:XML = xmlSrc..Matrix[0];
				if (xmlMt.@a.length())
				{
					mt.a = xmlMt.@a;
				}
				if (xmlMt.@b.length())
				{
					mt.b = xmlMt.@b;
				}
				if (xmlMt.@c.length())
				{
					mt.c = xmlMt.@c;
				}
				if (xmlMt.@d.length())
				{
					mt.d = xmlMt.@d;
				}
				if (xmlMt.@tx.length())
				{
					mt.tx = xmlMt.@tx;
				}
				if (xmlMt.@ty.length())
				{
					mt.ty = xmlMt.@ty;
				}
			}
			spResult.transform.matrix = mt;
			
			//颜色变换信息
			var cr:ColorTransform = new ColorTransform;
			if (xmlSrc..Color.length())
			{
				var xmlCr:XML = xmlSrc..Color[0];
				if (xmlCr.@alphaMultiplier.length())
				{
					cr.alphaMultiplier = xmlCr.@alphaMultiplier;
				}
				if (xmlCr.@redMultiplier.length())
				{
					cr.redMultiplier = xmlCr.@redMultiplier;
				}
				if (xmlCr.@blueMultiplier.length())
				{
					cr.blueMultiplier = xmlCr.@blueMultiplier;
				}
				if (xmlCr.@greenMultiplier.length())
				{
					cr.greenMultiplier = xmlCr.@greenMultiplier;
				}
				if (xmlCr.@alphaOffset.length())
				{
					cr.alphaOffset = xmlCr.@alphaOffset;
				}
				if (xmlCr.@redOffset.length())
				{
					cr.redOffset = xmlCr.@redOffset;
				}
				if (xmlCr.@blueOffset.length())
				{
					cr.blueOffset = xmlCr.@blueOffset;
				}
				if (xmlCr.@greenOffset.length())
				{
					cr.greenOffset = xmlCr.@greenOffset;
				}
			}
			spResult.transform.colorTransform = cr;
			
			// 将结果存入cocos节点
			if(xmlCocosNode){
				if(spResult.x != 0)
					xmlCocosNode.@x = spResult.x.toFixed(0);
				if(spResult.y != 0)
					xmlCocosNode.@y = (-spResult.y).toFixed(0);
				if(spResult.rotation != 0)
					xmlCocosNode.@r = spResult.rotation.toFixed(1);
				if(spResult.scaleX != 1)
					xmlCocosNode.@sx = spResult.scaleX.toFixed(3);
				if(spResult.scaleY != 1)
					xmlCocosNode.@sy = spResult.scaleY.toFixed(3);
				
				if(cr.alphaMultiplier * 0xFF + cr.alphaOffset != 255)
					xmlCocosNode.@ca = ((cr.alphaMultiplier * 0xFF + cr.alphaOffset) / 255.0).toFixed(2);
				if(cr.redMultiplier * 0xFF + cr.redOffset != 255)
					xmlCocosNode.@cr = ((cr.redMultiplier * 0xFF + cr.redOffset) / 255.0).toFixed(2);
				if(cr.greenMultiplier * 0xFF + cr.greenOffset != 255)
					xmlCocosNode.@cg = ((cr.greenMultiplier * 0xFF + cr.greenOffset) / 255.0).toFixed(2);
				if(cr.blueMultiplier * 0xFF + cr.blueOffset != 255)
					xmlCocosNode.@cb = ((cr.blueMultiplier * 0xFF + cr.blueOffset) / 255.0).toFixed(2);
			}
			
			return spResult;
		}
	}
}