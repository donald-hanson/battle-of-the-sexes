Util = {

	Origin : { X : 0, Y : 0, Z : 0 },

	VectorSubtract : function(a,b,c)
	{
		c.X = a.X - b.X;
		c.Y = a.Y - b.Y;
		c.Z = a.Z - b.Z;
	},

	AxisClear : function(z)
	{
		var q = {
			X : { X : 1, Y : 0, Z : 0 },
			Y : { X : 0, Y : 1, Z : 0 },
			Z : { X : 0, Y : 0, Z : 1 }
		};
		z = q;
		return q;
	},

	AnglesToAxis : function(a, q)
	{
		Util.AngleVectors(a, q.X, q.Y, q.Z);
		Util.VectorSubtract(Util.Origin, q.Y, q.Y);

		return q;
	},

	AngleVectors : function(angles, forward, right, up)
	{
		var angle, sr, sp, sy, cr, cp, cy;

		angle = angles.Y * (Math.PI * 2 / 360);
		sy = Math.sin(angle);
		cy = Math.cos(angle);

		angle = angles.X * (Math.PI * 2 / 360);
		sp = Math.sin(angle);
		cp = Math.cos(angle);
		
		angle = angles.Z * (Math.PI * 2 / 360);
		sr = Math.sin(angle);
		cr = Math.cos(angle);

		if (forward)
		{
			forward.X = cp * cy;
			forward.Y = cp * sy;
			forward.Z = -sp;
		}
		if (right)
		{
			right.X = (-1*sr*sp*cy+-1*cr*-sy);
			right.Y = (-1*sr*sp*sy+-1*cr*cy);
			right.Z = -1*sr*cp;
		}
		if (up)
		{
			up.X = (cr*sp*cy+-sr*-sy);
			up.Y = (cr*sp*sy+-sr*cy);
			up.Z = cr*cp;
		}
	},
	
	StripColor : function(s)
	{
        for (var i=0;i<8;i++)
            s = s.replace('^' + i.toString(), '');
        return s;
	},

	IsColorString : function(s, index)
	{
	    if (s.charAt(index) == '^')
	        if (index + 1 < s.length)
                return Util._ColorIndex(s.charAt(index+1)) >= 0;
	    return false;
	},
	
	_ColorIndex : function(c)
	{
	    var x = c.charCodeAt(0);
	    if (x < 48 || x > 55)
	        return -1;
	    return x - 48;
	},
	
	_colorTable : [
            [0.0, 0.0, 0.0, 1.0],
	        [1.0, 0.0, 0.0, 1.0],
	        [0.0, 1.0, 0.0, 1.0],
	        [1.0, 1.0, 0.0, 1.0],
	        [0.0, 0.0, 1.0, 1.0],
	        [0.0, 1.0, 1.0, 1.0],
	        [1.0, 0.0, 1.0, 1.0],
	        [1.0, 1.0, 1.0, 1.0],
    ],
	
	LookupColor : function(s, index)
	{
	    var c = s.charAt(index+1);
	    var i = Util._ColorIndex(c);
	    return Util._colorTable[i];
	}
};