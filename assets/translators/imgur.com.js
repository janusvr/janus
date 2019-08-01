window.janus.createroom = function() 
{
	//room properties (entrance portal position, etc)
	window.janus.room = {pos:"0 0 -10",fwd:"0 0 1",visible:false,
			skybox_down_id:"sky_down",skybox_right_id:"sky_right",skybox_left_id:"sky_left",
			skybox_up_id:"sky_up",skybox_back_id:"sky_back",skybox_front_id:"sky_front",
			cubemap_radiance_id:"skybox_radiance",cubemap_irradiance_id:"skybox_irradiance"};

	var trans_path = "http://downloads.janusvr.com/translator/imgur/";
	var trans_path2 = "file:///home/james/Desktop/projects/2011/firebox/janusvr_www/downloads/translator/imgur/";

	//assets
	window.janus.createasset("object", {id:"Xart",src:trans_path+"imgur.dae.gz"});
	window.janus.createasset("object", {id:"XartG",src:trans_path+"imgurGlass.dae.gz"});
	window.janus.createasset("object", {id:"XartW",src:trans_path+"imgurwater.dae.gz"});
	window.janus.createasset("object", {id:"XartE",src:trans_path+"imguremit.dae.gz"});

	window.janus.createasset("object", {id:"Xartcol",src:trans_path+"imgur.obj.gz"});

	window.janus.createasset("object", {id:"artdecopiece1",src:trans_path+"spin1.dae.gz"});
	window.janus.createasset("object", {id:"artdecopiece2",src:trans_path+"spin2.dae.gz"});
	window.janus.createasset("object", {id:"artdecopiece3",src:trans_path+"spin3.dae.gz"});
	window.janus.createasset("object", {id:"artdecopiece4",src:trans_path+"spin4.dae.gz"});
	window.janus.createasset("object", {id:"artdecopiece5",src:trans_path+"spin5.dae.gz"});
	window.janus.createasset("object", {id:"artdecopiece6",src:trans_path+"spin6.dae.gz"});

	window.janus.createasset("image", {id:"sky_right",src:trans_path+"skyright.jpg"});
	window.janus.createasset("image", {id:"sky_front",src:trans_path+"skyfront.jpg"});
	window.janus.createasset("image", {id:"sky_up",src:trans_path+"skyup.jpg"});
	window.janus.createasset("image", {id:"sky_left",src:trans_path+"skyleft.jpg"});
	window.janus.createasset("image", {id:"sky_down",src:trans_path+"skydown.jpg"});
	window.janus.createasset("image", {id:"sky_back",src:trans_path+"skyback.jpg"});

	window.janus.createasset("image", {id:"skybox_radiance",src:trans_path+"ImgurSkyboxRadience.dds", tex_clamp:false, tex_linear:true});
	window.janus.createasset("image", {id:"skybox_irradiance",src:trans_path+"ImgurSkyboxIrRadience.dds", tex_clamp:false, tex_linear:true});

	//objects
	window.janus.createobject("object", {id:"Xart", collision_id:"Xartcol"});
	window.janus.createobject("object", {id:"XartE", lighting:false});
	window.janus.createobject("object", {id:"XartW"});
	window.janus.createobject("object", {id:"XartG"});

	window.janus.createobject("object", {id:"artdecopiece1", rotate_deg_per_sec:"10"});
	window.janus.createobject("object", {id:"artdecopiece2", rotate_deg_per_sec:"-10"});
	window.janus.createobject("object", {id:"artdecopiece3", rotate_deg_per_sec:"15"});
	window.janus.createobject("object", {id:"artdecopiece4", rotate_deg_per_sec:"-15"});
	window.janus.createobject("object", {id:"artdecopiece5", rotate_deg_per_sec:"20"});
	window.janus.createobject("object", {id:"artdecopiece6", rotate_deg_per_sec:"-20"});

	//50.23+ use the DOM to generate dynamic content
	//window.janus.createobject("text", {pos:"5 0.5 0", fwd:"1 0 0", scale:"5 5 5", innertext:"document.title: "+document.title});
	//window.janus.createobject("text", {pos:"5 1 0", fwd:"1 0 0", scale:"5 5 5", innertext:"document.images.length: "+document.images.length});
	
	var x =	document.getElementsByClassName("image-list-link");

	if (x.length > 0) { //imgur.com
		var urls = [];
		for (var i=0; i<x.length; i++) {		
			urls.push(x[i].href);
		}
	
		var num = Math.min(document.images.length, urls.length);
		for (var i=0; i<num; ++i) {
			window.janus.createasset("image", {src:document.images[i].src, id:"img"+i});	

			var imgdata = {thumb_id:"img"+i, pos:"0 1.5 3", fwd:"0 0 -1", scale:"3 3 0.1",url:urls[i],draw_glow:"false"};
			placeImage(imgdata, i, 21.0, num);
			window.janus.createobject("link", imgdata);			
		}
	}
	else { //imgur.com/gallery (or single images/gifs/vids)
		x = document.getElementsByClassName("post-image-placeholder");

		if (x.length == 0) {
			x = document.getElementsByTagName("img");
		}

		var urls = [];
		for (var i=0; i<x.length; i++) {		
			urls.push(x[i].src);
		}

		var num = urls.length;
		for (var i=0; i<num; ++i) {
			window.janus.createasset("image", {src:urls[i], id:"img"+i});	

			var imgdata = {id:"cube", collision_id:"cube", image_id:"img"+i, pos:"0 1.5 3", fwd:"0 0 -1", scale:"3 3 0.1", lighting:"false"};
			placeImage(imgdata, i, 21.0, num);
			window.janus.createobject("object", imgdata);			
		}
	}

	//show comments
	//var list = document.getElementsByTagName("span");
	//for (var i=0; i<list.length; ++i) {
	//	window.janus.createobject("text", {id:"text"+comments, pos:"5 1.8 "+comments, fwd:"1 0 0",innertext:"Comment:" + list[i].innerHTML,scale:"0.9 0.9 0.9"});			
	//}
}

var placeImage=function(imgdata, index, rad, num)
{
	var max_per_row = 20;
	if (num > 1) {
		var angle = (index % max_per_row) / max_per_row * 3.14159; 
		imgdata.pos = "" + (Math.cos(angle)*rad).toString() + " " + (1.5+(Math.floor(index/max_per_row))*3.25).toString() + " " + (Math.sin(angle) * rad).toString();
		imgdata.fwd = "" + (-Math.cos(angle)).toString() + " 0 " + (-Math.sin(angle)).toString();
		//imgdata.scale = "1.5 1.5 1.5";
	}
}
