ClassHud = {
    _Draw : function (ps, funcName)
    {
        var cls = ps.persistant[Constants.Persistant.Class];
        for (var n in Constants.Class)
        {
            if (cls == Constants.Class[n])
            {
                var other = ClassHud[n];
                if (other)
                {
                    var func = other[funcName];
                    if (func)
                        func(ps);
                }
            }
        }        
    },

    DrawTopRight: function(ps)
    {
        ClassHud._Draw(ps, "DrawTopRight");
    },

    Captain: {
        DrawTopRight: function(ps)
        {
        }
    },

    Bodyguard: {
        DrawTopRight: function(ps)
        {
            var level = ps.persistant[Constants.Persistant.Level];
            var cs = Sys.GetClassState();

            var laserLevel = level < 1 ? 1 : level;
            var messages = [];
            for (i=0;i<3;i++)
            {
                if (laserLevel < i+1)
                    continue;

                var laserState = cs.lasers[i];
                var status = "";
                if (!laserState.active)
                    status = "N/A";
                else if (laserState.on)
                    status = "On ";
                else
                    status = "Off";
                messages.push("Laser " + (i + 1) + ": " + status);
            }

            if (level > 0)
            {
                if (cs.decoyActive)
                    messages.push("Decoy: On  (" + cs.decoyTime.toString() + "s)");
                else
                    messages.push("Decoy: Off (" + cs.decoyTime.toString() + "s)");
            }

            if (level > 0)
                messages.push("Protect: " + (cs.protect ? "On ": "Off"));
           
            for(i=0;i<messages.length;i++)
                Hud.DrawSmallString(640, 32 + (i * 16), messages[i], 1.0, Constants.Hud.Alignment.Right);
        }
    },

    Sniper: {
        DrawTopRight: function(ps)
        {
        }
    },

    Soldier: {
        DrawTopRight: function(ps)
        {
        }
    },

    Berzerker: {
        DrawTopRight: function(ps)
        {
        }
    },

    Infiltrator: {
        DrawTopRight: function(ps)
        {
        }
    },

    Kamikazee: {
        DrawTopRight: function(ps)
        {
        }
    },
    
    Nurse: {
        DrawTopRight: function(ps)
        {
        }
    },
    
    Scientist: {
        DrawTopRight: function(ps)
        {
        }
    }
};