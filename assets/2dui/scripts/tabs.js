  
  
  function tabSelect(containertoload,tabbuttonthatwaspushed) {
  
  
	var rhandles = document.getElementsByClassName("tabcontainer"); //return all containers
	
	var i;
	for (i=0;i<rhandles.length;i++)
	{
	rhandles[i].style.display = "none"; //hide all containers
	}
  
 	var tabzz = document.getElementsByClassName("tabbutton"); //return all tabs
	
	var i;
	for (i=0;i<tabzz.length;i++)
	{
	tabzz[i].style.backgroundColor = ""; //set the background colour for the selected tab
	tabzz[i].style.color = ""; //set the background colour for the selected tab
	
	} 
  
  
  
  
  var exceptme = document.getElementById(containertoload).style.display = "block"; //except for the currently active container
  var exceptme = document.getElementById(tabbuttonthatwaspushed).style.backgroundColor = "#E4E4E4"; //set the background colour for the selected tab
  var exceptme = document.getElementById(tabbuttonthatwaspushed).style.color = "#242A2D"; //set the colour for the selected tab
	document.getElementById("windowarea").scrollTop = 0; //scroll back to top of window area div

  }