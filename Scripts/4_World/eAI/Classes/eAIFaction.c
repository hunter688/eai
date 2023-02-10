class eAIFaction {
	protected string name;
	string getName() {return name;}
	// Return true if another faction should be considered friendly to us.
	bool isFriendly(eAIFaction other) {
		return true;
	}
	//This probably should be an enum, but it is used for bodyguard behavior.
	// Return -1 for protect
	// Return 0 for do not target 
	// Return 1 for aim, but do not fire.
	// Return 2 for Open Fire 
	int confirmKill(PlayerBase target) {
		return 2;
	}
};

class eAIFactionRaiders : eAIFaction { // this is the faction that seeks the Lost Ark
	void eAIFactionRaiders() {name = "Raiders";}
	override bool isFriendly(eAIFaction other) {
		// Raiders view any group that is not them as enemy.
		return false;
	}
};

class eAIFactionGuards : eAIFaction {
	void eAIFactionGuards() {name = "Guards";}
	override bool isFriendly(eAIFaction other) {
		// "friendly" in this case just guarantees that they won't shoot at other guards with their weapons up
		if (other.getName() == "Guards") return true;
		return false;
	}
	override int confirmKill(PlayerBase target) {
		HumanInventory inv = target.GetHumanInventory();
		if (inv) {
			EntityAI ent = inv.GetEntityInHands();
			if (Weapon_Base.Cast(ent)) {
				if (HumanInputController.Cast(target.GetInputController()).IsWeaponRaised()) return 2;
				return 1;
			} else if (ToolBase.Cast(ent)) {
				if (HumanInputController.Cast(target.GetInputController()).IsWeaponRaised()) return 2;
				return 1;
			}
		}
		return 0;
	}
};

class eAIFactionWest : eAIFaction {
	void eAIFactionWest() {name = "West";}
	override bool isFriendly(eAIFaction other) {
		if (other.getName() == "West") return true;
		if (other.getName() == "Civilian") return true;
		return false;
	}
};

class eAIFactionEast : eAIFaction {
	void eAIFactionEast() {name = "Raiders";}
	override bool isFriendly(eAIFaction other) {
		if (other.getName() == "Raiders") return true;
		if (other.getName() == "Civilian") return true;
		return false;
	}
};

class eAIFactionCivilian : eAIFaction {
	void eAIFactionCivilian() {name = "Civilian";}
	override bool isFriendly(eAIFaction other) {
		return true;
	}
};