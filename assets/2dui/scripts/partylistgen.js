 


  
	function randomNumber(seed) {
	
	var origseed = seed;
		if (typeof seed == "string")
		{
		seed = origseed.length*origseed.charCodeAt(1);
		}
	
    var x = Math.sin(seed++) * 10000;

    return x - Math.floor(x);
	
	}
  
	function strip(html)
	{
		var tmp = document.createElement("DIV");
		tmp.innerHTML = html;
		return tmp.textContent || tmp.innerText;
	}
	
	function trimString(text,mylength) {
	

	var trimmedString = text.length > mylength ? 
                    text.substring(0, mylength - 3) + "..." : 
                    text;	 
					
	return trimmedString;
	
	}
	
	function extractDomain(url) {
		var domain;
		if (url != null)
		{
		//find & remove protocol (http, ftp, etc.) and get domain
		if (url.indexOf("://") > -1) {
			domain = url.split('/')[2];
		}
		else {
			domain = url.split('/')[0];
		}

		//find & remove port number
		domain = domain.split(':')[0];
		}
		else
		{
		domain = "URL Not Found";	
		}
		return domain;
}
  
    function launchPortal(url) {
  
  parent.window.janus.launchurl(url,1);
  
  }
  
  
//the only purpose of this function is to populate the partymode object
//if you're creating a list based on different data you don't need to use this
function populatePartyObject() {
parent.window.janus.updatepartymodedata() 


}

function getFileName(url) {
//this removes the anchor at the end, if there is one
url = url.substring(0, (url.indexOf("#") == -1) ? url.length : url.indexOf("#"));
//this removes the query after the file name, if there is one
url = url.substring(0, (url.indexOf("?") == -1) ? url.length : url.indexOf("?"));
//this removes everything before the last slash in the path
url = url.substring(url.lastIndexOf("/") + 1, url.length);
//return
return url;
}

var sitename;

  function createParties(containertopopulate,arraytype) {
	
  containertopopulate.innerHTML = "" //blank out the party container so we can refresh it with new data
	mainarray = returnMainArray(arraytype);

  if (mainarray.length > 0)
  {
	  var i;
	  for (i=0;i<mainarray.length;i++)
	  {
		var newelement=document.createElement("div");
		newelement.className="partyitem";
		
		
		if (arraytype == "partymode")
		{
			if (mainarray[i].name == null || mainarray[i].name == "" )
			{
				

				sitename = extractDomain(mainarray[i].url);

			
			
			}
			else
			{
				sitename = mainarray[i].name
			}
		}
		else if ((arraytype == "bookmarks"))
		{
			if (mainarray[i].title == null || mainarray[i].title == "" )
			{
				

				sitename = extractDomain(mainarray[i].url);

			
			
			}
			else
			{
				sitename = mainarray[i].title
			}			
		}
		else if (arraytype == "workspaces")
		{
			sitename = "";
		}
		
		
		
		if (arraytype == "partymode")
		{
			
			var specificname = "with "+trimString(strip(mainarray[i].userId),32);
		}
		else if ((arraytype == "bookmarks") || (arraytype == "workspaces"))
		{
			newelement.setAttribute("style","background:linear-gradient(rgba(0, 0, 0, 0.7),rgba(0, 0, 0, 0.7)), url('" + mainarray[i].thumbnail + "') no-repeat scroll center;visibility: visible !important;")
			
			
		
			if (arraytype == "bookmarks")
			{
			var filename = getFileName(mainarray[i].url);
			}
			else
			{
			var filename = mainarray[i].url;
			}
			
			
			if (filename != "")
			{
				var specificname = "("+filename+")";
			}
			else
			{
				var specificname = ""
			}
			
			
		}
		
		
		
		
		newelement.innerHTML = "<div class='partytext'><b>"+strip(sitename)+"</b><br>"+specificname+"</div>";
		newelement.onclick =  function(arg) {
										return function() {
											launchPortal(document.getElementById(arraytype+"myurl"+[arg]).innerHTML)
												}
												}(i);
												
		containertopopulate.appendChild(newelement);
		
		var urlcontainer=document.createElement("div");
		urlcontainer.className="partyhideurl";
		urlcontainer.id = function(arg) {
										return arraytype+"myurl"+[arg];
												}(i);
												
		urlcontainer.innerHTML = mainarray[i].url;
		newelement.appendChild(urlcontainer);
		
		
	  
	  }
	
	document.getElementById("windowarea").style.backgroundImage = "url(../../backgrounds/stars.png)"

	}
	else
	{
	
		if (arraytype == "partymode")
		{
			document.getElementById("windowarea").style.backgroundImage = "url(../../backgrounds/starssearch.png)"
		}
	}
	
  


  }
  


  
   window.onresize = function(event) { //make sure the window area stays a consistent size


 
 if (document.getElementById("tabrow").offsetWidth >= 200) //if you change the 200 here, also change the min-width of the window containing it. this is to prevent flicker when vertically resizing a minimally (width) sized window
 {
document.getElementById("windowarea").style.height = "calc(100% - "+(document.getElementsByClassName("tabrow")[0].offsetHeight+3)+"px)";	
 }
};

function returnMainArray(arraytype) {
	if (arraytype == "partymode")
	{
		xx = parent.window.janus.partymodedata;
	}
	else if (arraytype == "bookmarks")
	{
		xx = parent.window.janus.bookmarks;		
	}
	else if (arraytype == "workspaces")
	{
		
		xx = parent.window.janus.workspaces;
	}
	return xx;		
}

//array type can be partymode or bookmarks
function setRegularPartyUpdate(containertopopulate,arraytype,refreshtiming,regularpartydataupdate) {
	
mainarray = returnMainArray(arraytype);

if (regularpartydataupdate == 1) {
	populatePartyObject();
	setInterval(function(){ populatePartyObject(); },15000) //update the list every 15 seconds   
}

	
createParties(containertopopulate,arraytype);
setInterval(function(){   createParties(containertopopulate,arraytype); }, refreshtiming); //rebuild the list every five seconds 

}
