package
{
	import flash.desktop.NativeApplication;
	import flash.display.Sprite;
	import flash.events.InvokeEvent;
	import flash.net.SharedObject;
	
	public class FlashTransform extends Sprite
	{
		public function FlashTransform()
		{
			NativeApplication.nativeApplication.addEventListener(InvokeEvent.INVOKE, onInvoke);
		}
		
		private function onInvoke(e:InvokeEvent):void
		{
			NativeApplication.nativeApplication.removeEventListener(InvokeEvent.INVOKE, onInvoke);
			
			trace(e.arguments);
			if(e.arguments.length == 2)
			{
				// 将启动信息存入SharedObject，以便后续使用
				var so:SharedObject = SharedObject.getLocal("EffectTransform");
				if(so == null) return;
				so.data.src = e.arguments[0];
				so.data.dst = e.arguments[1];
			}
			
			this.doParse();
			
			NativeApplication.nativeApplication.exit();
		}
		
		private function doParse():void
		{
			// 从SharedObject读取路径信息
			var so:SharedObject = SharedObject.getLocal("EffectTransform");
			if(so == null) return;
			//so.data.dst = e.arguments[1];
			
			var dom_parser:DOMDocumentParser = new DOMDocumentParser();
			if(dom_parser == null) return;
			dom_parser.parse(so.data.src + "/DOMDocument.xml");
			dom_parser.save(so.data.dst + "/flash_effects.xml", so.data.dst + "/../Classes/FlashEffectDefine.h");
		}
	}
}