class eAICommandManager {
	bool Send(eAICommands cmd) {}
}

// This class handles the inputs from the eAICommandMenu locally and shoots RPCs to the server.
class eAICommandManagerClient : eAICommandManager {
	override bool Send(eAICommands cmd) {
		switch (cmd) {
			case eAICommands.DEB_SPAWNALLY:
				GetRPCManager().SendRPC("eAI", "SpawnEntity", new Param1<DayZPlayer>(GetGame().GetPlayer()));
				return true;
			
			case eAICommands.DEB_CLEARALL:
				GetRPCManager().SendRPC("eAI", "ClearAllAI", new Param1<DayZPlayer>(GetGame().GetPlayer()));
				return true;
			
			case eAICommands.DEB_RELOAD:
				GetRPCManager().SendRPC("eAI", "ProcessReload", new Param1<DayZPlayer>(GetGame().GetPlayer()));
				return true;
			
			case eAICommands.DEB_SPAWNZOM:
				GetRPCManager().SendRPC("eAI", "SpawnZombie", new Param1<DayZPlayer>(GetGame().GetPlayer()));
				return true;
			
		} return false;
	}
};