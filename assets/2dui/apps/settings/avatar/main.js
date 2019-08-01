	var populated = false;
	var settings = null;
	var roomavatars = [];
	var renaming_input_div = "";
	var renaming_input = null;
	var renaming_avatar = 0;
	var renaming_div = null;
	var renaming_text = "";
	function normalize2D(n1, n2) {
		var norm = Math.sqrt(n1 * n1 + n2 * n2);
		var point2 = {x:n1, y:n2};
		if (norm != 0) { // as3 return 0,0 for a point of zero length
			point2.x = 1 * n1 / norm;
			point2.y = 1 * n2 / norm;
		}
		return point2;
	}
	var last_random_color = "";
	randomColor()
	
	function randomColor()
	{
		last_random_color = '"#'+Math.floor(Math.random()*16777215).toString(16) + '"';
		return last_random_color;
	}
	function lastRandomColor()
	{
		return last_random_color;
	}
	function loadsettings()
	{
		settings = JSON.parse(window.janus.getsetting('avatarData'));
		if (settings == null)
		{
			settings = [];
			//settings.push({ name: "Unnamed Avatar", string: escape(janus.getavatar()), });
			
			settings.push({ name: "Beta", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" src=\"http://www.janusvr.com/avatars/animated/Beta/Beta.fbx.gz\" /><AssetObject id=\"run\" src=\"http://www.janusvr.com/avatars/animated/Beta/run.fbx.gz\" /><AssetObject id=\"walk_back\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk_back.fbx.gz\" /><AssetObject id=\"walk_right\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk_right.fbx.gz\" /><AssetObject id=\"walk\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk.fbx.gz\" /><AssetObject id=\"walk_left\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk_left.fbx.gz\" /><AssetObject id=\"jump\" src=\"http://www.janusvr.com/avatars/animated/Beta/jump.fbx.gz\" /><AssetObject id=\"fly\" src=\"http://www.janusvr.com/avatars/animated/Beta/fly.fbx.gz\" /><AssetObject id=\"type\" src=\"http://www.janusvr.com/avatars/animated/Beta/type.fbx.gz\" /><AssetObject id=\"idle\" src=\"http://www.janusvr.com/avatars/animated/Beta/idle.fbx.gz\" /><AssetObject id=\"speak\" src=\"http://www.janusvr.com/avatars/animated/Beta/speak.fbx.gz\" /><AssetObject id=\"portal\" src=\"http://www.janusvr.com/avatars/animated/Beta/portal.fbx.gz\" /></Assets><Room><Ghost id=\"Beta\" scale=\"0.0095 0.0095 0.0095\" col=\"randomColor()\" body_id=\"body\" userid_pos=\"0 0.5 0\" /></Room></FireBoxRoom>")});
			settings.push({ name: "Y_Bot", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/body.fbx.gz\" /><AssetObject id=\"run\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/run.fbx.gz\" /><AssetObject id=\"walk_back\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/walk_back.fbx.gz\" /><AssetObject id=\"walk_right\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/walk_right.fbx.gz\" /><AssetObject id=\"walk\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/walk.fbx.gz\" /><AssetObject id=\"walk_left\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/walk_left.fbx.gz\" /><AssetObject id=\"jump\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/jump.fbx.gz\" /><AssetObject id=\"fly\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/fly.fbx.gz\" /><AssetObject id=\"type\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/type.fbx.gz\" /><AssetObject id=\"idle\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/idle.fbx.gz\" /><AssetObject id=\"speak\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/speak.fbx.gz\" /><AssetObject id=\"portal\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/portal.fbx.gz\" /></Assets><Room><Ghost id=\"Y_Bot\" scale=\"0.0095 0.0095 0.0095\" col=\"#ffffff\" body_id=\"body\" userid_pos=\"0 0.5 0\" /></Room></FireBoxRoom>")});

			//settings.push({ name: "Janus Knight", string: escape("<FireBoxRoom><Assets><AssetObject id=\"walk_right\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk_right.fbx.gz\" /><AssetObject id=\"speak\" src=\"http://www.janusvr.com/avatars/animated/Beta/speak.fbx.gz\" /><AssetObject id=\"walk_left\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk_left.fbx.gz\" /><AssetObject id=\"idle\" src=\"http://www.janusvr.com/avatars/animated/Beta/idle.fbx.gz\" /><AssetObject id=\"walk_back\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk_back.fbx.gz\" /><AssetObject id=\"body\" src=\"http://janusvr.com/avatars/animated/ks/janus_knight.fbx.gz\" /><AssetObject id=\"portal\" src=\"http://www.janusvr.com/avatars/animated/Beta/portal.fbx.gz\" /><AssetObject id=\"jump\" src=\"http://www.janusvr.com/avatars/animated/Beta/jump.fbx.gz\" /><AssetObject id=\"walk\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk.fbx.gz\" /><AssetObject id=\"type\" src=\"http://www.janusvr.com/avatars/animated/Beta/type.fbx.gz\" /><AssetObject id=\"run\" src=\"http://www.janusvr.com/avatars/animated/Beta/run.fbx.gz\" /><AssetObject id=\"fly\" src=\"http://www.janusvr.com/avatars/animated/Beta/fly.fbx.gz\" /></Assets><Room><Ghost id=\"JANUS_KNIGHT\" scale=\"0.0095 0.0095 0.0095\" col=\"randomColor()\" body_id=\"body\" userid_pos=\"0 0.5 0\" /></Room></FireBoxRoom>")});
			//settings.push({ name: "Inconspicuous Man", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" src=\"http://www.janusvr.com/avatars/animated/Beta/Beta.fbx.gz\" /><AssetObject id=\"ball_cap\" src=\"http://janusvr.com/avatars/animated/ks/ball_cap.fbx.gz\" /><AssetObject id=\"sunglasses\" src=\"http://janusvr.com/avatars/animated/ks/sunglasses.fbx.gz\" /><AssetObject id=\"stache\" src=\"http://janusvr.com/avatars/animated/ks/stache.fbx.gz\" /><AssetObject id=\"run\" src=\"http://www.janusvr.com/avatars/animated/Beta/run.fbx.gz\" /><AssetObject id=\"speak\" src=\"http://www.janusvr.com/avatars/animated/Beta/speak.fbx.gz\" /><AssetObject id=\"portal\" src=\"http://www.janusvr.com/avatars/animated/Beta/portal.fbx.gz\" /><AssetObject id=\"walk_right\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk_right.fbx.gz\" /><AssetObject id=\"idle\" src=\"http://www.janusvr.com/avatars/animated/Beta/idle.fbx.gz\" /><AssetObject id=\"jump\" src=\"http://www.janusvr.com/avatars/animated/Beta/jump.fbx.gz\" /><AssetObject id=\"walk_back\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk_back.fbx.gz\" /><AssetObject id=\"fly\" src=\"http://www.janusvr.com/avatars/animated/Beta/fly.fbx.gz\" /><AssetObject id=\"walk\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk.fbx.gz\" /><AssetObject id=\"type\" src=\"http://www.janusvr.com/avatars/animated/Beta/type.fbx.gz\" /><AssetObject id=\"walk_left\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk_left.fbx.gz\" /></Assets><Room><Ghost id=\"Inconspicuous_Man\" scale=\"0.0095 0.0095 0.0095\" col=\"randomColor()\" body_id=\"body\" userid_pos=\"0 0.5 0\"><Object id=\"sunglasses\"  bone_id=\"Head\" col=\"#000000\" /><Object id=\"stache\" pos=\"0 3 15\" xdir=\"0 -1 0\" ydir=\"0 0 1\" zdir=\"-1 0 0\" scale=\"0.5 0.5 0.5\" col=\"#000000\" bone_id=\"Head\" /><Object id=\"ball_cap\" bone_id=\"Head\" col=\"randomColor()\" /></Ghost></Room></FireBoxRoom>")});

			settings.push({ name: "Claire", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" src=\"https://vesta.janusvr.com/avatars/animated/claire/body.fbx.gz\" /><AssetObject id=\"run\" src=\"https://vesta.janusvr.com/avatars/animated/claire/run.fbx.gz\" /><AssetObject id=\"walk_back\" src=\"https://vesta.janusvr.com/avatars/animated/claire/walk_back.fbx.gz\" /><AssetObject id=\"walk_right\" src=\"https://vesta.janusvr.com/avatars/animated/claire/walk_right.fbx.gz\" /><AssetObject id=\"walk\" src=\"https://vesta.janusvr.com/avatars/animated/claire/walk.fbx.gz\" /><AssetObject id=\"walk_left\" src=\"https://vesta.janusvr.com/avatars/animated/claire/walk_left.fbx.gz\" /><AssetObject id=\"jump\" src=\"https://vesta.janusvr.com/avatars/animated/claire/jump.fbx.gz\" /><AssetObject id=\"fly\" src=\"https://vesta.janusvr.com/avatars/animated/claire/fly.fbx.gz\" /><AssetObject id=\"type\" src=\"https://vesta.janusvr.com/avatars/animated/claire/type.fbx.gz\" /><AssetObject id=\"idle\" src=\"https://vesta.janusvr.com/avatars/animated/claire/idle.fbx.gz\" /><AssetObject id=\"speak\" src=\"https://vesta.janusvr.com/avatars/animated/claire/speak.fbx.gz\" /><AssetObject id=\"portal\" src=\"https://vesta.janusvr.com/avatars/animated/claire/portal.fbx.gz\" /></Assets><Room><Ghost id=\"Claire\" scale=\"0.0095 0.0095 0.0095\" col=\"#ffffff\" body_id=\"body\" userid_pos=\"0 0.5 0\" /></Room></FireBoxRoom>")});
			settings.push({ name: "Ty", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" src=\"https://vesta.janusvr.com/avatars/animated/ty/body.fbx.gz\" /><AssetObject id=\"jump\" src=\"http://www.janusvr.com/avatars/animated/Beta/jump.fbx.gz\" /><AssetObject id=\"speak\" src=\"http://www.janusvr.com/avatars/animated/Beta/speak.fbx.gz\" /><AssetObject id=\"idle\" src=\"http://www.janusvr.com/avatars/animated/Beta/idle.fbx.gz\" /><AssetObject id=\"walk\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk.fbx.gz\" /><AssetObject id=\"fly\" src=\"http://www.janusvr.com/avatars/animated/Beta/fly.fbx.gz\" /><AssetObject id=\"run\" src=\"http://www.janusvr.com/avatars/animated/Beta/run.fbx.gz\" /><AssetObject id=\"walk_right\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk_right.fbx.gz\" /><AssetObject id=\"portal\" src=\"http://www.janusvr.com/avatars/animated/Beta/portal.fbx.gz\" /><AssetObject id=\"walk_left\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk_left.fbx.gz\" /><AssetObject id=\"type\" src=\"http://www.janusvr.com/avatars/animated/Beta/type.fbx.gz\" /><AssetObject id=\"walk_back\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk_back.fbx.gz\" /></Assets><Room><Ghost id=\"Ty\" scale=\"0.0095 0.0095 0.0095\" col=\"#ffffff\" body_id=\"body\" userid_pos=\"0 0.5 0\" /></Room></FireBoxRoom>")});
			settings.push({ name: "Drunk Guy", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/body.fbx.gz\" /><AssetObject id=\"run\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/run.fbx.gz\" /><AssetObject id=\"walk_back\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/walk_back.fbx.gz\" /><AssetObject id=\"walk_right\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/walk_right.fbx.gz\" /><AssetObject id=\"walk\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/walk.fbx.gz\" /><AssetObject id=\"walk_left\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/walk_left.fbx.gz\" /><AssetObject id=\"jump\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/jump.fbx.gz\" /><AssetObject id=\"fly\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/fly.fbx.gz\" /><AssetObject id=\"type\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/type.fbx.gz\" /><AssetObject id=\"idle\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/idle.fbx.gz\" /><AssetObject id=\"speak\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/speak.fbx.gz\" /><AssetObject id=\"portal\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/portal.fbx.gz\" /></Assets><Room><Ghost id=\"DrunkGuy\" scale=\"0.0095 0.0095 0.0095\" col=\"#ffffff\" body_id=\"body\" userid_pos=\"0 0.5 0\" /></Room></FireBoxRoom>")});
			settings.push({ name: "Elvis", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" src=\"https://vesta.janusvr.com/avatars/animated/elvis/body.fbx.gz\" /><AssetObject id=\"run\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/run.fbx.gz\" /><AssetObject id=\"walk_back\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/walk_back.fbx.gz\" /><AssetObject id=\"walk_right\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/walk_right.fbx.gz\" /><AssetObject id=\"walk\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/walk.fbx.gz\" /><AssetObject id=\"walk_left\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/walk_left.fbx.gz\" /><AssetObject id=\"jump\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/jump.fbx.gz\" /><AssetObject id=\"fly\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/fly.fbx.gz\" /><AssetObject id=\"type\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/type.fbx.gz\" /><AssetObject id=\"idle\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/idle.fbx.gz\" /><AssetObject id=\"speak\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/speak.fbx.gz\" /><AssetObject id=\"portal\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/portal.fbx.gz\" /></Assets><Room><Ghost id=\"Elvis\" scale=\"0.0095 0.0095 0.0095\" col=\"#ffffff\" body_id=\"body\" userid_pos=\"0 0.5 0\" /></Room></FireBoxRoom>")});
			settings.push({ name: "Boy", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" src=\"https://vesta.janusvr.com/avatars/animated/boy/body.fbx.gz\" /><AssetObject id=\"run\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/run.fbx.gz\" /><AssetObject id=\"walk_back\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/walk_back.fbx.gz\" /><AssetObject id=\"walk_right\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/walk_right.fbx.gz\" /><AssetObject id=\"walk\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/walk.fbx.gz\" /><AssetObject id=\"walk_left\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/walk_left.fbx.gz\" /><AssetObject id=\"jump\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/jump.fbx.gz\" /><AssetObject id=\"fly\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/fly.fbx.gz\" /><AssetObject id=\"type\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/type.fbx.gz\" /><AssetObject id=\"idle\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/idle.fbx.gz\" /><AssetObject id=\"speak\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/speak.fbx.gz\" /><AssetObject id=\"portal\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/portal.fbx.gz\" /></Assets><Room><Ghost id=\"Boy\" scale=\"0.0095 0.0095 0.0095\" col=\"#ffffff\" body_id=\"body\" userid_pos=\"0 0.5 0\" /></Room></FireBoxRoom>")});
			//settings.push({ name: "Castle Guard", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" src=\"https://vesta.janusvr.com/avatars/animated/castle_guard/body.fbx.gz\" /><AssetObject id=\"run\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/run.fbx.gz\" /><AssetObject id=\"walk_back\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/walk_back.fbx.gz\" /><AssetObject id=\"walk_right\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/walk_right.fbx.gz\" /><AssetObject id=\"walk\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/walk.fbx.gz\" /><AssetObject id=\"walk_left\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/walk_left.fbx.gz\" /><AssetObject id=\"jump\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/jump.fbx.gz\" /><AssetObject id=\"fly\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/fly.fbx.gz\" /><AssetObject id=\"type\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/type.fbx.gz\" /><AssetObject id=\"idle\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/idle.fbx.gz\" /><AssetObject id=\"speak\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/speak.fbx.gz\" /><AssetObject id=\"portal\" src=\"https://vesta.janusvr.com/avatars/animated/drunkguy/portal.fbx.gz\" /></Assets><Room><Ghost id=\"CastleGuard\" scale=\"0.0095 0.0095 0.0095\" col=\"#ffffff\" body_id=\"body\" userid_pos=\"0 0.5 0\" /></Room></FireBoxRoom>")});
			//settings.push({ name: "Peasant Girl", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" src=\"https://vesta.janusvr.com/avatars/animated/peasant_girl/body.fbx.gz\" /><AssetObject id=\"run\" src=\"https://vesta.janusvr.com/avatars/animated/claire/run.fbx.gz\" /><AssetObject id=\"walk_back\" src=\"https://vesta.janusvr.com/avatars/animated/claire/walk_back.fbx.gz\" /><AssetObject id=\"walk_right\" src=\"https://vesta.janusvr.com/avatars/animated/claire/walk_right.fbx.gz\" /><AssetObject id=\"walk\" src=\"https://vesta.janusvr.com/avatars/animated/claire/walk.fbx.gz\" /><AssetObject id=\"walk_left\" src=\"https://vesta.janusvr.com/avatars/animated/claire/walk_left.fbx.gz\" /><AssetObject id=\"jump\" src=\"https://vesta.janusvr.com/avatars/animated/claire/jump.fbx.gz\" /><AssetObject id=\"fly\" src=\"https://vesta.janusvr.com/avatars/animated/claire/fly.fbx.gz\" /><AssetObject id=\"type\" src=\"https://vesta.janusvr.com/avatars/animated/claire/type.fbx.gz\" /><AssetObject id=\"idle\" src=\"https://vesta.janusvr.com/avatars/animated/claire/idle.fbx.gz\" /><AssetObject id=\"speak\" src=\"https://vesta.janusvr.com/avatars/animated/claire/speak.fbx.gz\" /><AssetObject id=\"portal\" src=\"https://vesta.janusvr.com/avatars/animated/claire/portal.fbx.gz\" /></Assets><Room><Ghost id=\"PeasantGirl\" scale=\"0.0095 0.0095 0.0095\" col=\"#ffffff\" body_id=\"body\" userid_pos=\"0 0.5 0\" /></Room></FireBoxRoom>")});
			//settings.push({ name: "Peasant Man", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" src=\"https://vesta.janusvr.com/avatars/animated/peasant_man/body.fbx.gz\" /><AssetObject id=\"run\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/run.fbx.gz\" /><AssetObject id=\"walk_back\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/walk_back.fbx.gz\" /><AssetObject id=\"walk_right\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/walk_right.fbx.gz\" /><AssetObject id=\"walk\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/walk.fbx.gz\" /><AssetObject id=\"walk_left\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/walk_left.fbx.gz\" /><AssetObject id=\"jump\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/jump.fbx.gz\" /><AssetObject id=\"fly\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/fly.fbx.gz\" /><AssetObject id=\"type\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/type.fbx.gz\" /><AssetObject id=\"idle\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/idle.fbx.gz\" /><AssetObject id=\"speak\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/speak.fbx.gz\" /><AssetObject id=\"portal\" src=\"https://vesta.janusvr.com/avatars/animated/y_bot/portal.fbx.gz\" /></Assets><Room><Ghost id=\"PeasantMan\" scale=\"0.0095 0.0095 0.0095\" col=\"#ffffff\" body_id=\"body\" userid_pos=\"0 0.5 0\" /></Room></FireBoxRoom>")});
			settings.push({ name: "Sporty Granny", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" src=\"https://vesta.janusvr.com/avatars/animated/granny/body.fbx.gz\" /><AssetObject id=\"run\" src=\"https://vesta.janusvr.com/avatars/animated/claire/run.fbx.gz\" /><AssetObject id=\"walk_back\" src=\"https://vesta.janusvr.com/avatars/animated/claire/walk_back.fbx.gz\" /><AssetObject id=\"walk_right\" src=\"https://vesta.janusvr.com/avatars/animated/claire/walk_right.fbx.gz\" /><AssetObject id=\"walk\" src=\"https://vesta.janusvr.com/avatars/animated/granny/walk.fbx.gz\" /><AssetObject id=\"walk_left\" src=\"https://vesta.janusvr.com/avatars/animated/claire/walk_left.fbx.gz\" /><AssetObject id=\"jump\" src=\"https://vesta.janusvr.com/avatars/animated/claire/jump.fbx.gz\" /><AssetObject id=\"fly\" src=\"https://vesta.janusvr.com/avatars/animated/claire/fly.fbx.gz\" /><AssetObject id=\"type\" src=\"https://vesta.janusvr.com/avatars/animated/claire/type.fbx.gz\" /><AssetObject id=\"idle\" src=\"https://vesta.janusvr.com/avatars/animated/granny/idle.fbx.gz\" /><AssetObject id=\"speak\" src=\"https://vesta.janusvr.com/avatars/animated/claire/speak.fbx.gz\" /><AssetObject id=\"portal\" src=\"https://vesta.janusvr.com/avatars/animated/claire/portal.fbx.gz\" /></Assets><Room><Ghost id=\"SportyGranny\" scale=\"0.0095 0.0095 0.0095\" col=\"#ffffff\" body_id=\"body\" userid_pos=\"0 0.5 0\" /></Room></FireBoxRoom>")});

			settings.push({ name: "Handyman", string: escape("<FireBoxRoom><Assets><AssetObject id=\"bracelet\" src=\"http://janusvr.com/avatars/animated/ks/rbracelet.fbx.gz\" /><AssetObject id=\"hands\" src=\"http://janusvr.com/avatars/animated/ks/jvr_hands.fbx.gz\" /><AssetObject id=\"run\" src=\"http://www.janusvr.com/avatars/animated/Beta/run.fbx.gz\" /><AssetObject id=\"speak\" src=\"http://www.janusvr.com/avatars/animated/Beta/speak.fbx.gz\" /><AssetObject id=\"portal\" src=\"http://www.janusvr.com/avatars/animated/Beta/portal.fbx.gz\" /><AssetObject id=\"walk_right\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk_right.fbx.gz\" /><AssetObject id=\"idle\" src=\"http://www.janusvr.com/avatars/animated/Beta/idle.fbx.gz\" /><AssetObject id=\"jump\" src=\"http://www.janusvr.com/avatars/animated/Beta/jump.fbx.gz\" /><AssetObject id=\"walk_back\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk_back.fbx.gz\" /><AssetObject id=\"fly\" src=\"http://www.janusvr.com/avatars/animated/Beta/fly.fbx.gz\" /><AssetObject id=\"walk\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk.fbx.gz\" /><AssetObject id=\"type\" src=\"http://www.janusvr.com/avatars/animated/Beta/type.fbx.gz\" /><AssetObject id=\"walk_left\" src=\"http://www.janusvr.com/avatars/animated/Beta/walk_left.fbx.gz\" /></Assets><Room><Ghost id=\"Handyman\" scale=\"0.0095 0.0095 0.0095\" col=\"randomColor()\" head_pos=\"0 0 0\" body_id=\"hands\" userid_pos=\"0 0.5 0\"><Object id=\"bracelet\" col=\"lastRandomColor()\" bone_id=\"RightForeArm\" /><Object id=\"bracelet\" pos=\"41.713 0 0\" col=\"lastRandomColor()\" bone_id=\"LeftForeArm\" /></Ghost></Room></FireBoxRoom>")});
			
			settings.push({ name:"ASCII Business Suit Man", string: escape("<FireBoxRoom><Assets><AssetObject id=\"ghost_business_obj\" mtl=\"https://vesta.janusvr.com/avatars/static/flat/BusinessGuy.mtl\" src=\"https://vesta.janusvr.com/avatars/static/flat/CardboardGuy.obj.gz\" /></Assets><Room><Ghost id=\"Business_Suit_Man\" col=\"randomColor()\" scale=\"1 1 1\" lighting=\"false\" head_pos=\"0 1.2 0\" body_id=\"ghost_business_obj\" cull_face=\"none\" /></Room></FireBoxRoom>")});
			settings.push({ name:"ASCII Woman in a Dress", string: escape("<FireBoxRoom><Assets><AssetObject id=\"ghost_reddress_obj\" mtl=\"https://vesta.janusvr.com/avatars/static/flat/DressLady.mtl\" src=\"https://vesta.janusvr.com/avatars/static/flat/CardboardGuy.obj.gz\" /></Assets><Room><Ghost id=\"Business_Suit_Man\" col=\"randomColor()\" scale=\"1 1 1\" lighting=\"false\" head_pos=\"0 1.2 0\" body_id=\"ghost_reddress_obj\" cull_face=\"none\" /></Room></FireBoxRoom>")});

			settings.push({ name: "kool Sanic Going Fast", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" mtl=\"https://kool.website/avatars/1/23Xv6CK.mtl\" src=\"https://kool.website/avatars/1/23Xv6CK.obj\" /><AssetObject id=\"head\" mtl=\"\" src=\"\" /></Assets><Room><Ghost id=\"kool\" scale=\"1 1 1\" lighting=\"false\" body_id=\"body\" anim_id=\"type\" userid_pos=\"0 0.5 0\" cull_face=\"none\" /></Room></FireBoxRoom>")});
			settings.push({ name: "kool No Fun Allowed Bot", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" mtl=\"https://kool.website/avatars/1/F9Twb88.mtl\" src=\"https://kool.website/avatars/1/F9Twb88.obj\" /><AssetObject id=\"head\" mtl=\"\" src=\"\" /></Assets><Room><Ghost id=\"kool\" scale=\"1 1 1\" lighting=\"false\" body_id=\"body\" anim_id=\"type\" userid_pos=\"0 0.5 0\" cull_face=\"none\" /></Room></FireBoxRoom>")});
			settings.push({ name: "kool Phallic Llama", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" mtl=\"https://kool.website/avatars/1/GXiRI8H.mtl\" src=\"https://kool.website/avatars/1/GXiRI8H.obj\" /><AssetObject id=\"head\" mtl=\"\" src=\"\" /></Assets><Room><Ghost id=\"kool\" scale=\"1 1 1\" lighting=\"false\" body_id=\"body\" anim_id=\"type\" userid_pos=\"0 0.5 0\" cull_face=\"none\" /></Room></FireBoxRoom>")});
			settings.push({ name: "kool Unsightly Frog", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" mtl=\"https://kool.website/avatars/1/iDlNxUL.mtl\" src=\"https://kool.website/avatars/1/iDlNxUL.obj\" /><AssetObject id=\"head\" mtl=\"\" src=\"\" /></Assets><Room><Ghost id=\"kool\" scale=\"1 1 1\" lighting=\"false\" body_id=\"body\" anim_id=\"type\" userid_pos=\"0 0.5 0\" cull_face=\"none\" /></Room></FireBoxRoom>")});
			settings.push({ name: "kool Spooky Skeleton", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" mtl=\"https://kool.website/avatars/1/jasNKEA.mtl\" src=\"https://kool.website/avatars/1/jasNKEA.obj\" /><AssetObject id=\"head\" mtl=\"\" src=\"\" /></Assets><Room><Ghost id=\"kool\" scale=\"1 1 1\" lighting=\"false\" body_id=\"body\" anim_id=\"type\" userid_pos=\"0 0.5 0\" cull_face=\"none\" /></Room></FireBoxRoom>")});
			settings.push({ name: "kool Jazz Player", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" mtl=\"https://kool.website/avatars/1/Raw3by7.mtl\" src=\"https://kool.website/avatars/1/Raw3by7.obj\" /><AssetObject id=\"head\" mtl=\"\" src=\"\" /></Assets><Room><Ghost id=\"kool\" scale=\"1 1 1\" lighting=\"false\" body_id=\"body\" anim_id=\"type\" userid_pos=\"0 0.5 0\" cull_face=\"none\" /></Room></FireBoxRoom>")});
			settings.push({ name: "kool Raptor", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" mtl=\"https://kool.website/avatars/1/p3sb6rZ.mtl\" src=\"https://kool.website/avatars/1/p3sb6rZ.obj\" /><AssetObject id=\"head\" mtl=\"\" src=\"\" /></Assets><Room><Ghost id=\"kool\" scale=\"1 1 1\" lighting=\"false\" body_id=\"body\" anim_id=\"type\" userid_pos=\"0 0.5 0\" cull_face=\"none\" /></Room></FireBoxRoom>")});
			settings.push({ name: "kool T-Rex", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" mtl=\"https://kool.website/avatars/1/PlkuvpD.mtl\" src=\"https://kool.website/avatars/1/PlkuvpD.obj\" /><AssetObject id=\"head\" mtl=\"\" src=\"\" /></Assets><Room><Ghost id=\"kool\" scale=\"1 1 1\" lighting=\"false\" body_id=\"body\" anim_id=\"type\" userid_pos=\"0 0.5 0\" cull_face=\"none\" /></Room></FireBoxRoom>")});
			settings.push({ name: "kool Dat Boi", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" mtl=\"https://kool.website/avatars/1/Py8aLmy.mtl\" src=\"https://kool.website/avatars/1/Py8aLmy.obj\" /><AssetObject id=\"head\" mtl=\"\" src=\"\" /></Assets><Room><Ghost id=\"kool\" scale=\"1 1 1\" lighting=\"false\" body_id=\"body\" anim_id=\"type\" userid_pos=\"0 0.5 0\" cull_face=\"none\" /></Room></FireBoxRoom>")});
			settings.push({ name: "kool Meme Dog", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" mtl=\"https://kool.website/avatars/1/nGiodN4.mtl\" src=\"https://kool.website/avatars/1/nGiodN4.obj\" /><AssetObject id=\"head\" mtl=\"\" src=\"\" /></Assets><Room><Ghost id=\"kool\" scale=\"1 1 1\" lighting=\"false\" body_id=\"body\" anim_id=\"type\" userid_pos=\"0 0.5 0\" cull_face=\"none\" /></Room></FireBoxRoom>")});
			settings.push({ name: "kool Cat with tiny body", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" mtl=\"https://kool.website/avatars/1/qV9W0cu.mtl\" src=\"https://kool.website/avatars/1/qV9W0cu.obj\" /><AssetObject id=\"head\" mtl=\"\" src=\"\" /></Assets><Room><Ghost id=\"kool\" scale=\"1 1 1\" lighting=\"false\" body_id=\"body\" anim_id=\"type\" userid_pos=\"0 0.5 0\" cull_face=\"none\" /></Room></FireBoxRoom>")});
			settings.push({ name: "kool Dancing Cat 1", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" mtl=\"https://kool.website/avatars/1/cdnmFjm.mtl\" src=\"https://kool.website/avatars/1/cdnmFjm.obj\" /><AssetObject id=\"head\" mtl=\"\" src=\"\" /></Assets><Room><Ghost id=\"kool\" scale=\"1 1 1\" lighting=\"false\" body_id=\"body\" anim_id=\"type\" userid_pos=\"0 0.5 0\" cull_face=\"none\" /></Room></FireBoxRoom>")});
			settings.push({ name: "kool Dancing Cat 2", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" mtl=\"https://kool.website/avatars/1/WCRE80h.mtl\" src=\"https://kool.website/avatars/1/WCRE80h.obj\" /><AssetObject id=\"head\" mtl=\"\" src=\"\" /></Assets><Room><Ghost id=\"kool\" scale=\"1 1 1\" lighting=\"false\" body_id=\"body\" anim_id=\"type\" userid_pos=\"0 0.5 0\" cull_face=\"none\" /></Room></FireBoxRoom>")});
			settings.push({ name: "kool 90's Penguin", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" mtl=\"https://kool.website/avatars/1/axlyEp9.mtl\" src=\"https://kool.website/avatars/1/axlyEp9.obj\" /><AssetObject id=\"head\" mtl=\"\" src=\"\" /></Assets><Room><Ghost id=\"kool\" scale=\"1 1 1\" lighting=\"false\" body_id=\"body\" anim_id=\"type\" userid_pos=\"0 0.5 0\" cull_face=\"none\" /></Room></FireBoxRoom>")});
			settings.push({ name: "kool 90's UFO", string: escape("<FireBoxRoom><Assets><AssetObject id=\"body\" mtl=\"https://kool.website/avatars/1/w1zPYNk.mtl\" src=\"https://kool.website/avatars/1/w1zPYNk.obj\" /><AssetObject id=\"head\" mtl=\"\" src=\"\" /></Assets><Room><Ghost id=\"kool\" scale=\"1 1 1\" lighting=\"false\" body_id=\"body\" anim_id=\"type\" userid_pos=\"0 0.5 0\" cull_face=\"none\" /></Room></FireBoxRoom>")});
		}
		else if (settings.length == 0)
		{
			settings = [{ name:"Unnamed Avatar", string: escape(janus.getavatar()), }];
		}
	}
	function savesettings()
	{
		window.janus.setsetting('avatarData', JSON.stringify(settings));
	}
	function test()
	{
		
	}
	function startrename(avatar)
	{
		if (renaming_text == "" && populated == true)
		{
			renaming_avatar = avatar;
			//log('selecting');
			var trashicon = document.createElement('img');
			trashicon.src = '../inventory/images/icons/icon-trash.png';
			trashicon.style.width = '24px';
			trashicon.style.height = '24px';
			trashicon.style.verticalAlign = 'middle';
	
			trashicon.setAttribute('class', 'menu-icon right');
			trashicon.setAttribute('onclick', 'delete_avatar('+avatar+');');
			trashicon.id = 'trash_div_'+avatar;
			
			var inputdiv = document.createElement('div');
			inputdiv.setAttribute('class', 'inline');
			
			var shirtimage = document.createElement('div');
			shirtimage.className = "shirticon";
			shirtimage.setAttribute('onclick', 'load_avatar('+avatar+');');
			var itemdiv = document.getElementById('avatar_div_'+avatar);
			var input = document.createElement('input');
			renaming_text = settings[avatar].name;
			input.value = renaming_text;
			itemdiv.textContent = '';
			//itemdiv.appendChild(shirtimage);
			inputdiv.appendChild(input);
			inputdiv.appendChild(trashicon);
			inputdiv.appendChild(document.createElement("br"));
			itemdiv.appendChild(inputdiv);
			input.focus();
			input.select();
			input.onkeydown = function(ev)
			{
				if (ev.which == 13)
				{
					finishrename(avatar);
				}
			};
			input.onblur = function()
			{
				var canceltimer = setTimeout(cancelrename,100);
			}
			renaming_input_div = inputdiv;
			renaming_input = input;
			renaming_div = itemdiv;
		}
		else
		{
			//log('start failed');
		}
	}
	function finishrename(avatar)
	{
		if (renaming_text != "")
		{
			var old = settings[avatar].name;
			settings[avatar].name = renaming_input.value;
			savesettings()
			//log('Renamed "' + old + '" to "' + settings[avatar].name + '"');
			notify('Renamed "' + old + '" to "' + settings[avatar].name + '"');
			

			var text = renaming_input.value;
		
			renaming_div.removeChild(renaming_input_div);

			//renaming_div.innerHTML = "<div class='shirticon' onclick='load_avatar("+avatar+");'></div>"+text;
			renaming_div.innerHTML = text;
	
			
			
			renaming_input_div = null;
			renaming_input = null;
			renaming_div = null;
			renaming_text = "";
			
		}
		else
		{
			//log('finish failed');
		}
	}
	function cancelrename()
	{
		if (renaming_text != "")
		{
			//renaming_div.textContent = renaming_input.value;
			renaming_div.removeChild(renaming_input_div);
			//renaming_div.innerHTML = "<div class='shirticon'></div>"+renaming_text;
			//renaming_div.innerHTML = "<div class='shirticon' onclick='load_avatar("+renaming_avatar+");'></div>"+renaming_text;
			renaming_div.innerHTML = renaming_text;
			//log('rename cancelled');
			renaming_input_div = null;
			renaming_input = null;
			renaming_div = null;
			renaming_text = "";
		}
		else
		{
			//log('cancel failed');
		}
	}
	function addAvatarToList(avatar,avname)
	{
		var avname = "";
		if (settings.length > avatar)
		{
			avname = settings[avatar].name;
		}
		else
		{
			avname = "[ CURRENT PAGE ] "+roomavatars[avatar - settings.length].name;
		}
		var list = document.getElementById("avatar_list");
		var avatar_list = document.getElementById("avatar_list");
		var itemli = document.createElement('li');
		var itemdiv = document.createElement('div');
		var shirtimage = document.createElement('div');
		shirtimage.className = "shirticon";
		shirtimage.setAttribute('onclick', 'load_avatar('+avatar+');');
		var text = document.createTextNode(avname)
		itemdiv.id = 'avatar_div_'+avatar;
		itemdiv.setAttribute('onclick', 'avatar_clicked('+avatar+');');
		itemdiv.class = 'avatarcontainer';
		//itemdiv.appendChild(shirtimage)
		itemdiv.appendChild(text);
		itemli.appendChild(itemdiv);
		list.appendChild(itemli);
	}
	var timer = null;
	var swap_action = true;
	function avatar_clicked(avatar)
	{
		if (timer)
		{
			clearTimeout(timer);
			timer = null;
			if (swap_action)
			{
				startrename(avatar);
			}
			else
			{
				load_avatar(avatar);
			}
		}
		else
		{
			timer = setTimeout(function() {
				clearTimeout(timer);
				timer = null;
				if (swap_action)
				{
					load_avatar(avatar);
				}
				else
				{
					startrename(avatar);
				}
			}
			, 250);
		}
	}
	function populatelist()
	{
		renaming_text = "";
		loadsettings()
		var list = document.getElementById("avatar_list");
		list.innerHTML = "";
		for (var avatar in settings)
		{
			addAvatarToList(avatar);
			
			//log(settings[avatar].name);
		}
		load_room_avatars()
		populated = true;
	}
	function replace_Colors(input)
	{
		var output = input;
		while (output.match(/"randomColor\(\)"/i) || output.match(/"lastRandomColor\(\)"/i))
		{
			if (output.match(/"randomColor\(\)"/i) && output.match(/"lastRandomColor\(\)"/i) )
			{
				// both
				if (output.match(/"randomColor\(\)"/i).index < output.match(/"lastRandomColor\(\)"/i).index)
				{
					// randomColor() first
					output = output.replace( /"randomColor\(\)"/i, randomColor() );
				}
				else
				{
					// lastRandomColor() first
					output = output.replace( /"lastRandomColor\(\)"/i, lastRandomColor() );
				}
			}
			else if (output.match(/"randomColor\(\)"/i) )
			{
				// randomColor() only
				output = output.replace( /"randomColor\(\)"/i, randomColor() );
			}
			else if (output.match(/"lastRandomColor\(\)"/i) )
			{
				// lastRandomColor() only
				output = output.replace( /"lastRandomColor\(\)"/i, lastRandomColor() );
			}
		}
		return output;
	}
	function load_avatar(avatar)
	{
		setTimeout(function() { if (timer) { clearTimeout(timer); timer = null; } }, 10);
		var settingsavatar = null;
		if (settings.length > avatar)
		{
			settingsavatar = settings[avatar];
		}
		else
		{
			settingsavatar = roomavatars[avatar - settings.length];
		}
		var avatar_string = unescape(settingsavatar.string.replace(/></g,">"+decodeURIComponent("%0A")+"<"));
		avatar_string = avatar_string.replace( /Ghost id="(.*?)"/i, "Ghost id=\""+parent.window.janus.userid+"\"" );
		avatar_string = replace_Colors(avatar_string)
		janus.setavatar(avatar_string);
		notify('Wearing avatar: "' + settingsavatar.name + '"');
	
	}
	function ParseJsonString(str)
	{
		var returnval = null;
		try {
			returnval = JSON.parse(str);
		} catch (e) {
			returnval = null;
		}
		return returnval;
	}
	function unescapeHtml(safe)
	{
		return decodeURI(safe).replace("%3B",";").replace("%2C",",").replace("%2F","/").replace("%3F","?").replace("%3A",":").replace("%40","@").replace("%26","&").replace("%3D","=").replace("%2B","+").replace("%24","$");
	}
	function escapeHtml(unsafe)
	{
		return safe.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;").replace("'", "&#039;");
	}
	function load_room_avatars()
	{
		roomavatars = [];
		var results = GetRoomText()
		var added = 0;
		for (var index in results)
		{
			if (results[index][0].indexOf('JanusVRAvatarUI_Item') != -1)
			{
				var json_string = decodeURIComponent(results[index][1]);
				var json_avatar = null;
				try
				{
					json_avatar = JSON.parse(json_string);
				}
				catch(e)
				{
					return;
				}
				if (json_avatar)
				{
					if ('name' in json_avatar && 'string' in json_avatar)
					{
						added += 1;
						roomavatars.push({name:json_avatar.name, string:unescape(json_avatar.string)});
						addAvatarToList(settings.length-1+added);
					}
				}
			}
		}
	}
	function save_avatar_string(input, do_notify)
	{
		var avatar = input;
		if (avatar == "")
		{
			avatar = escape(janus.getavatar())
		}
		settings.push({ name:"Unnamed Avatar", string: avatar, });
		savesettings();
		addAvatarToList(settings.length-1);
		if (do_notify)
		{
			notify('Saved current avatar');
		}
	}
	function save_avatar()
	{
		settings.push({ name:"Unnamed Avatar", string: escape(janus.getavatar()), });
		savesettings();
		addAvatarToList(settings.length-1);
		notify('Saved current avatar');
	}
	function delete_avatar(avatar)
	{
		notify('Deleted avatar "' + settings[avatar].name + '"');
		settings.splice(avatar,1);
		savesettings();
		document.getElementById("avatar_list").innerHTML = "";
		populated = false;
		setTimeout( function() {
			cancelrename()
			clearTimeout(timer)
			timer = null;
			populatelist()
		} ,100);
	}
	var ghost_name = "";
	var ghost_preview = false;
	function log(text)
	{
		parent.logToConsole("/2dui/apps/avatar: "+text);
	}
	function notify(text)
	{
		
		if (!(findGetParameter("dashboard") == "true"))
		 {	
			parent.shownotification(text,'notifications/logo.png','null','#323232');
		 }
	
	}
	function preview_start()
	{
		previewbuttonmode = 1;
		var spawn_distance = 1.5;
		var pos = parent.window.janus.playerlist[0].pos;
		var zdir = parent.window.janus.playerlist[0].zdir;
		var zdirnorm = normalize2D(zdir.x, zdir.z);
		zdir = {x:-zdirnorm.x, y:0, z:-zdirnorm.y};
		var ghost_pos = Math.round((pos.x+zdirnorm.x*spawn_distance)*1000)/1000.0+"_"+Math.round(pos.y*1000)/1000.0+"_"+Math.round((pos.z+zdirnorm.y*spawn_distance)*1000)/1000.0;
		var ghost_zdir = Math.round(zdir.x*1000)/1000.0+"_"+Math.round(zdir.y*1000)/1000.0+"_"+Math.round(zdir.z*1000)/1000.0;
		var ghost_url = "";
		var url = 'http://spyduck.net/php/avatar_ghost.php';
		var data = new FormData();
		data.append('pos', ghost_pos);
		data.append('zdir', ghost_zdir);

		var avatar_string = janus.getavatar();
		avatar_string = avatar_string.replace( /Ghost id="(.*?)"/i, "Ghost id=\""+parent.window.janus.userid+"\"" );
		data.append('avatar', escape(avatar_string));
		var request = new XMLHttpRequest();
		request.onload = function()
		{
			ghost_url = this.responseText;
			
		};
		request.open('POST', url, false);
		request.send(data);
		
		ghost_name = "Avatar Preview";
		parent.window.janus.createasset('ghost', {id:ghost_name, src:ghost_url}, false);
		if (parent.window.janus.roomcode().indexOf('<Ghost id="'+ghost_name+'"') == -1)
		{
			parent.window.janus.createobject('ghost', {id:ghost_name, js_id:ghost_name, auto_play:true, loop:false, scale:"1 1 1"}, false);
		}
		ghost_preview = true;
	}
	
	function preview_end()
	{
		previewbuttonmode = 0;
		parent.window.janus.removeobject(ghost_name, false);
		ghost_preview = false;
	}


	setInterval(function()
	{
		if (renaming_div == null)
		{
			populatelist()
		}
	}
	, 1000);

	var previewbuttonmode = 0;

	function togglePreview() {
	
	if (previewbuttonmode == 0)
	{
	preview_start();
	previewbuttonmode = 1;
	}
	else
	{
	preview_end();
	previewbuttonmode = 0;	
	}
	
	}
	
	function findGetParameter(parameterName) {
    var result = null,
        tmp = [];
    location.search
    .substr(1)
        .split("&")
        .forEach(function (item) {
        tmp = item.split("=");
        if (tmp[0] === parameterName) result = decodeURIComponent(tmp[1]);
    });
    return result;
}
  
	
	
	function dashboardSetup() {

	
		  if (findGetParameter("dashboard") == "true")
		  {
	


			document.getElementsByClassName("avatarlist")[0].style.backgroundColor= "rgba(0,0,0,0)";
			document.getElementById("avatarscontainer").style.backgroundColor= "rgba(0,0,0,0)";

			document.getElementsByClassName("previewbutton")[0].style.display = "none";

						
		  }
  
	
	
	}
