var showinstruct = false;

room.update = function(delta_time) 
{
	if (player.hmd_type == "rift" || player.hmd_type == "vive") {
		left_update(player.hmd_type);
		right_update(player.hmd_type);
	}
}

room.onMouseDown = function()
{
	showinstruct = true;
}

room.onMouseUp = function()
{
	showinstruct = false;
}

function left_update(hmd_type) 
{
	var n = "__menu_" + hmd_type + "0";
  	if (showinstruct && player.hand0_active) {
      		room.objects[n].visible = true;
      		room.objects[n].pos  = player.hand0_pos;
      		room.objects[n].xdir = player.hand0_xdir;
      		room.objects[n].ydir = player.hand0_ydir;
      		room.objects[n].zdir = player.hand0_zdir;
  	} 
  	else {
      		room.objects[n].visible = false;
  	}
}

function right_update(hmd_type) 
{
	var n = "__menu_" + hmd_type + "1";
  	if (showinstruct && player.hand1_active) {
    		room.objects[n].visible = true;
    		room.objects[n].pos  = player.hand1_pos;
    		room.objects[n].xdir = player.hand1_xdir;
    		room.objects[n].ydir = player.hand1_ydir;
    		room.objects[n].zdir = player.hand1_zdir;
  	} 
  	else {
    		room.objects[n].visible = false;
  	}
}
