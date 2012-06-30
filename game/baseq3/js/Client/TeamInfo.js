TeamInfo = {
       
    Initialize : function()
    {
        for (var k in Constants.Team)
        {
            var v = Constants.Team[k];
            
            this.SetPromotionPoints(v, 0);
            this.SetTechPoints(v, 0);
            this.SetScore(v, 0);
        }
    },
    
    _teamInfo : {},
    
    _GetTeamInfo : function(team)
    {
        return this._teamInfo[team] = this._teamInfo[team] || {};
    },
    
    SetPromotionPoints : function(team, value)
    {
        this._GetTeamInfo(team).promoPoints = value;
    },

    GetPromotionPoints : function(team)
    {
        return this._GetTeamInfo(team).promoPoints;
    },
    
    SetTechPoints : function(team, value)
    {
        this._GetTeamInfo(team).techPoints = value;
    },

    GetTechPoints : function(team)
    {
        return this._GetTeamInfo(team).techPoints;
    },
    
    SetScore :function(team, value)
    {
        this._GetTeamInfo(team).score = value;
    },
    
    GetScore : function(team)
    {
        return this._GetTeamInfo(team).score;
    }
};

TeamInfo.Initialize();