	var getActiveElement = function( document ){

		document = document || window.document;

		// Check if the active element is in the main web or iframe
		if( document.body === document.activeElement
				|| document.activeElement.tagName == 'IFRAME' ){
			// Get iframes
			var iframes = document.getElementsByTagName('iframe');
			for(var i = 0; i<iframes.length; i++ ){
				// Recall
				var focused = getActiveElement( iframes[i].contentWindow.document );
				if( focused !== false ){
					return focused; // The focused
				}
			}
		}

		else return document.activeElement;

		return false;
	};	
	
	function goToRoom(urlobject) {
		
	var keyPressed = event.keyCode || event.which;

	if(keyPressed==13)
		{
			window.janus.launchurl(urlobject.value,1);
			urlobject.blur;
			window.janus.unfocus();
			
		}
	}
	
	function sendChat(chatobject) {
		
	var keyPressed = event.keyCode || event.which;

	if(keyPressed==13)
		{
			window.janus.chatsend(chatobject.value)
			chatobject.value = "";
			chatobject.blur;
			window.janus.unfocus();			
		}
	}
	
	var prevURL;
	function updateRoomURL(){
		
		if ((prevURL != window.janus.currenturl()) || ( (document.getElementById("myurl").value.length == 0) && (window.janus.hasFocus() == 0) ) )
		{
			prevURL = window.janus.currenturl();
			document.getElementById("myurl").value = prevURL;
		}
	
	
	}
	
	function updateProgressBar() {

		var progbar = document.getElementById("vrurlcontainer");
		var progressx = (window.janus.roomprogress())*200;

		document.getElementById("vrurlcontainer").style.backgroundSize = ""+progressx+"% 100%";

		
	}
	
	function pollFocus() {
		//this function polls to see if T or TAB has been pressed. if it has, focus on the chatlog/URL Bar
		
		if (window.janus.currentkey == "Tab" )
		{
			var eletemp=getActiveElement();
			if (   ((eletemp.tagName != "INPUT" && eletemp.tagName != "TEXTAREA") && window.janus.hasFocus()) ||(!(window.janus.hasFocus())))
			{
				if (!(window.janus.hasFocus()))
				{
					window.janus.focus();
				}
				document.getElementById("myurl").focus();
			}
		}
		
		if (window.janus.currentkey == "T" )
		{
			var eletemp=getActiveElement();
			if (   ((eletemp.tagName != "INPUT" && eletemp.tagName != "TEXTAREA") && window.janus.hasFocus()) ||(!(window.janus.hasFocus())))
			{
				if (!(window.janus.hasFocus()))
				{
					window.janus.focus();
				}
				document.getElementById("mychat").focus();
			}
		}		
		
	}

	window.onload = function() {
		
		setInterval(function() {
		updateProgressBar();
		updateRoomURL();	
		},500)
		
		setInterval(function() {
		pollFocus()
		},10)
				
		
				
		
	}