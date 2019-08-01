window.janus.createroom = function() 
{
	//room properties (entrance portal position, etc)
	window.janus.room = {pos:"20 0 2",fwd:"-1 0 0",visible:false};

	var trans_path = "http://downloads.janusvr.com/translator/";

	//assets
	window.janus.createasset("websurface", {src:window.janus.url, id:"web1", width:1280, height:768});
	window.janus.createasset("object", {id:"class",src:trans_path+"reddit/redditroomchairs.dae"});
	window.janus.createasset("object", {id:"class2",src:trans_path+"reddit/redditroomFinal.dae"});
	window.janus.createasset("object", {id:"class3",src:trans_path+"reddit/redditroomcircle1.dae"});
	window.janus.createasset("object", {id:"class4",src:trans_path+"reddit/redditroomcircle2.dae"});
	window.janus.createasset("object", {id:"class5",src:trans_path+"reddit/redditroomlight.dae"});
	window.janus.createasset("object", {id:"screen1",src:trans_path+"reddit/screensingle.obj"});
	window.janus.createasset("object", {id:"col",src:trans_path+"reddit/redditcollisionmesh.obj"});
	window.janus.createasset("object", {id:"colchair",src:trans_path+"reddit/redditcollisionmeshchair.obj"});

	//objects
	window.janus.createobject("object", {id:"class", lighting:false, collision_id:"colchair", pos:"0 0 0", dir:"0 0 1", collision_radius:2.0});
	window.janus.createobject("object", {id:"class2", lighting:false, collision_id:"col", pos:"0 0 0", fwd:"-1 0 0"});
	window.janus.createobject("object", {id:"class3", lighting:false, pos:"0 0 0", fwd:"-1 0 0"});
	window.janus.createobject("object", {id:"class4", lighting:false, pos:"0 0 0", fwd:"-1 0 0"});
    	window.janus.createobject("object", {id:"class5", lighting:false, collision_id:"class5", pos:"0 0 0", fwd:"-1 0 0"});
    	window.janus.createobject("object", {id:"screen1", lighting:false, websurface_id:"web1", pos:"-12.5 0.3 -12.5", fwd:"-1 0 0"});
    	window.janus.createobject("object", {id:"screen1", lighting:false, websurface_id:"web1", pos:"-12.5 0.3 12.5", fwd:"0 0 1"});
    	window.janus.createobject("object", {id:"screen1", lighting:false, websurface_id:"web1", pos:"12.5 0.3 -12.5", fwd:"0 0 -1"});
    	window.janus.createobject("object", {id:"screen1", lighting:false, websurface_id:"web1", pos:"12.5 0.3 12.5", fwd:"1 0 0"});
       	window.janus.createobject("object", {id:"class5", lighting:false, collision_id:"class5", pos:"0 0 0", fwd:"1 0 0"});
       	window.janus.createobject("object", {id:"class5", lighting:false, collision_id:"class5", pos:"0 0 0", fwd:"0 0 1"});
       	window.janus.createobject("object", {id:"class5", lighting:false, collision_id:"class5", pos:"0 0 0", fwd:"0 0 -1"});
       	window.janus.createobject("object", {id:"class", lighting:false, collision_id:"colchair", pos:"0 0 0", fwd:"0 0 -1"});
       	window.janus.createobject("object", {id:"class", lighting:false, collision_id:"colchair", pos:"0 0 0", fwd:"1 0 0"});
       	window.janus.createobject("object", {id:"class", lighting:false, collision_id:"colchair", pos:"0 0 0", fwd:"-1 0 0"});

	//50.23+ use the DOM to generate dynamic content
	window.janus.createobject("text", {pos:"5 0.5 0", fwd:"1 0 0", scale:"5 5 5", innertext:"document.title: "+document.title});
	window.janus.createobject("text", {pos:"5 1 0", fwd:"1 0 0", scale:"5 5 5", innertext:"document.images.length: "+document.images.length});
	for (var i=0; i<document.images.length; ++i) {
		window.janus.createobject("text", {id:"img"+i, pos:"5 1.8 "+i, fwd:"1 0 0",innertext:document.images[i].src,scale:"0.9 0.9 0.9"});
		window.janus.createasset("image", {src:document.images[i].src, id:"img"+i});
		window.janus.createobject("image", {id:"img"+i, pos:"5 1.5 "+i, fwd:"1 0 0", scale:"0.5 0.5 0.5"});
	}
}
