
#include <Core/Utils.h>
#include <Core/Application/Application.h>
#include <Core/Application/Product.h>
#include <Core/Application/ValueInput.h>
#include <Core/Application/UnitsManager.h>
#include <Core/Application/ObjectCollection.h>
#include <Core/UserInterface/UserInterface.h>
#include <Core/UserInterface/CommandCreatedEventHandler.h>
#include <Core/UserInterface/CommandCreatedEvent.h>
#include <Core/UserInterface/CommandCreatedEventArgs.h>
#include <Core/UserInterface/CommandEvent.h>
#include <Core/UserInterface/CommandEventArgs.h>
#include <Core/UserInterface/CommandEventHandler.h>
#include <Core/UserInterface/Command.h>
#include <Core/UserInterface/CommandDefinition.h>
#include <Core/UserInterface/CommandDefinitions.h>
#include <Core/UserInterface/CommandInputs.h>
#include <Core/UserInterface/ValueCommandInput.h>
#include <Core/UserInterface/StringValueCommandInput.h>
#include <Core/UserInterface/SelectionCommandInput.h>
#include <Core/UserInterface/ToolbarPanel.h>
#include <Core/UserInterface/ToolbarPanelList.h>
#include <Core/UserInterface/ToolbarControl.h>
#include <Core/UserInterface/ToolbarControls.h>
#include <Core/UserInterface/ListItems.h>
#include <Core/Geometry/Point3D.h>
#include <Core/Geometry/Vector3D.h>
#include <Core/UserInterface/DirectionCommandInput.h>
#include <Core/UserInterface/DistanceValueCommandInput.h>
#include <Core/UserInterface/TableCommandInput.h>
#include <Core/UserInterface/AngleValueCommandInput.h>

#include <sstream>

using namespace adsk::core;
using namespace adsk::fusion;

Ptr<Application> app;
Ptr<UserInterface> ui;

const char* BUTTON_NAME = "FlatpackButton";
const char* PANEL_TO_USE = "SolidScriptsAddinsPanel";
const char* COMMAND_ID = "FlatpackCmdId";




// CommandExecuted event handler.
class OnExecuteEventHander : public adsk::core::CommandEventHandler
{
public:
	void notify(const Ptr<CommandEventArgs>& eventArgs) override
	{

	}
};

// CommandDestroyed event handler
class OnDestroyEventHandler : public adsk::core::CommandEventHandler
{
public:
	void notify(const Ptr<CommandEventArgs>& eventArgs) override
	{
		
	}
};

// CommandCreated event handler.
class CommandCreatedEventHandler : public adsk::core::CommandCreatedEventHandler
{
public:
	void notify(const Ptr<CommandCreatedEventArgs>& eventArgs) override
	{
		if (eventArgs)
		{
			// Get the command that was created.
			Ptr<Command> command = eventArgs->command();
			if (command)
			{
				// Connect to the command destroyed event.
				Ptr<CommandEvent> onDestroy = command->destroy();
				if (!onDestroy)
					return;
				bool isOk = onDestroy->add(&onDestroyHandler);
				if (!isOk)
					return;

				// Get the CommandInputs collection associated with the command.
				Ptr<CommandInputs> inputs = command->commandInputs();
				if (!inputs)
					return;

				// Create a read only textbox input.
				//inputs->addTextBoxCommandInput("readonly_textBox", "Text Box 1", "This is an example of a read-only text box.", 2, true);

				Ptr<SelectionCommandInput> facesSelectionInput = inputs->addSelectionInput("facesSelection", "Faces to export", "Faces to export");
				if (!facesSelectionInput)
					return;
				facesSelectionInput->addSelectionFilter("PlanarFaces");
				facesSelectionInput->setSelectionLimits(0);

				Ptr<SelectionCommandInput> binSelectionInput = inputs->addSelectionInput("binSelection", "Bin", "Sketch lines forming the outline of the stock material to fit the parts into");
				if (!binSelectionInput)
					return;
				binSelectionInput->addSelectionFilter("Profiles");
				binSelectionInput->setSelectionLimits(1);

				// Create value input.
				//inputs->addValueInput("value", "Value", "cm", ValueInput::createByReal(0.0));

			}
		}
	}
private:
	OnExecuteEventHander onExecuteHandler;
	OnDestroyEventHandler onDestroyHandler;
} _cmdCreatedHandler;




 extern "C" XI_EXPORT bool run(const char* context)
 {
	 app = Application::get();
	 if (!app)
		 return false;

	 ui = app->userInterface();
	 if (!ui)
		 return false;

	 // Create a button command definition.
	 Ptr<CommandDefinitions> cmdDefs = ui->commandDefinitions();
	 Ptr<CommandDefinition> cmdDef = cmdDefs->addButtonDefinition(COMMAND_ID,
		 "Export faces to DXF",
		 "");

	 Ptr<ToolbarPanel> addinsPanel = ui->allToolbarPanels()->itemById(PANEL_TO_USE);
	 Ptr<ToolbarControl> cntrl = addinsPanel->controls()->itemById(COMMAND_ID);
	 if (!cntrl) {
		 addinsPanel->controls()->addCommand(cmdDef);
	 }


	 // Connect to the Command Created event.
	 Ptr<CommandCreatedEvent> commandCreatedEvent = cmdDef->commandCreated();
	 commandCreatedEvent->add(&_cmdCreatedHandler);


	 return true;
 }




extern "C" XI_EXPORT bool stop(const char* context)
{
	if (ui)
	{
		// Create the command definition.
		Ptr<CommandDefinitions> commandDefinitions = ui->commandDefinitions();

		if (!commandDefinitions) {
			ui = nullptr;
			return false;
		}


		// Clean up the UI.
		
		Ptr<CommandDefinition> cmdDef = commandDefinitions->itemById(COMMAND_ID);
		if(cmdDef) {
			cmdDef->deleteMe();
		}
		else
			ui->messageBox("unable to find cmdDef to delete it!");

		Ptr<ToolbarPanel> addinsPanel = ui->allToolbarPanels()->itemById(PANEL_TO_USE);
		Ptr<ToolbarControl> cntrl = addinsPanel->controls()->itemById(COMMAND_ID);	
		if (cntrl) {
			cntrl->deleteMe();
		}
		else
			ui->messageBox("unable to find cntrl to delete it!");
		
		ui = nullptr;
		return true;
	}

	return true;
}



#ifdef XI_WIN

#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hmodule, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

#endif // XI_WIN
