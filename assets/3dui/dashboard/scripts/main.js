//Init below

var interval;
var intervalParty;
var intervalPopular;
var intervalPopulateProperties;
var trendingdata = [];

window.onload = function() {

interval = setInterval(function(){ populateBookmarks() }, 250);
intervalPopulateProperties = setInterval(function(){ populateProperties() }, 100);
updatetrendingdata();
}

function populateBookmarks() {

  if (typeof window.janus.bookmarks != "undefined")
    {

      refreshBookmarks();
      clearInterval(interval);

    }

}

function populateParty(force) {

  if ( ( (typeof window.janus.partymodedata != "undefined") && (window.janus.partymodedata.length > 0) ) || (force == 1) )
    {
      refreshPartymode();

      window.janus.updatepartymodedata();
      clearInterval(intervalParty);
      intervalParty = setInterval(function(){ populateParty(1) }, 15000);


    }

}

function populatePopular() {

  if (typeof window.janus.populardata != "undefined" && window.janus.populardata.length > 0)
    {
      refreshPopular();
      clearInterval(intervalPopular);

    }

}

function populateProperties() {

  if (typeof window.janus.populardata != "undefined")
    {

      window.janus.updatepartymodedata();
      window.janus.updatepopulardata("?orderBy=weight&desc=true&limit=50");
      intervalParty = setInterval(function(){ populateParty() }, 250);
      intervalPopular = setInterval(function(){ refreshTrending() }, 250);
      clearInterval(intervalPopulateProperties);

    }

}
//End init







function strip(html)
{
var tmp = document.createElement("DIV");
tmp.innerHTML = html;
return tmp.textContent || tmp.innerText;
}


function emptyElement(ele) {
  ele.innerHTML = "";
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



function refreshBookmarks() {

    //Populate list with now available window.janus.bookmarks array.

    emptyElement(document.getElementById("3DList"));

    for (var i=0;i<window.janus.bookmarks.length;i++)
    {

      var thumb= window.janus.bookmarks[i].thumbnail ;

      var title= window.janus.bookmarks[i].title || extractDomain(window.janus.bookmarks[i].url) || "Untitled Webspace";

      var url = window.janus.bookmarks[i].url;


      document.getElementById("3DList").innerHTML += "<div class='tile bookmarks' onclick='window.janus.launchurl(\""+url+"\",0)' style='background: url(\""+thumb+"\") no-repeat center/cover;' ><div class='infoHolder'>"+strip(title)+"</div></div>";

    }

}




function refreshPartymode() {

    //Populate list with now available window.janus.bookmarks array.

    emptyElement(document.getElementById("partiesList"));

    for (var i=0;i<window.janus.partymodedata.length;i++)
    {

      var userId = window.janus.partymodedata[i].userId;

      var name= window.janus.partymodedata[i].name || extractDomain(window.janus.partymodedata[i].url) || "Untitled Webspace";

      var url = window.janus.partymodedata[i].url;


      document.getElementById("partiesList").innerHTML += "<div class='tile user' onclick='window.janus.launchurl(\""+url+"\",0)' ><b>"+strip(userId)+"</b> at "+strip(name)+"</div>";

    }

}



function refreshPopular() {
	
    //Populate list with now available window.janus.bookmarks array.
    var blacklist_urls = ['https://vesta.janusvr.com/popular/all/1', 'https://vesta.janusvr.com/recent/all/1', 'https://www.reddit.com/r/VRsites/', 'https://www.youtube.com', 'https://www.janusvr.com/newlobby/index.html', 'https://vesta.janusvr.com/sandbox', 'https://janusvr.com/help', 'https://vesta.janusvr.com/chat', 'https://janusvr.com/help/' ];
    emptyElement(document.getElementById("popularList"));

    for (var i=0;i<window.janus.populardata.length;i++)
    {

      var thumb= window.janus.populardata[i].thumbnail ;

      var title= window.janus.populardata[i].roomName || extractDomain(window.janus.populardata[i].roomUrl) || "Untitled Webspace";

      var url = window.janus.populardata[i].roomUrl;


      if (blacklist_urls.indexOf(url) === -1)
	  {
        document.getElementById("popularList").innerHTML += "<div class='tile popular' onclick='window.janus.launchurl(\""+url+"\",0)' style='background: url(\""+thumb+"\") no-repeat center/cover;' ><div class='infoHolder'>"+strip(title)+"</div></div>";
      }

    }

}
function refreshTrending() {

    //Populate list with now available window.janus.bookmarks array.
    var blacklist_urls = ['https://vesta.janusvr.com/popular/all/1', 'https://vesta.janusvr.com/recent/all/1', 'https://www.reddit.com/r/VRsites/', 'https://www.youtube.com', 'https://www.janusvr.com/newlobby/index.html', 'https://vesta.janusvr.com/sandbox', 'https://janusvr.com/help', 'https://vesta.janusvr.com/chat', 'https://janusvr.com/help/' ];
    emptyElement(document.getElementById("popularList"));

    for (var i=0;i<trendingdata.length;i++)
    {

      var thumb= trendingdata[i].thumbnail;

      var title= trendingdata[i].roomName || extractDomain(trendingdata[i].roomUrl) || "Untitled Webspace";

      var url = trendingdata[i].roomUrl;


      if (blacklist_urls.indexOf(url) === -1)
	  {
        document.getElementById("popularList").innerHTML += "<div class='tile popular' onclick='window.janus.launchurl(\""+url+"\",0)' style='background: url(\""+thumb+"\") no-repeat center/cover;' ><div class='infoHolder'>"+strip(title)+"</div></div>";
      }

    }

}

updatetrendingdata = function(params)
{
	fetch('https://vesta.janusvr.com/api/top_rooms?limit=36&orderBy=trending', { method:'get' })
	.then(function(response)
	{
		if (!response.ok)
		{
			throw Error(response.statusText);
		}
		return response.json();
	})
	.then(function(data)
	{
		trendingdata = data.data;
	})
	.catch(function() {
		console.log("error loading activity");
	});
	trendingTimeout = setTimeout(function(){ updatetrendingdata() }, 30000);
}