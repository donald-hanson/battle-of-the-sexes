DefaultHud = {

	Draw : function()
	{
		var ps = Game.GetPlayerState();
		if (ps.persistant[Constants.Persistant.Team] == Constants.Team.Spectator)
		{
		}
		else
		{
			if (ps.stats[Constants.Stats.Health] > 0)
			{
				DefaultHud.DrawBottomLeft(ps);
				DefaultHud.DrawBottomRight(ps);	
				DefaultHud.DrawTopRight(ps);
				DefaultHud.DrawCrosshair(ps);
			}
		}
	},

	DrawBottomLeft : function(ps)
	{
	    DefaultHud.DrawAmmo(ps);
	
        DefaultHud.DrawHealth(ps);
        DefaultHud.DrawArmor(ps);
	},
	
	DrawAmmo : function(ps)
	{
	    var l = [];
	
	    for (var w in Constants.Weapons)
	    {
	        var i = Constants.Weapons[w];
	        var v = ps.ammo[i];
	        if (v >= 0 && ps.HasWeapon(i))
	            l.push( { w : w, i : i, v : v });
	    }
	    
	    l.reverse();
	    
	    var y = 432 - ( 3 * Constants.Hud.Char.Height );

	    for (var index = 0; index < l.length; index++)
	    {
	        var info = l[index];
	        var w = info.w;
	        var i = info.i;
	        var v = info.v;
	        
		    Hud.SetColor([1.0, 0.69, 0.0, 1.0]);
		    Hud.DrawField(0, y, 3, v);
		    Hud.SetColor();
		    
			var x = Constants.Hud.Char.Width*3 + Constants.Hud.Icon.Space;
			var w = Constants.Hud.Icon.Size;
			var h = Constants.Hud.Icon.Size;		    
			
			var item = ItemManager.FindForAmmo(i);
			var icon = Game.Static.Media.ItemIcons[item.classname];
		    Hud.DrawPic(x, y, w, h, icon);

		    y -= Constants.Hud.Char.Height;
	    }
	},
	
	DrawHealth : function(ps)
	{
	    var y = 432 - ( 1 * Constants.Hud.Char.Height);
	
		var colors = [
			[1.0, 0.69, 0.0, 1.0],
			[1.0, 0.2, 0.2, 1.0],
			[0.5, 0.5, 0.5, 1.0],
			[1.0, 1.0, 1.0, 1.0]
		];
		
		// Draw Health Number
		var health = ps.stats[Constants.Stats.Health];
		{
		    if (health > 100)
			    colorIndex = 3;
		    else if (health > 25)
			    colorIndex = 0;
		    else if (health > 0)
			    colorIndex = (Game.ServerTime >> 8) & 1;
		    else
			    colorIndex = 1;
		    Hud.SetColor(colors[colorIndex]);
		    Hud.DrawField(0, y, 3, health);
		    Hud.SetColor();	
        }
		
	    //Draw Health Model
        /*
		{
			var origin = new Vec3(60,0,-5);
			var angles = new Vec3(0,(Game.ServerTime & 2047) * -360.0 / 2048.0,0);
			var x = Constants.Hud.Char.Width*3 + Constants.Hud.Icon.Space;
			var w = Constants.Hud.Icon.Size;
			var h = Constants.Hud.Icon.Size;
			Hud.Draw3DModel(x, y, w, h, Game.Static.Media.healthSphereModel, 0, origin, angles);
			Hud.Draw3DModel(x, y, w, h, Game.Static.Media.healthCrossModel, 0, origin, angles);
		}
        */
	},
	
	DrawArmor : function(ps)
	{
	    var y = 432 - ( 0 * Constants.Hud.Char.Height );
	
        //Draw Armor Number
		var armor = ps.stats[Constants.Stats.Armor];
		{
			Hud.SetColor([1.0, 0.69, 0.0, 1.0]);
			Hud.DrawField(0, y, 3, armor);
			Hud.SetColor();
		}
		
	    //Draw Armor Model
        /*
		{
			var origin = new Vec3(90,0,-10);
			var angles = new Vec3(0,(Game.ServerTime & 2047) * 360.0 / 2048.0,0);
			var x = Constants.Hud.Char.Width*3 + Constants.Hud.Icon.Space;
			var w = Constants.Hud.Icon.Size;
			var h = Constants.Hud.Icon.Size;
			Hud.Draw3DModel(x, y, w, h, Game.Static.Media.armorModel, 0, origin, angles);
		}
        */
	},
	
	DrawBottomRight : function(ps)
	{
	    var team = ps.persistant[Constants.Persistant.Team];
	
	    var redScore = TeamInfo.GetScore(Constants.Team.Red);
	    var bluScore = TeamInfo.GetScore(Constants.Team.Blue);
	    
	    var redPromo = TeamInfo.GetPromotionPoints(Constants.Team.Red);
	    var bluPromo = TeamInfo.GetPromotionPoints(Constants.Team.Blue);
	    
        var redTech = TeamInfo.GetTechPoints(Constants.Team.Red);
	    var bluTech = TeamInfo.GetTechPoints(Constants.Team.Blue);	    
	    
	    var redColor = [ 1.0, 0.0, 0.0, 1.0 ];
	    var bluColor = [ 0.0, 0.0, 1.0, 1.0 ];
	    
	    var isRedTeam = team == Constants.Team.Red;
	    var isBluTeam = team == Constants.Team.Blue;
	    
	    var redMsg = isRedTeam ? redScore.toString() + '/' + redPromo.toString() + '/' + redTech.toString() : redScore.toString();
	    var bluMsg = isBluTeam ? bluScore.toString() + '/' + bluPromo.toString() + '/' + bluTech.toString() : bluScore.toString();

	    var top = 480 - (2 * Constants.Hud.SmallChar.Height);
	    var bottom = 480 - (1 * Constants.Hud.SmallChar.Height);
	    
	    var redPosition = isRedTeam ? bottom : top;
	    var bluPosition = isBluTeam ? bottom : top;
	    
	    Hud.DrawBigStringColor(640, bluPosition, bluMsg, bluColor, Constants.Hud.Alignment.Right);	    
	    Hud.DrawBigStringColor(640, redPosition, redMsg, redColor, Constants.Hud.Alignment.Right);
	},
	
	DrawTopRight: function(ps)
	{
	    var cls = ps.persistant[Constants.Persistant.Class];
	    for (var n in Constants.Class)
	        if (cls == Constants.Class[n])
	            Hud.DrawBigString(640, 0, n, 1.0, Constants.Hud.Alignment.Right);

        var lvl = ps.persistant[Constants.Persistant.Level];
        Hud.DrawBigString(640, Constants.Hud.BigChar.Height, "Level " + lvl.toString(), 1.0, Constants.Hud.Alignment.Right);
	},
	
	DrawCrosshair : function(ps)
	{
	    var c = Sys.Cvar.GetInt("cg_drawCrosshair");
	    if (c == 0)
	        return;
	        
	    if (c < 0) c = 0;

	    var shader = Game.Static.Media.crosshairShader[c % Game.Static.Media.crosshairShader.length ];
	    
	    var x = Sys.Cvar.GetInt("cg_crosshairX");
	    var y = Sys.Cvar.GetInt("cg_crosshairY");
	    var w = Sys.Cvar.GetInt("cg_crosshairSize");
	    var h = Sys.Cvar.GetInt("cg_crosshairSize");
	    
	    var a = Hud.AdjustFrom640(x,y,w,h);
	    
	    x = a[0];
	    y = a[1];
	    w = a[2];
	    h = a[3];
	    
	    var sW = Game.GLConfig.vidWidth;
	    var sH = Game.GLConfig.vidHeight;

        if (Sys.Cvar.GetInt("cg_crosshairHealth") > 0)
            Hud.SetColor(Hud.GetColorForHealth(ps));
        else
            Hud.SetColor();

        Sys.DrawStretchPic(x + 0.5 * (sW - w), y + 0.5 * (sH - h), w, h, 0, 0, 1, 1, shader);
	}
};

Event.Subscribe( "Hud/Draw", DefaultHud.Draw );