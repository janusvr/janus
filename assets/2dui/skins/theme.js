//FOR EVERYONE: Hey! To change your UI theme, extract the theme to your 2dui/skins/ folder and specify the paths to their main.css, mainVR.css and app.css below. Format these relative to your root directory
	var myinterfacetheme = "skins/oneironaut/main.css"; <!--This specifies where your main interface theme lives-->
	var myinterfaceVRtheme = "skins/oneironaut/mainVR.css"; <!--This specifies where your main interface's VR mode theme lives-->
	var myapptheme = "skins/oneironaut/app.css"; <!--This specifies where the theming for your applications (Like the Chat Log) lives-->
			


//FOR DEVELOPERS: Simply load this script and use the initializeAppTheme() function in order to use JanusVR's consistent CSS. This will allow you to be consistent with the user's chosen theme.

//This script will provide apps and the main interface with functions to initialize consistent themes. initilizeAppTheme takes an argument (prefix) so that you can nest an app multiple directories away from the
//root and have it function properly. Please look to the initializeAppTheme function for a better explanation.-->

















<!--------------------------------------------------------------------BEGIN CODE---------------------------------------------------------------------->


<!-- CSS Loading Functions-->
	function initializeMainInterfaceTheme() {
		loadjscssfile(myinterfacetheme,"css");	
	}

	function initializeMainInterfaceVRTheme() {
			if (window.janus.hmd != null)
		{
			if (window.janus.hmd == 1)
			{
				loadjscssfile(myinterfaceVRtheme,"css");
			}
		}
	}

	function initializeAppTheme(prefix) {
		//note that the prefix argument is how many folders away from the root directory the application is
		//for instance, if your app is two folders away from root, your function may be initializeAppTheme("../../");
		loadjscssfile(prefix+myapptheme,"css");	
		setTimeout(function(){ 	window.onresize(); }, 50); //This should solve the problem with apps that depend on an instant resize event since the css loads after the first resize takes place. However, you may have to call this event again if it still fails.
	
	}



<!-- Misc Functions -->
	 function loadjscssfile(filename, filetype) {
					if (filetype == "js") { //if filename is a external JavaScript file
					   // alert('called');
						var fileref = document.createElement('script')
						fileref.setAttribute("type", "text/javascript")
						fileref.setAttribute("src", filename)
						alert('called');
					}
					else if (filetype == "css") { //if filename is an external CSS file
						var fileref = document.createElement("link")
						fileref.setAttribute("rel", "stylesheet")
						fileref.setAttribute("type", "text/css")
						fileref.setAttribute("href", filename)
					}
					if (typeof fileref != "undefined")
						document.getElementsByTagName("head")[0].appendChild(fileref)
				}
				