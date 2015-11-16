Debug = {
    Stack: {
        Print : function Debug_Stack_Print(printer, ex)
        {
            var exception = ex;
            if (!exception)
            {
                try
                {
                    this.___exception___();
                }
                catch (e)
                {
                    exception = e;
                }
            }

            if (exception)
            {
                printer("====================\n");
                printer(exception.stack.replace(/(?:\n@:0)?\s+$/m, '').replace(/^\(/gm, '{anonymous}(') + "\n");
                printer("====================\n");
            }
        },
    },
	Json : {
		Serialize : function (v)
		{
			var a = [];
			this._Serialize(v, a);
			return a.join('');
		},
	
		_Serialize : function (v, a)
		{
		    if (v === undefined)
		    {
		        a.push("undefined");
		        return;
		    }

			if (v === null) 
			{
				a.push("null");
				return;
			}
			
			switch (typeof(v))
			{
				case 'number':
					//if (!isFinite(v)) throw "Debug.Json.Serialize: Not a valid JSON number: " + v.toString();
					a.push(v.toString());
					break;
				case 'string':
					this._SerializeString(v, a);
					break;
				case 'boolean':
					a.push(v.toString());
					break;
				case 'object':
					if (v instanceof Array)
						this._SerializeArray(v, a);
					else
						this._SerializeObject(v, a);
					break;
				default:
					throw "Debug.Json.Serialize: Not a valid JSON value: " + String(v);
			}
		},
	
		_rxStrSegs : /[^\u0000-\u001F\"\\]+|[\u0000-\u001F\"\\]+/g,
		_SerializeString : function (v, a)
		{
			a.push('"');
			var m = v.match(this._rxStrSegs);
			if (m)
			{
				var c = m[0].charAt(0);
				var b = c == '"' || c == '\\' || c < ' ';
				for (var i = 0; i < m.length; ++i)
				{
					var n = m[i];
					if (b)
					{
						for (var j = 0; j < n.length; ++j)
						{
							c = n.charAt(j);
							switch (c)
							{
								case '"':
									a.push('\\"');
									break;
								case '\\':
									a.push('\\\\');
									break;
								case '\b':
									a.push('\\b');
									break;
								case '\f':
									a.push('\\f');
									break;
								case '\n':
									a.push('\\n');
									break;
								case '\r':
									a.push('\\r');
									break;
								case '\t':
									a.push('\\t');
									break;
								default:
									a.push('\\u');
									var hex = '000' + c.charCodeAt(0).toString(16);
									a.push(hex.substr(hex.length-4));
							}
						}
					}
					else
						a.push(n);
					b = !b;
				}
			}
			a.push('"');
		},
	
		_SerializeArray : function (v, a)
		{
			a.push("[");
			var needcomma = false;
			for (var i = 0; i < v.length; ++i)
			{
				if (needcomma) a.push(",");
				else needcomma = true;
				this._Serialize( v[i], a );
			}
			a.push("]");
		},
		
		_SerializeObject : function (v, a)
		{
			a.push("{");
			var needcomma = false;
			for (p in v)
			{
				var vp = v[p];
				var type = typeof vp;
				if (type == 'function' || type == 'undefined') continue;
				if (needcomma) a.push(",");
				else needcomma = true;
				this._SerializeString( p, a );
				a.push(":");
				this._Serialize( vp, a );
			}
			a.push("}");
		},
		
		Deserialize : function (s)
		{
			return Debug.Json.EvalExpr(s);
		},
	
		EvalExpr : function (_)
		{
			return (function(){return eval('(' + _ + ')')})();
		}
	}
};