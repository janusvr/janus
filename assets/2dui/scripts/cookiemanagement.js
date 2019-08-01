
	function setCookie(cname, cvalue, exdays) {
		var d = new Date();
		d.setTime(d.getTime() + (exdays*24*60*60*1000));
		var expires = "expires="+d.toUTCString();
		document.cookie = cname + "=" + cvalue + "; " + expires;
	}

	function getCookie(cname) {
		var name = cname + "=";
		var ca = document.cookie.split(';');
		for(var i = 0; i < ca.length; i++) {
			var c = ca[i];
			while (c.charAt(0) == ' ') {
				c = c.substring(1);
			}
			if (c.indexOf(name) == 0) {
				return c.substring(name.length, c.length);
			}
		}
		return "";
	}
	
		function addToCookie(cname, cvalue, exdays,separatorstring) {
			
		if 	(getCookie(cname) == "")
		{
		separatorstring="";
		}
		
		var d = new Date();
		d.setTime(d.getTime() + (exdays*24*60*60*1000));
		var expires = "expires="+d.toUTCString();
		document.cookie = cname + "=" + getCookie(cname)+separatorstring+cvalue + "; " + expires;
	}
	
	
	function logToConsole(consolestring)
	{
	
				if ( (document.getElementById("window_CONSOLE_windowarea") != null) )
				{
					addToCookie("consoledata",consolestring,"5","<br>*")
				}
		
	}
