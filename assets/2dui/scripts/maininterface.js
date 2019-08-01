

function loadVR2DUI() {
	if (window.janus.hmd != null)
	{
			if (window.janus.hmd == 1)
			{
				window.location = "apps/vrhud/index.html";
			}
	}
}

//to log to the F2 console use logToConsole(consolestring), or from an app add parent. to the front of the function


function updatePartyMode() {

	//check if party mode is disabled
	if (window.janus.getsetting('partymode') == true)
	{



		document.getElementById("partystatus").setAttribute("style","visibility: visible !important;")

		//determine graphic to use
		if (window.janus.roompartymode() == true )
		{
			document.getElementById("partystatus").onmousemove = function() {showtooltip('Broadcasting location.')}
			document.getElementById("partystatus").className = "stat statisticright unselectable";
		}
		else
		{

			document.getElementById("partystatus").onmousemove = function() {showtooltip('Party Mode prohibited.')}
			document.getElementById("partystatus").className = "stat statisticrightlocked unselectable";
		}

	}
	else
	{
		document.getElementById("partystatus").setAttribute("style","visibility: hidden !important;")
	}


}

	function include(src, cb) {
		cb = cb || function() {};

		if (src.substr(0,1) == '/')
		src = params.rootdir + src;

		src += '?d=' + new Date().getTime();
		var s = document.createElement('script');
		s.async = true;
		s.src = src;

		s.addEventListener('load', function() {
			cb();
		}, false);

		document.head.appendChild(s);

		return s;
	}

	window.addEventListener('load', function() {
		include('startup.js');
	}, false);

	<!-- register an app to be run at load, should be included in ./startup.js -->
	<!-- will run the contents of /apps/(appName)/startup.js -->
	function registerApp(appName) {
		include('apps/'+appName+'/startup.js');
	}

	<!-- adds a button to the quickbar, first 3 parameters are required -->
	<!-- element_name is the text displayed by the tooltip -->

	function addToQuickbar(element_id, element_name, element_class, element_onclick, element_icon) {
		if (element_id != "" && element_name != "" && element_class != "")
		{
			var quickbar_button = document.createElement("button");
			var t = document.createTextNode(" ");
			quickbar_button.appendChild(t);
			quickbar_button.setAttribute("id",element_id);
			quickbar_button.setAttribute("onmousemove","showtooltip('"+element_name+"')");
			quickbar_button.setAttribute("onmouseout","hidetooltip()");
			if (element_onclick != "")
			{
				quickbar_button.setAttribute("onclick",element_onclick);
			}
			quickbar_button.setAttribute("class",element_class);
			if (element_icon != "")
			{
				quickbar_button.style.backgroundImage = "url('"+element_icon+"')";
				quickbar_button.style.backgroundRepeat = 'no-repeat';
				quickbar_button.style.backgroundSize = 'contain';
				quickbar_button.style.backgroundPosition = 'center';
			}
			document.getElementById('quickbar').appendChild(quickbar_button);
		}
	}


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

	function vwToPx(vw) {
		vw = parseFloat(vw);
		var clientWidth = document.documentElement.clientWidth;
		return Math.round((vw/100) * clientWidth) + 'px';
	}

	function vhToPx(vh) {
		vh = parseFloat(vh);
		var clientHeight = document.documentElement.clientHeight;
		return Math.round((vh/100) * clientHeight) + 'px';
	}

	function pxToVw(px) {
		px = parseInt(px);
		var clientWidth = document.documentElement.clientWidth;
		return (px*100 / clientWidth)+"vw";
	}

	function pxToVh(px) {
		px = parseInt(px);
		var clientHeight = document.documentElement.clientHeight;
		return (px*100 / clientHeight)+"vh";
	}


	function formatAMPM() {
		var date = new Date();
		var hours = date.getHours();
		var minutes = date.getMinutes();
		var ampm = hours >= 12 ? 'PM' : 'AM';
		hours = hours % 12;
		hours = hours ? hours : 12; // the hour '0' should be '12'
		minutes = minutes < 10 ? '0'+minutes : minutes;
		var strTime = hours + ':' + minutes + ' ' + ampm;
		return strTime;
	}











	function strip(html)
	{
		var tmp = document.createElement("DIV");
		tmp.innerHTML = html;
		return tmp.textContent || tmp.innerText;
	}


	var progressx = 0;

	<!-- Noise when you click on buttons. -->
	clickSound = function() {
		document.getElementById('sound1').play();
	}

	closesound = function() {
		document.getElementById('closesound').play();
	}
	dragonsound = function() {
		document.getElementById('dragon').play();
	}
	dragoffsound = function() {
		document.getElementById('dragoff').play();
	}
	shadesound = function() {
		document.getElementById('shadesound').play();
	}
	notifysound = function() {

		document.getElementById('notifysound').play();



	}
	pinsound = function() {
		document.getElementById('pinsound').play();
	}

	<!-- Menu List -->

	function togglemenulist(closeornot) {


		if (document.getElementById("menulist").style.visibility == "visible")
		{
			clickSound()
		}
		if (document.getElementById("menulist").style.visibility == "visible" || closeornot=="true")
		{
			document.getElementById("menulist").style.visibility = "hidden"



		}
		else
		{
			document.getElementById("menulist").style.visibility = "visible"
			clickSound()
		}

	}



	function toggleavatarlist(closeornot) {


		if (document.getElementById("avatarlist").style.visibility == "visible")
		{
			clickSound()
		}
		if (document.getElementById("avatarlist").style.visibility == "visible" || closeornot=="true")
		{
			document.getElementById("avatarlist").style.visibility = "hidden"



		}
		else
		{
			document.getElementById("avatarlist").style.visibility = "visible"
			clickSound()
		}

	}



	<!-- Windowing System. -->

	var clicked = new Array(); <!--Establish array that keeps track of which windows have had their titlebars clicked on.-->
	var mousex;
	var mousey;
	var highestwindow = -1; //every time you click a window, it'll become higher than the last one
	var activeobject;
	var winx;
	var winy;


	function clickwindow(ele) {

		dragonsound();
		highestwindow += 1;
		ele.style.zIndex = highestwindow.toString();
		document.getElementById(ele.id+"_resizer").style.zIndex = highestwindow.toString();


		activeobject = ele;
		clicked[ele] = true;
		mousex = event.clientX;
		mousey = event.clientY;

		ele.style.backgroundImage = "url('skins/oneironaut/stripelit.png')";


		var dragaid = document.getElementById(ele.id.replace(' ','&nbsp;')+"_dragaid");
		dragaid.style.width = "100%";
		dragaid.style.height = "100%";


	}

	function unclickwindow(ele) { <!-- Happens on mouse up.-->
		dragoffsound();
		clicked[ele] = false;

		ele.style.backgroundImage = '';


		var dragaid = document.getElementById(ele.id.replace(' ','&nbsp;')+"_dragaid");
		dragaid.className = "almostinvisible"
		dragaid.style.width = "0";
		dragaid.style.height = "0";
	}

	function movewindow(ele) {

		if ((clicked[ele] == false) || (ele != activeobject))
		return;

		var deltaX = event.clientX - mousex;
		var deltaY = event.clientY - mousey;

		var anchor = parseInt(ele.getAttribute('data-anchor'));

		winx = ele.offsetLeft + deltaX; //return new position in px
		winy = ele.offsetTop + deltaY; //same here

		var winWidth = ele.offsetWidth;
		var winHeight = ele.offsetHeight;

		var clientWidth = document.documentElement.clientWidth;
		var clientHeight = document.documentElement.clientHeight

		// Convert px values in relative vw/vh values :D
		switch (anchor) {
		case Anchor.TopLeft:
			ele.style.left = (winx*100 / clientWidth)+"vw";
			ele.style.top = (winy*100 / clientHeight)+"vh";
			break;

		case Anchor.TopRight:
			ele.style.right = ((winx + winWidth)*100 / clientWidth)+"vw";
			ele.style.top = (winy*100 / clientHeight)+"vh";
			break;

		case Anchor.BottomLeft:
			ele.style.left = (winx*100 / clientWidth)+"vw";
			ele.style.bottom = ((winy + winHeight)*100 / clientHeight)+"vh";
			break;

		case Anchor.BottomRight:
			ele.style.right = ((winx + winWidth)*100 / clientWidth)+"vw";
			ele.style.bottom = ((winy + winHeight)*100 / clientHeight)+"vh";
			break;

		}


		mousex = event.clientX;
		mousey = event.clientY;

		var brdr = document.getElementById(ele.id+"_resizer");
		var div = document.getElementById(ele.id);
		var wind = document.getElementById(ele.id+"_windowarea");



		setResizeHandlePosition(brdr,div,wind);
	}












	//window resizing



	var resizehandlegrabbed = new Array(); //keep track of which resize handles are grabbed

	function clickresize(brdr,div) {


		resizehandlegrabbed[brdr.id] = true;

		var raid = document.getElementById(div.id+"_resizeaid")


		raid.style.width = "100%";
		raid.style.height = "100%";

		highestwindow +=1; //increase the highest window number
		div.style.zIndex = highestwindow;



	}

	function unclickresize(brdr,raid) {

		resizehandlegrabbed[brdr.id] =  false;
		raid.style.width = "0";
		raid.style.height = "0";
	}

	function setWindowSize(callerid, winid, width, height) {
		var titlebar = document.getElementById(winid);
		var win = document.getElementById(winid + '_windowarea');
		var brdr =  document.getElementById(titlebar.id+"_resizer");
		win.style.width = width;
		win.style.height = height;
		setResizeHandlePosition(brdr,titlebar,win);
	}

	function resizewindow(titlebar) {


		var wind = document.getElementById(titlebar.id+"_windowarea");
		var brdr =  document.getElementById(titlebar.id+"_resizer");


		if (!resizehandlegrabbed[brdr.id])
		return;

		var xsize = event.clientX-(titlebar.offsetLeft);
		var ysize = event.clientY-(titlebar.offsetTop+32); //32 for titlebar offset?


		var clientWidth = document.documentElement.clientWidth;
		var clientHeight = document.documentElement.clientHeight;

		var oldWidth = titlebar.offsetWidth;
		var oldHeight = titlebar.offsetHeight;
		var offsetHeight = titlebar.offsetHeight;
		var offsetLeft = titlebar.offsetLeft;
		var offsetTop = titlebar.offsetTop;
		var offsetBottom = (clientHeight - offsetTop) - oldHeight;
		var offsetRight = (clientWidth - offsetLeft) - oldWidth;

		wind.style.width = xsize + "px";
		wind.style.height = ysize + "px";

		var deltaX = titlebar.offsetWidth - oldWidth;
		var deltaY = titlebar.offsetHeight - oldHeight;

		var anchor = parseInt(titlebar.getAttribute('data-anchor'));
		switch (anchor) {
			/*case Anchor.TopLeft:
		wind.style.width = xsize + "px";
		wind.style.height = ysize + "px";
		break;*/
		case Anchor.TopRight:
			var newRight = ((offsetRight-deltaX) * 100 / clientWidth)+"vw";
			titlebar.style.right = newRight;
			break;
		case Anchor.BottomLeft:
			var newBottom = ((offsetBottom-deltaY) * 100 / clientHeight)+"vh";
			titlebar.style.bottom = newBottom;
			break;
		case Anchor.BottomRight:
			var newRight = ((offsetRight-deltaX) * 100 / clientWidth)+"vw";
			var newBottom = ((offsetBottom-deltaY) * 100 / clientHeight)+"vh";
			titlebar.style.right = newRight;
			titlebar.style.bottom = newBottom;
			break;
		}
		//reset the handled position after the scale
		setResizeHandlePosition(brdr,titlebar,wind);
	}

	//probably do the same thing did for window moving and create a 'resize aid' to help with the resizing















	<!--Window pin button-->

	function pinWindow(callerid,winid,pinbutton)
	{
		var win = document.getElementById(winid);

		if (win.classList.contains('pinnedwindow')) {
			win.classList.remove('pinnedwindow');
			pinbutton.className = "pinbutton";
			}
			else
			{
			win.classList.add('pinnedwindow');
			pinbutton.className = "pinbuttonactive";
			}

		pinsound();

	}





	<!--Window close button-->

	function closeWindow(callerid,winid)
	{
				if (callerid != "null") {
					callerid.classList.remove("quickbarselection");
				}

		var win = document.getElementById(winid);
		if (win.getAttribute('data-persist')) {
			if (win.style.display == 'none') {
				win.style.display = 'block';
				document.getElementById(winid+"_resizer").style.display = 'block';

				if (callerid != "null") {
					if (hasClass(callerid,"genericquickbutton"))
					{
					callerid.classList.add("quickbarselection");
					}
					clickSound();
				}
			} else {
				win.style.display = 'none';
				document.getElementById(winid+"_resizer").style.display = 'none';
				callerid.classList.remove("quickbarselection");
				closesound();

			}
		} else {

			document.getElementById(winid).remove();
			document.getElementById(winid+"_resizer").remove();

			if (callerid != "null") {
				callerid.style.borderColor = "";
			}
				closesound();
		}


	}

	function shadeWindow(callerid,winid,shademe,titlebarid) {
		//alert(winid);
		var win = document.getElementById(winid);
		var index;

		if (win.classList.contains('collapsed')) {
			win.classList.remove('collapsed');

			if (win.classList.contains('anchor-bottom')) {
				if (win.offsetTop < 0) {
					var clientHeight = document.documentElement.clientHeight;

					win.style.bottom = pxToVh(clientHeight - win.offsetHeight);
				}
			}
		} else {
			win.classList.add('collapsed');
		}

		if (document.getElementById(shademe).style.visibility == "") {
			document.getElementById(shademe).style.visibility = "hidden";
			document.getElementById(titlebarid.id+"_resizer").style.visibility = "hidden";
		}
		else {
			document.getElementById(shademe).style.visibility = "";
			document.getElementById(titlebarid.id+"_resizer").style.visibility = "";
		}

		shadesound();
	}

	/* Listen for messages from child windows to create new windows */
	window.addEventListener('message', function(ev) {
		var data = ev.data;
		switch (data.cmd) {
		case 'toggleWindow':
			toggleWindow(data.callerid, data.width, data.height, data.title, data.page, data.spawnx, data.spawny);

			break;

		case 'setWindowSize':
			setWindowSize(null, data.winid, data.width, data.height);
			break;
		}
	}, false);

	<!-- Window Toggle Button System -->

	var Anchor = {
		TopLeft : 1,
		TopRight : 2,
		BottomLeft : 3,
		BottomRight : 4,
	}

	function hasClass(element, cls) {
    return (' ' + element.className + ' ').indexOf(' ' + cls + ' ') > -1;
}

	function toggleWindow(callerid,width,height,title,page,spawnx,spawny,anchor, persist) { //set callerid to null if there is no element calling this function
		anchor = anchor || Anchor.TopLeft;


		var winid= 'window_'+title.replace(/ /g,'&nbsp;');

		if (document.getElementById(winid) != null) {


			closeWindow(callerid,winid);


		}
		else
		{
			clickSound()
			<!--Set calling elements border colour to green.-->

			if (callerid != "null")
			{


				if (hasClass(callerid,"genericquickbutton"))
				{
				callerid.classList.add("quickbarselection");
				}


			}
			highestwindow +=1; //increase the highest window number


			<!--Create border-->
			var brdr = document.createElement("div");
			brdr.className  = "resizehandle";
			brdr.setAttribute('id', "window_"+title.replace(/ /g,'&nbsp;')+"_resizer");
			brdr.style.zIndex = highestwindow;


			//create the titlebar element first so we can pass it into the resizewindow function
			var div = document.createElement("div");

			brdr.onmousemove = function() { resizewindow(div) };
			brdr.onmousedown = function() { clickresize(brdr,div) };
			brdr.onmouseup = function() { unclickresize(brdr) };
			document.body.appendChild(brdr);




			<!--Create titlebar (element is created up there)-->

			div.className  = "titlebar";
			div.onmousedown  = function(){clickwindow(this)};
			div.onmouseup  = function(){unclickwindow(this)};
			div.onmousemove  = function(){movewindow(this)};

			if (persist)
			div.setAttribute('data-persist', true);
			div.setAttribute('data-anchor', anchor);
			switch (anchor) {
			case Anchor.TopLeft:
				div.classList.add('anchor-top','anchor-left');
				div.style.top = spawny;
				div.style.left = spawnx;
				break;
			case Anchor.TopRight:
				div.classList.add('anchor-top','anchor-right');
				div.style.top = spawny;
				div.style.right = spawnx;
				break;
			case Anchor.BottomLeft:
				div.classList.add('anchor-bottom','anchor-left');
				div.style.bottom = spawny;
				div.style.left = spawnx;
				break;
			case Anchor.BottomRight:
				div.classList.add('anchor-bottom','anchor-left');
				div.style.bottom = spawny;
				div.style.right = spawnx;
				break;
			}



			div.style.zIndex = highestwindow;
			div.setAttribute('id', winid); //set the id of this window to window_windowtitlewithspacesremoved
			document.body.appendChild(div);





			<!--Create window title-->
			var span = document.createElement("span");
			span.innerHTML = title.toUpperCase();

			div.appendChild(span);


			<!--Create window drag aid-->
			var aid = document.createElement("div");
			aid.onmousemove  = function(){movewindow(this.parentElement)};
			aid.onmouseup  = function(){unclickwindow(this.parentElement)};
			aid.style.position = "fixed";
			aid.style.width = "0";
			aid.style.height = "0";
			aid.style.left = "0";
			aid.style.top = "0";
			aid.style.zIndex = "2222222222" <!-- We want this to be basically above everything.-->
			aid.setAttribute('id', "window_"+title.replace(/ /g,'&nbsp;')+"_dragaid");
			div.appendChild(aid);



			<!--Create window resize aid-->
			var raid = document.createElement("div");
			raid.onmousemove  = function(){resizewindow(div)};
			raid.onmouseup  = function(){unclickresize(brdr,raid)};
			raid.style.position = "fixed";
			raid.style.width = "0";
			raid.style.height = "0";
			raid.style.left = "0";
			raid.style.top = "0";
			raid.style.zIndex = "2222222222" <!-- We want this to be basically above everything.-->
			raid.setAttribute('id', "window_"+title.replace(/ /g,'&nbsp;')+"_resizeaid");
			div.appendChild(raid);





			<!--Create closebutton-->
			var div2 = document.createElement("div");
			div2.className  = "closebutton";
			div2.onmousedown = function(ev) { closeWindow(callerid,winid); ev.stopPropagation() };

			span.appendChild(div2);

			<!--Create pinbutton-->
			var divp1 = document.createElement("div");
			divp1.className  = "pinbutton";
			divp1.onmousedown = function(ev) { pinWindow(callerid,winid,divp1); ev.stopPropagation() };

			span.appendChild(divp1);

			<!--Create shadebutton-->
			var sb = document.createElement("div");
			sb.className  = "shadebutton";
			sb.onmousedown = function() { shadeWindow(callerid,winid,'window_'+title.replace(/ /g,'&nbsp;')+'_windowarea',div) };

			span.appendChild(sb);

			<!--Create Window-->

			var wind = document.createElement("div");
			wind.className  = "wind";

			wind.style.width = width+"px"; //change to vw to make responsive and 1000 to 100
			wind.style.height = height+"px";  //change to vh to make responsive



			wind.setAttribute('id', "window_"+title.replace(/ /g,'&nbsp;')+"_windowarea");
			div.appendChild(wind);

			//set resize handle to window's bottom and left (basically grab window left and top and add their width/height to get this.)
			setResizeHandlePosition(brdr,div,wind);

			var rootdir = location.pathname.split('/');
			rootdir.pop();

			rootdir = rootdir.join('/');

			var params = 'rootdir=' + rootdir + '&winid=' + winid;

			if (page.split('?').length > 1) {
				page += '&' + params;
			} else {
				page += '?' + params;
			}

			<!--Create Iframe-->
			var frame = document.createElement("iframe");
			frame.src = page;

			frame.onload = function() {

				frame.contentWindow.postMessage({
					cmd : 'wininit',
					winid : winid
				}, '*')
			}

			wind.appendChild(frame);
		}

	}

	function setResizeHandlePosition(brdr,div,wind) {

		brdr.style.top = ((div.offsetTop+wind.offsetHeight)+8)+"px" ;
		brdr.style.left = ((div.offsetLeft+wind.offsetWidth)-brdr.offsetWidth+6)+"px" ;


	}

	//on window resize, recalculate all the resize handle locations
	window.onresize = function(event) {

		var rhandles = document.getElementsByClassName("resizehandle"); //return all resize handles

		var i;
		for (i=0;i<rhandles.length;i++)
		{


			var ele = rhandles[i].id.substring(0,rhandles[i].id.length-8);
			var brdr = document.getElementById(ele+"_resizer");
			var div = document.getElementById(ele);
			var wind = document.getElementById(ele+"_windowarea");



			setResizeHandlePosition(brdr,div,wind);

		}


	};




	function shadeColor(color, percent) {

		var R = parseInt(color.substring(1,3),16);
		var G = parseInt(color.substring(3,5),16);
		var B = parseInt(color.substring(5,7),16);

		R = parseInt(R * (100 + percent) / 100);
		G = parseInt(G * (100 + percent) / 100);
		B = parseInt(B * (100 + percent) / 100);

		R = (R<255)?R:255;
		G = (G<255)?G:255;
		B = (B<255)?B:255;

		var RR = ((R.toString(16).length==1)?"0"+R.toString(16):R.toString(16));
		var GG = ((G.toString(16).length==1)?"0"+G.toString(16):G.toString(16));
		var BB = ((B.toString(16).length==1)?"0"+B.toString(16):B.toString(16));

		return "#"+RR+GG+BB;
	}






	<!-- hide notification -->

	function hidenotification() {

		document.getElementById("notificationcontainer").style.visibility = "hidden";
		closesound();

	}

	var currentdisplay=0;
	var notiftext= [];;
	var notifimage= [];;
	var notifbordercolor = [];
	var notifexecute= [];;
	var notifbgcolor= [];;
	//shownotification puts a new notification into the queue
	function shownotification(text,image,executeonclick,bgcolor) {
		notifysound();
		notifbgcolor[notifbgcolor.length] = bgcolor;
		notifbordercolor[notifbordercolor.length] = "#242A2D";//shadeColor(bgcolor,80); uncomment this to use a lighter version of the bg
		notiftext[notiftext.length]  = text;
		notifimage[notifimage.length] = image;
		notifexecute[notifexecute.length] = executeonclick;
		displaynotification(notiftext.length-1);
		<!--Later executeonclick can perform an action or something-->
	}
	//displaynotification actually displays the specified notification
	function displaynotification(index) {


		setTimeout(function(){
			if (currentdisplay < 1)
			{
				document.getElementById("notifnavleft").style.visibility = "hidden";
			}
			else
			{
				document.getElementById("notifnavleft").style.visibility = "";
			}
			if (currentdisplay >= notiftext.length-1)
			{
				document.getElementById("notifnavright").style.visibility = "hidden";
			}
			else
			{
				document.getElementById("notifnavright").style.visibility = "";
			}
		}, 20);

		currentdisplay = index;
		document.getElementById("notificationbg").style.backgroundColor = notifbgcolor[index];
		document.getElementById("notificationbg").style.borderRightColor = notifbordercolor[index];

		document.getElementById("notificationtext").innerHTML   = notiftext[index];
		document.getElementById("notificationimage").src = notifimage[index];
		document.getElementById("notificationcontainer").style.visibility = "visible";
		<!--Later executeonclick can perform an action or something-->
	}

	function navnotification(direction) {
				clickSound();
		//0 is previous, 1 is next
		if (direction == 0)
		{
			if (currentdisplay > 0){
			displaynotification(currentdisplay-1);

			}
			else
			{
			displaynotification(currentdisplay);
			}
		}
		else
		{
			if (currentdisplay < notiftext.length-1){
			displaynotification(currentdisplay+1);
			}
			else
			{
			displaynotification(currentdisplay);
			}
		}
	}

	function removenotification() {
	closesound();

		if (notiftext.length <= 1)
		{
			hidenotification()
			notiftext.splice(currentdisplay,1);
			notifimage.splice(currentdisplay,1);
			notifbordercolor.splice(currentdisplay,1);
			notifexecute.splice(currentdisplay,1);
			notifbgcolor.splice(currentdisplay,1);

		}

		else
		{
		//remove the associated notification entry
			 notiftext.splice(currentdisplay,1);
			 notifimage.splice(currentdisplay,1);
			 notifbordercolor.splice(currentdisplay,1);
			 notifexecute.splice(currentdisplay,1);
			 notifbgcolor.splice(currentdisplay,1);
			 navnotification(0)
		}


	}



	<!-- Save room -->

	function saveroom() {

		var savefilename = window.janus.saveroom();
		shownotification('You just saved the webspace to <b>'+savefilename+'</b> and your clipboard.','notifications/xml.png',"null","#323232");



	}

	function clickBack() {

		clickSound()

		window.janus.navback();

	}
	function clickForwards() {

		clickSound()

		window.janus.navforward();

	}


	function clickHome() {

		clickSound()

		window.janus.navhome();

	}



	<!-- Sync room -->

	function syncroom() {


		shownotification('You just synchronized everyone in this room with any objects you placed before they entered.','notifications/sync.png',"null","#323232");
		window.janus.sync();



	}

	function quitjanus() {



		window.janus.quit();



	}

	<!-- Go button -->

	function gotoroom() {

		var progbar = document.getElementById("progressbar");

		switch(progbar.value.toLowerCase()) <!--Determine if debug string or URL to visit.-->
		{
		case "debug://tailtoddle":
			toggleWindow("null",500,400,"TAMMY GARS MA TAIL TODDLE","https://www.youtube.com/embed/heV-ccihSuI?autoplay=true&loop=1","40vw","35vh");
			break;

		case "debug://version":
			shownotification("You are using JanusVR v"+window.janus.version,"notifications/logo.png","null","#323232");
			break;

		case "debug://rosebud":
			shownotification("Do you canoe? <b>("+(Math.round(Math.random()*100))+")</b>","notifications/call.png","null","#323232");
			break;
		case "debug://credits":
			toggleWindow('null',500,400,'CREDITS','apps/credits/index.html','40vw','35vh');
			break;

		case "debug://dizzpet":
			toggleWindow('null',128,192,'DIZZPET','apps/extras/dizzpet/index.html','10vw','20vh');
			break;

		case "debug://console":
			toggleWindow("null",500,400," CONSOLE","apps/console/index.html","40vw","35vh",Anchor.TopLeft,true);
			break;

		default:
			window.janus.launchurl(document.getElementById("progressbar").value,1);
		}

	}

	<!-- When enter is pressed -->
	function gotoroomenter() {
		var keyPressed = event.keyCode || event.which;

		if(keyPressed==13)
		{
			gotoroom()
			window.janus.unfocus();
			<!--unfocus field-->
			var progbar = document.getElementById("progressbar");
			progbar.blur();


		}



	}





	function showUpdateNotification() {


		if (window.janus.version < window.janus.versiononline) //if the current version differs from the version online
		{


			shownotification('JanusVR v'+window.janus.versiononline+' has been released. Please upgrade at http://janusvr.com.','notifications/logo.png',"null","#323232");

		}

	}



function displayTip() {

			if (getCookie("fact1") != "used")
			{
				shownotification('You are now in UI mode. Click on the <b>Main Menu</b> to perform a wide variety of useful features.','notifications/menu.png',"null","#323232");
				setCookie("fact1","used","100000000")
			}
			else if (getCookie("fact2") != "used")
			{
				shownotification('Want to return to your PocketSpace? Simply press the <b>PocketSpace</b> toggle button to the left of the URL Bar!','notifications/space.png',"null","#323232");
				setCookie("fact2","used","100000000")
			}
			else if (getCookie("fact3") != "used")
			{
				shownotification('Find other people easily! Click on the <b>Parties Icon</b> in the bottom bar to meet new users.','notifications/partymode.png',"null","#323232");
				setCookie("fact3","used","100000000")
			}



}


function initsound() {
		//init sounds (prevents webkit bug)
		pinsound();
		closesound();

}


	window.onload = function() {




		initializeMainInterfaceTheme()
		//initializeMainInterfaceVRTheme()
		loadVR2DUI()
		showUpdateNotification();
		initsound()		//init sounds (prevents webkit bug)


		if ((getCookie("lastversion") == "")||(getCookie("lastversion") != window.janus.version)) //if you have never previously used janus, or the last used version of janus is not the current version
		{
			shownotification('Welcome to JanusVR v'+window.janus.version+'! Press <b>ESC</b> to interact with the HUD.','notifications/logo.png',"null","#323232");

			//set last version checker to current version
			setCookie("lastversion",window.janus.version,36500)

		}




	};


	// BEGIN spyduck userid edits
	var reuse_id_attempt = 0;
	// END spyduck userid edits
	var tipshown;
	var opacityoftooltip;
	var previousroomurl;
	var previoususerid=window.janus.userid;
	//UPDATING HUD STATUS IN REAL TIME ====================================================================================================================================
	setInterval(function() {



		updatePartyMode()




		if (window.janus.hasFocus()) {

			//if UI focused for the first time, demonstrate how to use [...] menu



			if (tipshown != 1)
			{
			setTimeout(function(){ displayTip(); }, 1000);
			tipshown = 1;
			}



			//darken UI on focus

			//document.getElementById("fader").style.backgroundColor = "rgba(0,0,0,0.3)";



		}
		else
		{
			document.getElementById("fader").style.backgroundColor = "";
			tipshown = 0;
		}










		//change janus url based on what room you're in

		if (window.janus.currenturl() != previousroomurl)
		{
			document.getElementById("progressbar").value = window.janus.currenturl();
		}

		previousroomurl = window.janus.currenturl();



		//update statistic counters

		//if room is locked or unlocked change locked display

		switch(window.janus.networkstatus) {
		case 0:

			if (window.janus.roomserver == "private")
			{
			document.getElementById("onlinestatus").onmousemove = function() {showtooltip('This room is <b>private</b>. Nobody can see you here.')}
			document.getElementById("onlinestatus").className = "stat statisticleftprivate";
			}
			else
			{
			document.getElementById("onlinestatus").onmousemove = function() {showtooltip('You are <b>not connected</b> to a presence server.')}
			document.getElementById("onlinestatus").className = "stat statisticleftdisconnected";
			}



			break;
		case 1:
			document.getElementById("onlinestatus").onmousemove = function() {showtooltip('Attempting to connect to the <b>'+window.janus.roomserver+'</b> presence server.')}
			document.getElementById("onlinestatus").className = "stat statisticleftpending";

			break;
		case 2:
			document.getElementById("onlinestatus").onmousemove = function() {showtooltip('Online at <b>'+window.janus.roomserver+'</b> presence server.')}
			document.getElementById("onlinestatus").className = "stat statisticleft";

			break;
		case -1:
			document.getElementById("onlinestatus").className = "stat statisticleftdisconnected";
			// BEGIN Spyduck userid edits
			if (window.janus.networkerror == "User name is already in use")
			{
				var delimiter = "_"
				var preferred_userid = getCookie("janusvr_userid");
				if (preferred_userid == "")
				{
					reuse_id_attempt = 1;
					setCookie("janusvr_userid", window.janus.userid, "100000000");
					preferred_userid = window.janus.userid;
				}
				else
				{
					if (reuse_id_attempt == 0)
					{
						window.janus.userid = preferred_userid;
						reuse_id_attempt = 1;
					}
					else if (reuse_id_attempt == 1)
					{
						var adjectives = ["Cute","NewAge","Retro","Hippie","Uneasy","Red","Super","Ancient","Okay","Silly","Crazy","Gentle","Tasty","Happy","Giant","Tiny","New","Old","Fast","Quick","Mega","Giga","Friendly","Green","Blue","Purple","Artsy","Humble","Mecha","Clean","Shiny","Brave","Worldly","Social","Rainy","Cloudy","Sunny","Magic","Pure"];
						var index = Math.floor(Math.random() * adjectives.length);
						var prefix = adjectives[index] + delimiter + adjectives[Math.floor((index + Math.random() * adjectives.length / 2.0) % adjectives.length)] + delimiter;
						var lastid = window.janus.userid;
						window.janus.userid = prefix+preferred_userid;
						previoususerid = window.janus.userid;
						shownotification('Username <b>'+lastid+'</b> is already in use.<br>Others now see you as <b>'+strip(window.janus.userid)+'</b>.','notifications/user.png',"null","#323232");
					}
				}
			}
			// END Spyduck userid edits
			document.getElementById("onlinestatus").onmousemove = function() {showtooltip(strip(window.janus.networkerror))}
			break;
		default:
			document.getElementById("onlinestatus").className = "stat statisticleftdisconnected";

			document.getElementById("onlinestatus").onmousemove = function() {showtooltip('Something went wrong with the client.')}
		}

		//set player count and tricount
		document.getElementById("playercountstatus").innerHTML = window.janus.playercount-1;


		if ((window.janus.playercount-1) == 1)
		{
			document.getElementById("playercountstatuscontainer").onmousemove = function()  {showtooltip('<b>'+(window.janus.playerlist.length-1)+'</b> person here.')}
		}
		else
		{
			document.getElementById("playercountstatuscontainer").onmousemove = function() {showtooltip('<b>'+(window.janus.playerlist.length-1)+'</b> people here.')}

		}





		//if room is locked or unlocked change locked display

		if (window.janus.locked() == 0)
		{
			document.getElementById("lockedstatus").className = "stat statisticright2unlocked";
			document.getElementById("lockedstatus").onmousemove = function() {showtooltip('You <b>may edit</b> here.')}
		}
		else
		{
			document.getElementById("lockedstatus").className = "stat statisticright2";
			document.getElementById("lockedstatus").onmousemove = function() {showtooltip('<b>Cannot edit</b> here.')}
		}




		//when an iframe is being used, bring its window to the top

		if (document.activeElement.tagName == "IFRAME")
		{

			if (document.activeElement.parentElement.parentElement.style.zIndex < highestwindow)
			{
				highestwindow +=1;
				document.activeElement.parentElement.parentElement.style.zIndex = highestwindow;
			}



		}

		//when the tooltip is shown, steadily increase its opacity, and when it isn't shown, hide it

		if (document.getElementById("tooltip").style.visibility == "visible")
		{
			opacityoftooltip += 0.05;

		}
		else
		{

			opacityoftooltip = -0.75;
		}

		document.getElementById("tooltip").style.opacity = opacityoftooltip+"";


		//progressbar

		var progbar = document.getElementById("progressbar");
		progressx = (window.janus.roomprogress())*200;

		document.getElementById("progressbar").style.backgroundSize = ""+progressx+"% 100%";

		if (progressx < 200)
		{
			document.getElementById("contain").style.opacity = "1";
		}
		else
		{
			document.getElementById("contain").style.opacity = "";
		}










		//poll keys to see if TAB is pressed

		if (window.janus.currentkey == "Tab" )
		{


			var eletemp=getActiveElement();



			if (   ((eletemp.tagName != "INPUT" && eletemp.tagName != "TEXTAREA") && window.janus.hasFocus()) ||(!(window.janus.hasFocus())))
			{



				if (!(window.janus.hasFocus()))
				{
					window.janus.focus();
				}

				document.getElementById("progressbar").focus();

			}



		}




		//blur active element if janus lacks focus
		if (!(window.janus.hasFocus()))
		{
			document.activeElement.blur()
		}






		//poll keys to see if T is pressed

		if (window.janus.currentkey == "T" )
		{


			var eletemp=getActiveElement();


			//if, when the ui has focus, you are not currently using a window or input, or if it lacks ui focus, allow chatlog usage
			if (   ((eletemp.tagName != "INPUT" && eletemp.tagName != "TEXTAREA") && window.janus.hasFocus()) ||(!(window.janus.hasFocus())))
			{

				//if chatlog doesn't exist, create it
				if (document.getElementById("window_CHAT&nbsp;LOG_windowarea") == null)
				{
					toggleWindow(document.getElementById("chatlogbutton"),425,200,'CHAT LOG','apps/chatlog/index.html','1vw',pxToVh(50), Anchor.BottomLeft)
				}



				//if the UI is focused and the chatlog is not
				if (!(window.janus.hasFocus()))
				{
					window.janus.focus();
				}

				document.getElementById("window_CHAT&nbsp;LOG_windowarea").children[0].contentWindow.document.getElementById("entryform").focus();

			}



		}


		//poll keys to see if ~ is pressed

		if (window.janus.currentkey == "F2" )
		{


			var eletemp=getActiveElement();


			//if, when the ui has focus, you are not currently using a window or input, or if it lacks ui focus, allow chatlog usage
			if (   ((eletemp.tagName != "INPUT" && eletemp.tagName != "TEXTAREA") && window.janus.hasFocus()) ||(!(window.janus.hasFocus())))
			{

				//if chatlog doesn't exist, create it
				if ((document.getElementById("window_CONSOLE_windowarea") == null) || (document.getElementById("window_CONSOLE").style.display == "none") )
				{
					toggleWindow("null",500,400,"CONSOLE","apps/console/index.html","40vw","35vh",Anchor.TopLeft,true);
				}



				//if the UI is focused and the chatlog is not
				if (!(window.janus.hasFocus()))
				{
					window.janus.focus();
				}

				document.getElementById("window_CONSOLE_windowarea").children[0].contentWindow.document.getElementById("entryform").focus();

			}



		}











		if (document.getElementById("window_CHAT&nbsp;LOG_windowarea") !== null) {
			//check if chatlog is highlighted, and if it is, make it fully opaque
			if (document.activeElement == document.getElementById("window_CHAT&nbsp;LOG_windowarea").children[0])
			{


				//check if the input inside of this is selected

				var eletemp=getActiveElement();


				if ( window.janus.hasFocus() )
				{
					document.getElementById("window_CHAT&nbsp;LOG").style.opacity = "1";
				}
				else
				{
					document.getElementById("window_CHAT&nbsp;LOG").style.opacity = "";
				}


			}
			else
			{

				document.getElementById("window_CHAT&nbsp;LOG").style.opacity = "";
			}
		}


		document.getElementById("urlwidthtest").innerHTML =  document.getElementById("progressbar").value;


		if ( window.janus.hasFocus() == 0 )
		{


			//hide buttons on unfocus
			document.getElementById("buttonscontainer").style.left = -(document.getElementById("buttonscontainer").offsetWidth)+"px";
			document.getElementById("progressbar").style.left = -(document.getElementById("buttonscontainer").offsetWidth)+"px";
			document.getElementById("quickbar").style.bottom =  -(document.getElementById("quickbar").offsetHeight)+"px";


			document.getElementById("progressbar").style.width = (document.getElementById("urlwidthtest").offsetWidth+32)+"px";




		}
		else
		{

			//show buttons on unfocus
			document.getElementById("buttonscontainer").style.left = "";
			document.getElementById("progressbar").style.left = "";
			document.getElementById("quickbar").style.bottom = "";

			document.getElementById("progressbar").style.width = ""




		}


		//check if username has changed, and if it has, show notification

		if ((window.janus.userid != previoususerid) && (previoususerid != ""))
		{
			shownotification('Others now see you as <b>'+strip(window.janus.userid)+'</b>.','notifications/user.png',"null","#323232");
			previoususerid = window.janus.userid;
			// BEGIN Spyduck userid edits
			setCookie("janusvr_userid", window.janus.userid, "100000000");
			reuse_id_attempt = 0;
			// END Spyduck userid edits
		}






	}, 10);

	//END UPDATING HUD STATUS IN REAL TIME ====================================================================================================================================


	function showtooltip(whattoshow) {

		document.getElementById("tooltip").style.visibility = "visible";
		document.getElementById("tooltip").innerHTML = whattoshow;


		if (( (event.clientX+(document.getElementById("tooltip").offsetWidth*0.5)) > document.body.clientWidth) )
		{
			document.getElementById("tooltip").style.left = "";
			document.getElementById("tooltip").style.right = "0px";
		}
		else if (( (event.clientX-(document.getElementById("tooltip").offsetWidth*0.5)) > 0) )
		{
			document.getElementById("tooltip").style.left = (event.clientX-(document.getElementById("tooltip").offsetWidth*0.5))+"px";
			document.getElementById("tooltip").style.right = "";
		}
		else
		{
			document.getElementById("tooltip").style.left = "0px";
			document.getElementById("tooltip").style.right = "";
		}



		if (event.clientY > window.innerHeight*0.5)
		{
			document.getElementById("tooltip").style.top = ((event.clientY-32)-(document.getElementById("tooltip").offsetHeight*0.75))+"px";
		}
		else
		{
			document.getElementById("tooltip").style.top = ((event.clientY+16)+(document.getElementById("tooltip").offsetHeight*0.75))+"px";
		}
		//height


	}

	function hidetooltip() {
		document.getElementById("tooltip").style.visibility = "";
		document.getElementById("tooltip").innerHTML = "";

	}

	function selectURLBar(caller)
	{
		caller.focus();
		caller.select();
		brightenURLContainer()
	}


	function blururlBar() {

		//when the url bar blurs, if it has nothing in it, put the url of the room back in it
		darkenURLContainer()
		if (document.getElementById("progressbar").value.trim().length == 0)
		{
			document.getElementById("progressbar").value = window.janus.currenturl();
		}

	}

	function brightenURLContainer() {
		document.getElementById("contain").style.opacity = "1";
	}
	function darkenURLContainer() {
		document.getElementById("contain").style.opacity = "";
	}
