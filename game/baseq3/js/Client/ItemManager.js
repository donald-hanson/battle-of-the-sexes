ItemManager = {

    Initialize : function()
    {
        Sys.Print('Item Manager loading...');
        var items = Sys.GetItems();
        for (var i=0;i<items.length;i++)
            ItemManager.StoreItem(items[i]);
        this._items = items;
        Sys.Print('completed\n');
    },
    
    _powerups : {},
    _holdables : {},
    _weapons : {},
    _pickups : {},
    _ammo : {},
    _items : [],
    
    GetAllItems : function()
    {
        return this._items;
    },
    
    StoreItem : function(item)
    {  
        if (item.giType == Constants.ItemType.Powerup || 
            item.giType == Constants.ItemType.Team ||
            item.giType == Constants.ItemType.PersistantPowerup)
            this._powerups[item.giTag] = item;
        
        if (item.giType == Constants.ItemType.Holdable)
            this._holdables[item.giTag] = item;
        
        if (item.giType == Constants.ItemType.Weapon)
            this._weapons[item.giTag] = item;
            
        if (item.giType == Constants.ItemType.Ammo)
            this._ammo[item.giTag] = item;
            
        this._pickups[item.pickupName] = item;
    },
    
    FindForAmmo : function(w)
    {
        return this._ammo[w];
    },
    
    FindForPowerup : function(pw)
    {
        return this._powerups[pw];
    },
    
    FindForHoldable : function(h)
    {
        return this._holdables[h];
    },
    
    FindForWeapon : function(w)
    {
        return this._weapons[w];
    },
    
    Find : function(name)
    {
        this._pickups[name];
    }
};

ItemManager.Initialize();