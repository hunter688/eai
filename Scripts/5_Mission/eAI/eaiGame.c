// Copyright 2021 William Bowers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

class eAIGame {
	
	// On client, list of weapons we are asked to provide a feed of
	autoptr eAIClientAimArbiterManager m_ClientAimMngr;
	
	// Server side list of weapon data 
	autoptr eAIServerAimProfileManager m_ServerAimMngr;
	
	autoptr array<string> adminIDs = {};
	
	vector debug_offset = "8 0 0"; // Offset from player to spawn a new AI entity at when debug called
	vector debug_offset_2 = "20 0 0"; // Electric bugaloo
	
	float gametime = 0;
	
    void eAIGame() {
		if (GetGame().IsClient()) {
			m_ClientAimMngr = new eAIClientAimArbiterManager();
			GetRPCManager().AddRPC("eAI", "eAIAimArbiterSetup", m_ClientAimMngr, SingeplayerExecutionType.Client);
			GetRPCManager().AddRPC("eAI", "eAIAimArbiterStart", m_ClientAimMngr, SingeplayerExecutionType.Client);
			GetRPCManager().AddRPC("eAI", "eAIAimArbiterStop", m_ClientAimMngr, SingeplayerExecutionType.Client);
			GetRPCManager().AddRPC("eAI", "HCLinkObject", m_ClientAimMngr, SingeplayerExecutionType.Client);
			GetRPCManager().AddRPC("eAI", "HCUnlinkObject", m_ClientAimMngr, SingeplayerExecutionType.Client);
		}
		
		if (GetGame().IsServer()) {
			m_ServerAimMngr = new eAIServerAimProfileManager();
			GetRPCManager().AddRPC("eAI", "eAIAimDetails", m_ServerAimMngr, SingeplayerExecutionType.Server);
		}
		
		GetRPCManager().AddRPC("eAI", "ReqDebugMenu", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "SpawnEntity", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "DebugFire", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "DebugParticle", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "SpawnZombie", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "ClearAllAI", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "ProcessReload", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "ReqFormationChange", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "ReqFormRejoin", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "ReqFormStop", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "DayZPlayerInventory_OnEventForRemoteWeaponAICallback", this, SingeplayerExecutionType.Server);
    }
	
	void ReqDebugMenu(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<string> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server) {
			Print("eAI: Debug menu request");
			string id = sender.GetPlainId();
			if (adminIDs.Find(id) > -1) {
				GetRPCManager().SendRPC("eAI", "ReqDebugMenu", new Param1<string>(id), true, sender);
				Print("Access granted to " + sender.GetPlainId() + " ("+sender.GetName()+")");
			} else Print("Access denied to " + sender.GetPlainId() + " ("+sender.GetName()+")");
		} else {
			Print("Accessing Debug Menu for " + data.param1 + " as " + GetGame().GetPlayer().GetIdentity().GetPlainId());
			//if (data.param1.ToInt() > 0) {
				Print("Access Granted");
				//if (!eAICommandMenu.instance) new eAICommandMenu();
				//GetGame().GetUIManager().ShowScriptedMenu(eAICommandMenu.instance, null);
				if (!GetGame().GetUIManager().FindMenu(EAI_COMMAND_MENU))
				{
					GetGame().GetUIManager().EnterScriptedMenu(EAI_COMMAND_MENU, NULL);
				}

			//}
		}
	}
	
	// return the group owned by leader, otherwise create a new one.
	eAIGroup GetGroupByLeader(PlayerBase leader, bool createIfNoneExists = true) {
		for (int i = 0; i < eAIGroup.GROUPS.Count(); i++) {
			if (!eAIGroup.GROUPS[i]) continue;
			eAIBase GrpLeader = eAIGroup.GROUPS[i].GetLeader();
			if (GrpLeader && GrpLeader == leader)
				return eAIGroup.GROUPS[i];
		}
		
		if (!createIfNoneExists) return null;
		
		eAIGroup newGroup = new eAIGroup();
		newGroup.SetLeader(leader);
		leader.SetGroup(newGroup);
		return newGroup;
	}
	
	//! @param owner Who is the manager of this AI
	//! @param formOffset Where should this AI follow relative to the formation?
	eAIBase SpawnAI_Helper(DayZPlayer owner, string loadout = "SoldierLoadout.json") {
		PlayerBase pb_Human;
		if (!Class.CastTo(pb_Human, owner)) return null;
		
		eAIGroup ownerGrp = GetGroupByLeader(pb_Human);

		eAIBase pb_AI;
		if (!Class.CastTo(pb_AI, GetGame().CreatePlayer(null, SurvivorRandom(), pb_Human.GetPosition(), 0, "NONE"))) return null;
		if (eAIGlobal_HeadlessClient) GetRPCManager().SendRPC("eAI", "HCLinkObject", new Param1<PlayerBase>(pb_AI), false, eAIGlobal_HeadlessClient);
		
		pb_AI.SetAI(ownerGrp);
			
		HumanLoadout.Apply(pb_AI, loadout);
		Print("LEADER " + ownerGrp);
		return pb_AI;
	}
	
	eAIBase SpawnAI_Sentry(vector pos, string loadout = "SoldierLoadout.json") {
		eAIBase pb_AI;
		if (!Class.CastTo(pb_AI, GetGame().CreatePlayer(null, SurvivorRandom(), pos, 0, "NONE"))) return null;
		if (eAIGlobal_HeadlessClient) GetRPCManager().SendRPC("eAI", "HCLinkObject", new Param1<PlayerBase>(pb_AI), false, eAIGlobal_HeadlessClient);
		
		eAIGroup ownerGrp = GetGroupByLeader(pb_AI);
		
		pb_AI.SetAI(ownerGrp);
		
		if (vector.DistanceSq(pos, pb_AI.GetPosition()) > 1.0)
			pb_AI.SetPosition(pos);
			
		HumanLoadout.Apply(pb_AI, loadout);
				
		return pb_AI;
	}
	
	eAIBase SpawnAI_Patrol(vector pos, string loadout = "SoldierLoadout.json") {
		eAIBase pb_AI;
		if (!Class.CastTo(pb_AI, GetGame().CreatePlayer(null, SurvivorRandom(), pos, 0, "NONE"))) return null;
		if (eAIGlobal_HeadlessClient) GetRPCManager().SendRPC("eAI", "HCLinkObject", new Param1<PlayerBase>(pb_AI), false, eAIGlobal_HeadlessClient);
		
		eAIGroup ownerGrp = GetGroupByLeader(pb_AI);
		
		pb_AI.SetAI(ownerGrp);
		
		if (vector.DistanceSq(pos, pb_AI.GetPosition()) > 1.0)
			pb_AI.SetPosition(pos);
			
		HumanLoadout.Apply(pb_AI, loadout);
		
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(pb_AI.RequestTransition, 10000, false, "Rejoin");
				
		return pb_AI;
	}
	
	// Server Side: This RPC spawns a helper AI next to the player, and tells them to join the player's formation.
	void SpawnEntity(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<DayZPlayer> data(null);
        if (!ctx.Read(data)) return;
		
		if (IsMissionOffline()) data.param1 = GetGame().GetPlayer();
		
		if(type == CallType.Server )
		{
            Print("eAI: spawn entity RPC called.");
			SpawnAI_Helper(data.param1);
		}
	}
	
	// Client Side: Link the given AI
	void HCLinkObject(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<PlayerBase> data;
        if ( !ctx.Read( data ) ) return;
		if(type == CallType.Server) {
            Print("HC: Linking object " + data.param1);
			eAIObjectManager.Register(data.param1);
        }
	}
	
	// Client Side: Link the given AI
	void HCUnlinkObject(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<PlayerBase> data;
        if ( !ctx.Read( data ) ) return;
		if(type == CallType.Server) {
            Print("HC: Unlinking object " + data.param1);
			eAIObjectManager.Unregister(data.param1);
        }
	}
	
	// Server Side: This RPC spawns a zombie. It's actually not the right way to do it. But it's only for testing.
	// BUG: this has sometimes crashed us before. Not sure why yet.
	void SpawnZombie(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<PlayerBase> data;
        if ( !ctx.Read( data ) ) return;
		if(type == CallType.Server) {
            Print("eAI: SpawnZombie RPC called.");
			GetGame().CreateObject("ZmbF_JournalistNormal_Blue", data.param1.GetPosition() + debug_offset_2, false, true, true);
        }
	}
	
	// Server Side: Delete AI.
	void ClearAllAI(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<PlayerBase> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
            Print("eAI: ClearAllAI called.");
			foreach (eAIGroup g : eAIGroup.GROUPS) {
				for (int i = g.Count() - 1; i > -1; i--) {
					PlayerBase p = g.GetMember(i);
					if (p.IsAI()) {
						g.RemoveMember(i);
						GetGame().ObjectDelete(p);
					}
				}	
			}
		}
	}
	
	// Server Side: Delete AI.
	void ProcessReload(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<PlayerBase> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
            Print("eAI: ProcessReload called.");
			
			eAIGroup g = GetGroupByLeader(data.param1);

			for (int i = g.Count() - 1; i > -1; i--) {
				PlayerBase p = g.GetMember(i);
				Weapon_Base w = Weapon_Base.Cast(p.GetHumanInventory().GetEntityInHands());
				if (p.IsAI() && w)
					p.QuickReloadWeapon(w);
			}	

		}
	}
	
	// Client Side: This RPC spawns a debug particle at a location requested by the server.
	void DebugParticle(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param2<vector, vector> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Client ) {
			//Particle p = Particle.PlayInWorld(ParticleList.DEBUG_DOT, data.param1);
			//p.SetOrientation(data.param2);			
		}
	}
	
	// Client Side: This RPC replaces a member function of DayZPlayerInventory that handles remote weapon events. I cannot override the functionality that 
	// class, but this workaround seems to do a pretty good job.
	void DayZPlayerInventory_OnEventForRemoteWeaponAICallback(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param3<int, DayZPlayer, Magazine> data;
        if (!ctx.Read(data)) return;
		//if(type == CallType.Client ) {
			Print("Received weapon event for " + data.param1.ToString() + " player:" + data.param2.ToString() + " mag:" + data.param3.ToString());
            DayZPlayerInventory_OnEventForRemoteWeaponAI(data.param1, data.param2, data.param3);
		//}
	}
	
	// Client Side: This RPC gets the client side transformation of a Weapon_Base, then sends some data back to server
	void DebugWeaponLocation(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param2<Weapon_Base, string> data;
        if (!ctx.Read(data)) return;
		
		// Set up the model space transformation locally on the client
		vector usti_hlavne_position = data.param1.GetSelectionPositionLS("usti hlavne"); // front?
		vector konec_hlavne_position = data.param1.GetSelectionPositionLS("konec hlavne"); // back?
		vector out_front, out_back;
		out_front = data.param1.ModelToWorld(usti_hlavne_position);
		out_back = data.param1.ModelToWorld(konec_hlavne_position);
		
		// Now, sync data to server.
		GetRPCManager().SendRPC("eAI", "SpawnBullet", new Param3<Weapon_Base, vector, vector>(data.param1, out_front, out_back));
	}
	
	// Client Side: This RPC takes the weapon data like in the previous RPC, then forwards it to the ServerWeaponAimCheck RPC
	void ClientWeaponDataWithCallback(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param2<Weapon_Base, string> data;
        if (!ctx.Read(data)) return;

		if(type == CallType.Client ) {
			vector usti_hlavne_position = data.param1.GetSelectionPositionLS("usti hlavne"); // front?
			vector konec_hlavne_position = data.param1.GetSelectionPositionLS("konec hlavne"); // back?
			vector out_front, out_back;
			out_front = data.param1.ModelToWorld(usti_hlavne_position);
			out_back = data.param1.ModelToWorld(konec_hlavne_position);
		
			// Now, sync data to server.
			GetRPCManager().SendRPC("eAI", data.param2, new Param3<Weapon_Base, vector, vector>(data.param1, out_front, out_back));
		}
		else {Error("ClientWeaponDataWithCallback called wrongfully");}
	}
	
	void ReqFormRejoin(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<DayZPlayer> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
			Print("eAI: ReqFormRejoin called.");
			eAIGroup g = GetGroupByLeader(data.param1);
			for (int i = 0; i < g.Count(); i++) {
				eAIBase ai = g.GetMember(i);
				if (ai && ai.IsAI() && ai.IsAlive())
					ai.RequestTransition("Rejoin");
			}
		}
	}
	
	void ReqFormStop(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<DayZPlayer> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
			Print("eAI: ReqFormStop called.");
			eAIGroup g = GetGroupByLeader(data.param1);
			for (int i = 0; i < g.Count(); i++) {
				eAIBase ai = g.GetMember(i);
				if (ai && ai.IsAI() && ai.IsAlive())
					ai.RequestTransition("Stop");
			}
		}
	}
	
	void ReqFormationChange(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param2<DayZPlayer, int> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
			Print("eAI: ReqFormationChange called.");
			eAIGroup g = GetGroupByLeader(data.param1);
			eAIFormation newForm;
			switch (data.param2) {
				case eAICommands.FOR_VEE:
					newForm = new eAIFormationVee();
					break;
				case eAICommands.FOR_FILE:
					newForm = new eAIFormationFile();
					break;
				case eAICommands.FOR_WALL:
					newForm = new eAIFormationWall();
					break;
				case eAICommands.FOR_COL:
					newForm = new eAIFormationColumn();
					break;
				// no default needed here
			}
			g.SetFormation(newForm);
		}
	}
};

modded class MissionServer
{
    autoptr eAIGame m_eaiGame;
	PlayerIdentity m_HeadlessClient;
	
	static string HeadlessClientSteamID = "REDACTED (PUT STEAMID HERE)";
	
	override void EquipCharacter(MenuDefaultCharacterData char_data) {
		super.EquipCharacter(char_data);
		m_eaiGame.GetGroupByLeader(m_player); // This forces respawning players into a new group
	}
	
	eAIGame GetEAIGame() {
		return m_eaiGame;
	}
	
	PlayerIdentity GetHeadlessClient() {
		return m_HeadlessClient;
	}

    void MissionServer()
    {
        m_eaiGame = new eAIGame();

		GetDayZGame().eAICreateManager();

        Print( "eAI - Loaded Mission, Reading $profile/eAI directory" );
		
		MakeDirectory("$profile:eAI/");
		
		// load the settings
		g_eAISettings = new eAISettings();
		if (!FileExist("$profile:eAI/eAISettings.json")) JsonFileLoader<eAISettings>.JsonSaveFile("$profile:eAI/eAISettings.json", g_eAISettings);
		JsonFileLoader<eAISettings>.JsonLoadFile("$profile:eAI/eAISettings.json", g_eAISettings);
		
		if (!FileExist("$profile:eAI/eAIAdmins.json")) JsonFileLoader<array<string>>.JsonSaveFile("$profile:eAI/eAIAdmins.json", m_eaiGame.adminIDs);
		JsonFileLoader<array<string>>.JsonLoadFile("$profile:eAI/eAIAdmins.json", m_eaiGame.adminIDs);
		
		// load a default loadout, just to save the default if not exist
		HumanLoadout Loadout = HumanLoadout.LoadData("SoldierLoadout.json");
    }
	
	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity) {
		super.InvokeOnConnect(player, identity);
		if (identity && identity.GetId() == HeadlessClientSteamID) {
			eAIGlobal_HeadlessClient = identity;
			foreach (eAIGroup g : eAIGroup.GROUPS) {
				for (int i = 0; i < g.Count(); i++) {
					eAIBase ai = g.GetMember(i);
					if (ai && ai.IsAI() && ai.IsAlive())
						GetRPCManager().SendRPC("eAI", "HCLinkObject", new Param1<PlayerBase>(ai), false, identity);
				}
			}
				
		} else
		m_eaiGame.GetGroupByLeader(player); // This forces returning players into a new group
	}
};

modded class MissionGameplay
{
    autoptr eAIGame m_eaiGame;
	UAInput m_eAIRadialKey;

    void MissionGameplay()
    {
        m_eaiGame = new eAIGame();
		m_eAIRadialKey = GetUApi().GetInputByName("eAICommandMenu");

		GetDayZGame().eAICreateManager();

        Print( "eAI - Loaded Client Mission" );
		
		MakeDirectory("$profile:eAI/");
		
		// load the settings
		g_eAISettings = new eAISettings();
		if (!FileExist("$profile:eAI/eAISettings.json")) JsonFileLoader<eAISettings>.JsonSaveFile("$profile:eAI/eAISettings.json", g_eAISettings);
		JsonFileLoader<eAISettings>.JsonLoadFile("$profile:eAI/eAISettings.json", g_eAISettings);
    }
	
	override void OnUpdate(float timeslice) {
		super.OnUpdate(timeslice);
		
		// If we want to open the command menu, and nothing else is open
		if (m_eAIRadialKey.LocalPress() && !GetGame().GetUIManager().GetMenu()) {
			// check to see if we are an admin
			GetRPCManager().SendRPC("eAI", "ReqDebugMenu", new Param1<string>("0"), true, NULL);
	
		}
		
		// If we want to close the command menu, and our menu is open
		if (m_eAIRadialKey.LocalRelease() && GetGame().GetUIManager().GetMenu() && GetGame().GetUIManager().GetMenu() == eAICommandMenu.instance) {
			eAICommandMenu.instance.OnMenuRelease();
			GetUIManager().Back();
		}
	}
};