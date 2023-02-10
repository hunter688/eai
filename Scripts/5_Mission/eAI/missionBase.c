modded class MissionBase {
    override UIScriptedMenu CreateScriptedMenu(int id) {
        UIScriptedMenu menu = NULL;
        menu = super.CreateScriptedMenu(id);
        if (!menu) {
            switch (id) {
            case EAI_COMMAND_MENU:
                menu = new eAICommandMenu;
            break;
            }
            if (menu) {
                menu.SetID(id);
            }
        }
        return menu;
    }
}