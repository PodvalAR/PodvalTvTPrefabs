modded class PS_MissionDataManager
{
    override void LateInit()
    {
        m_GameModeCoop = PS_GameModeCoop.Cast(GetOwner());
        m_PlayerManager = GetGame().GetPlayerManager();
        m_PlayableManager = PS_PlayableManager.GetInstance();
        m_ObjectiveManager = PS_ObjectiveManager.GetInstance();
        m_FactionManager = GetGame().GetFactionManager();
        
        GetGame().GetCallqueue().CallLater(DelayedInit, 1000, false);
        
        m_GameModeCoop.GetOnGameStateChange().Insert(OnGameStateChanged);
        m_GameModeCoop.GetOnPlayerAuditSuccess().Insert(OnPlayerAuditSuccess);
        if (RplSession.Mode() != RplMode.Dedicated) 
            OnPlayerAuditSuccess(GetGame().GetPlayerController().GetPlayerId());
    }
    
    void DelayedInit()
    {
        if (!Replication.IsServer())
            return;
        
        PS_PlayableManager playableManager = PS_PlayableManager.GetInstance();
        if (!playableManager)
            return;
        
        array<PS_PlayableContainer> playables = playableManager.GetPlayablesSorted();
        foreach (PS_PlayableContainer playable : playables)
        {
            PS_PlayableComponent playableComp = playable.GetPlayableComponent();
            if (!playableComp)
                continue;
            
            SCR_CharacterDamageManagerComponent dmg = playableComp.GetCharacterDamageManagerComponent();
            if (dmg)
            {
                dmg.GetOnDamageStateChanged().Insert(OnPlayableDamageStateChanged);
            }
        }
    }
    
    // Сохраняем map для отслеживания уже записанных смертей
    protected ref set<RplId> m_RecordedDeaths = new set<RplId>();
    
    void OnPlayableDamageStateChanged(EDamageState state)
    {
        if (state != EDamageState.DESTROYED)
            return;
        
        PS_PlayableManager playableManager = PS_PlayableManager.GetInstance();
        if (!playableManager)
            return;
        
        array<PS_PlayableContainer> playables = playableManager.GetPlayablesSorted();
        foreach (PS_PlayableContainer playable : playables)
        {
            PS_PlayableComponent playableComp = playable.GetPlayableComponent();
            if (!playableComp)
                continue;
            
            SCR_CharacterDamageManagerComponent dmg = playableComp.GetCharacterDamageManagerComponent();
            if (!dmg || !dmg.IsDestroyed())
                continue;
            
            RplId playableRplId = playableComp.GetRplId();
            
            // Уже записали эту смерть
            if (m_RecordedDeaths.Contains(playableRplId))
                continue;
            
            int playerId = playableManager.GetPlayerByPlayable(playableRplId);
            if (playerId <= 0)
                continue;
            
            m_RecordedDeaths.Insert(playableRplId);
            
            // Получаем killer через Instigator из damage manager
            Instigator killer = dmg.GetInstigator();
            int killerId = -1;
            if (killer)
                killerId = killer.GetInstigatorPlayerID();
            
            Print(string.Format("PLAYER DIED: %1 (killed by %2)", playerId, killerId), LogLevel.WARNING);
            
            PS_MissionDataPlayerKill killData = new PS_MissionDataPlayerKill();
            killData.m_iPlayerId = playerId;
            killData.InstigatorId = killerId;
            killData.Time = GetGame().GetWorld().GetWorldTime();
            killData.SystemTime = System.GetUnixTime();
            m_Data.Kills.Insert(killData);
        }
    }
}