Game = {
	Print : function Game_Print(msg)
	{
		Sys.Print(msg);
	},

	Error : function Game_Error(msg)
	{
		Sys.Error(msg);
	},

	Init : function Game_Init(serverMessageNum, serverCommandSequence, clientNum)
	{
		Game.Print("^6serverMessageNum: " + serverMessageNum + " serverCommandSequence: " + serverCommandSequence + " clientNum: " + clientNum + "\n");

		Game.GLConfig = Sys.GetGLConfig();
		
		Game.Static.screenXScale = Game.GLConfig.vidWidth / 640.0;
		Game.Static.screenYScale = Game.GLConfig.vidHeight / 480.0;
		
		Game.LoadMedia();
	},

	LoadMedia : function Game_LoadMedia()
	{
		Game.Static.Media.charsetShader = Sys.RegisterShader("gfx/2d/bigchars");
		Game.Static.Media.whiteShader = Sys.RegisterShader("white");
		Game.Static.Media.charsetProp = Sys.RegisterShaderNoMip( "menu/art/font1_prop.tga" );
		Game.Static.Media.charsetPropGlow = Sys.RegisterShaderNoMip( "menu/art/font1_prop_glo.tga" );
		Game.Static.Media.charsetPropB = Sys.RegisterShaderNoMip( "menu/art/font2_prop.tga" );

		Game.Static.Media.friendShader = Sys.RegisterShader("sprites/foe");
		Game.Static.Media.redQuadShader = Sys.RegisterShader("powerups/blueflag");
		Game.Static.Media.teamStatusBar = Sys.RegisterShader("gfx/2d/colorbar.tga");

		Game.Static.Media.armorModel = Sys.RegisterModel("models/powerups/armor/armor_yel.md3");
		Game.Static.Media.armorIcon = Sys.RegisterShaderNoMip("icons/iconr_yellow");
		
		Game.Static.Media.healthSphereModel = Sys.RegisterModel("models/powerups/health/medium_sphere.md3");
		Game.Static.Media.healthCrossModel = Sys.RegisterModel("models/powerups/health/medium_cross.md3");

		var sb_nums = [
			"gfx/2d/numbers/zero_32b",
			"gfx/2d/numbers/one_32b",
			"gfx/2d/numbers/two_32b",
			"gfx/2d/numbers/three_32b",
			"gfx/2d/numbers/four_32b",
			"gfx/2d/numbers/five_32b",
			"gfx/2d/numbers/six_32b",
			"gfx/2d/numbers/seven_32b",
			"gfx/2d/numbers/eight_32b",
			"gfx/2d/numbers/nine_32b",
			"gfx/2d/numbers/minus_32b"
		];

		for (var i=0;i<sb_nums.length;i++)
			Game.Static.Media.numberShaders.push(Sys.RegisterShader(sb_nums[i]));

		for (var i=0;i<10;i++)
		{
			var c = String.fromCharCode(97 + i);
			Game.Static.Media.crosshairShader.push(Sys.RegisterShader("gfx/2d/crosshair" + c));
		}

		var items = ItemManager.GetAllItems();
		for (var i=0;i<items.length;i++)
		{
		    var item = items[i];
		    Game.Static.Media.ItemIcons[item.classname] = Sys.RegisterShaderNoMip(item.icon);
		}
	},

	Static : {
		Media : {
			numberShaders : [],
			crosshairShader : [],
			ItemIcons : {}
		}
	},

	Draw2D: function Game_Draw2D()
	{
		Hud.Draw();
	},
	
	GetPlayerState: function Game_GetPlayerState()
	{
	    var ps = Sys.GetPlayerState();

        /*
	    var ps = {
	        stats: [],
	        persistant: [],
	        powerups: [],
	        ammo: [],
	        maxammo:[]
	    };

	    var ms = Sys.Milliseconds();

        if (ms > 10000)
            ps.persistant[Constants.Persistant.Team] = Constants.Team.Red;
	    else
            ps.persistant[Constants.Persistant.Team] = Constants.Team.Spectator;

	    ps.persistant[Constants.Persistant.Class] = Constants.Class.Soldier;
	    ps.persistant[Constants.Persistant.Level] = 32;
	    ps.stats[Constants.Stats.Health] = 50;
        ps.stats[Constants.Stats.Armor] = 0;
        ps.stats[Constants.Stats.Weapons] |= (1 << Constants.Weapons.RocketLauncher);
        ps.ammo[Constants.Weapons.RocketLauncher] = 50;
        ps.maxammo[Constants.Weapons.RocketLauncher] = 250;
        */
	    
	    ps.HasWeapon = function PlayerState_HasWeapon(i) {
	        var x = (1 << i);
	        var y = this.stats[Constants.Stats.Weapons];
	        var v = y & x;
	        return v == x;
	    };
	    
	    return ps;
	},

	CreateRefEntity: function Game_CreateRefEntity()
	{
		return {
			reType : 0,
			renderfx : 0,
			hModel : 0,
			lightingOrigin : { X : 0, Y : 0, Z : 0 },
			shadowPlane : 0,
			axis : {
				X : { X : 0, Y : 0, Z : 0 },
				Y : { X : 0, Y : 0, Z : 0 },
				Z : { X : 0, Y : 0, Z : 0 }	
			},
			nonNormalizedAxis : 0,
			origin : { X : 0, Y : 0, Z : 0 },
			frame : 0,
			oldorigin : { X : 0, Y : 0, Z : 0 },
			oldframe : 0,
			backlerp : 0,
			skinNum : 0,
			customSkin : 0,
			customShader : 0,
			shader : {
			    R: 0, G: 0, B: 0, A: 0,
			    X: 0, Y: 0,
                Time : 0
			},
			radius : 0,
			rotation : 0
		};
	},

	CreateRefDef: function Game_CreateRefDef()
	{
		return {
			x : 0,
			y : 0,
			width : 0,
			height : 0,
			vieworg : { X : 0, Y : 0, Z : 0 },
			viewaxis : {
				X : { X : 0, Y : 0, Z : 0 },
				Y : { X : 0, Y : 0, Z : 0 },
				Z : { X : 0, Y : 0, Z : 0 }
			},
			time : 0,
			rdflags : 0
		};
	}
};