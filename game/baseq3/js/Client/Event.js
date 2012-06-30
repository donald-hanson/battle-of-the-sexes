Event = {

	Publish : function(name, obj)
	{
		var funcs = this._subs[name] || [];
		for (var i=0;i<funcs.length;i++)
		{
			try
			{
				funcs[i](obj);
			}
			catch (e)
			{
				Sys.Error("Event.Publish: Uncaught Exception: " + e.toString() + "\n");
			}
		}
	},

	Subscribe : function(name, callback)
	{
		var subs = this._subs[name] || (this._subs[name] = []);
		subs.push(callback);
	},

	_subs : { }
};