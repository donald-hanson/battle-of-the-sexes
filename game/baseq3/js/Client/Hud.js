Hud = {

	Draw : function()
	{
		if (Sys.Cvar.GetInt("cg_draw2D") == 0)
			return;

		Event.Publish("Hud/Draw");
	},

	SetColor : function(o)
	{
		if (o instanceof Array)
			Sys.SetColor(o[0], o[1], o[2], o[3]);
		else if (o instanceof Object)
			Sys.SetColor(o.R, o.G, o.B, o.A);
		else
			Sys.SetColor();
	},

	GetColorForHealth : function()
	{
		var ps = Game.GetPlayerState();
		return Hud.GetColorForHealth(ps.stats[Constants.Stats.Health], ps.stats[Constants.Stats.Armor]);
	},

	GetColorForHealth : function(health, armor)
	{
		if (health <= 0)
			return [0,0,0,1];

		var count = armor;
		var max = health * Constants.ArmorProtection / (1.0 - Constants.ArmorProtection);
		if (max < count)
			count = max;
		health += count;
		
		var r = 1.0;
		var g = 0;
		var b = 0;
		var a = 1.0;
		if (health >= 100)
			b = 1.0;
		else if (health < 66)
			b = 0;
		else
			b = (health-66)/33.0;

		if (health > 60)
			g = 1.0;
		else if (health < 30)
			g = 0;
		else
			g = (health-30)/30.0;
		
		return [r,g,b,a];
	},

	AdjustFrom640 : function(x,y,w,h)
	{
		xs = Game.Static.screenXScale;
		ys = Game.Static.screenYScale;
		return [x * xs, y * ys, w * xs, h * ys];
	},

	DrawField : function(x,y,width,value)
	{
		value = parseInt(value.toString());
		if (width < 1)
			return;
		if (width > 5)
			width = 5;
		switch (width)
		{
			case 1:
				value = value > 9 ? 9 : value;
				value = value < 0 ? 0 : value;
				break;
			case 2:
				value = value > 99 ? 99 : value;
				value = value < -9 ? -9 : value;
				break;
			case 3:
				value = value > 999 ? 999 : value;
				value = value < -99 ? -99 : value;
				break;
			case 4:
				value = value > 9999 ? 9999 : value;
				value = value < -999 ? -999 : value;
				break;
		}

		var s = value.toString();
		var l = s.length;
		if (l > width)
			l = width;	


		x += 2 + Constants.Hud.Char.Width * (width - l);
		i = 0;
		while(1)
		{
			if (l == 0)
				break;

			var c = s.charAt(i);

			if (c == '-')
				frame = 10;
			else
				frame = parseInt(c);
	
			Hud.DrawPic( x, y, Constants.Hud.Char.Width, Constants.Hud.Char.Height, Game.Static.Media.numberShaders[frame] );

			x += Constants.Hud.Char.Width;

			i++;
			l--;
		}
	},

	DrawPic : function(x,y,w,h,handle)
	{
		var a = Hud.AdjustFrom640(x,y,w,h);
		Sys.DrawStretchPic( a[0], a[1], a[2], a[3], 0, 0, 1, 1, handle );
	},

	Draw3DModel : function(x, y, w, h, model, skin, origin, angles)
	{
		if (Sys.Cvar.GetInt("cg_draw3dIcons") == 0 || Sys.Cvar.GetInt("cg_drawIcons") == 0)
			return;

		var a = Hud.AdjustFrom640(x,y,w,h);

		var ent = Game.CreateRefEntity();

		Util.AnglesToAxis(angles, ent.axis);
		ent.origin = origin;
		ent.hModel = model;
		ent.customSkin = skin;
		ent.renderfx = Constants.RenderFx.NoShadow;

		var refdef = Game.CreateRefDef();
		refdef.rdflags = Constants.RefDefFlag.NoWorldModel;
		refdef.viewaxis = Util.AxisClear();
		refdef.fov_x = 30;
		refdef.fov_y = 30;
		refdef.x = a[0];
		refdef.y = a[1];
		refdef.width = a[2];
		refdef.height = a[3];
		refdef.time = Game.ServerTime;
		
		Sys.ClearScene();
		Sys.AddRefEntityToScene(ent);
		Sys.RenderScene(refdef);
	},
	
	DrawStrlen : function (s)
	{
	    return Util.StripColor(s).length;
	},
	
	DrawChar : function(x, y, width, height, ch)
	{
	    ch = ch.charCodeAt(0);
	    ch &= 255;
	    if (ch == ' '.charCodeAt(0))
	        return;
	        
	    var a = Hud.AdjustFrom640(x,y,width,height);
	    var row = ch >> 4;
	    var col = ch & 15;
	    var frow = row * 0.0625;
	    var fcol = col * 0.0625;
	    var size = 0.0625;
	    
	    Sys.DrawStretchPic( a[0], a[1], a[2], a[3], fcol, frow, fcol + size, frow + size, Game.Static.Media.charsetShader );
	},
	
	DrawStringExt : function(x, y, string, setColor, forceColor, shadow, charWidth, charHeight, maxChars, alignment)
	{
	    if (!alignment)
	        alignment = Constants.Hud.Alignment.Left;

        switch (alignment)
        {  
            case Constants.Hud.Alignment.Center:
                x = x - ((Hud.DrawStrlen(string) * charWidth) / 2.0);
                break;
            case Constants.Hud.Alignment.Right:
                x = x - (Hud.DrawStrlen(string) * charWidth);
                break;
        }
	
	    if (shadow)
	    {
	        var color = [0, 0, 0, setColor[3]];
	        Hud.SetColor(color);
	        
            var s = Util.StripColor(string);
            var xx = x;
            var c = maxChars;
            if (c <= 0)
	            c = s.length;
	        for (var i=0;i<c;i++,xx += charWidth)
	            Hud.DrawChar(xx+2,y+2, charWidth, charHeight, s.charAt(i));
	    }
	    
	    Hud.SetColor(setColor);
	    var xx = x;
	    var c = maxChars;
	    if (c <= 0)
	        c = string.length;
        for (var i=0;i<c;i++)
        {
            if (Util.IsColorString(string, i))
            {  
                if (!forceColor)
                {
                    var newColor = Util.LookupColor(string, i);
                    newColor[3] = setColor[3];
                    Hud.SetColor(newColor);
                }
                i++;
                continue;
            }
            Hud.DrawChar(xx,y,charWidth,charHeight, string.charAt(i));
            xx += charWidth;
        }
	    
	    Hud.SetColor();
	},
	
	DrawBigString : function(x, y, s, a, alignment)
	{
	    var color = [1.0, 1.0, 1.0, a];
	    Hud.DrawStringExt(x, y, s, color, false, true, Constants.Hud.BigChar.Width, Constants.Hud.BigChar.Height, 0, alignment);	
	},
	
	DrawBigStringColor : function(x, y, s, color, alignment)
	{
	    Hud.DrawStringExt(x, y, s, color, true, true,  Constants.Hud.BigChar.Width, Constants.Hud.BigChar.Height, 0, alignment);
	},
	
	DrawSmallString : function(x, y, s, a, alignment)
	{
	    var color = [1.0, 1.0, 1.0, a];
	    Hud.DrawStringExt(x, y, s, color, false, false, Constants.Hud.SmallChar.Width, Constants.Hud.SmallChar.Height, 0, alignment);
	},
	
	DrawSmallStringColor : function(x, y, s, color, alignment)
	{
	    Hud.DrawStringExt(x, y, s, color, true, false,  Constants.Hud.SmallChar.Width, Constants.Hud.SmallChar.Height, 0, alignment);
	}
};