#include <Core/Utils.h>
#include <Core/Application/Application.h>
#include <Core/Application/Product.h>
#include <Core/Application/ValueInput.h>
#include <Core/Application/UnitsManager.h>
#include <Core/Application/ObjectCollection.h>
#include <Core/CoreTypeDefs.h>
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
#include <Core/UserInterface/Selection.h>
#include <Core/UserInterface/ToolbarPanel.h>
#include <Core/UserInterface/ToolbarPanelList.h>
#include <Core/UserInterface/ToolbarControl.h>
#include <Core/UserInterface/ToolbarControls.h>
#include <Core/UserInterface/ListItems.h>
#include <Core/Geometry/Curve2D.h>
#include <Core/Geometry/NurbsCurve2D.h>
#include <Core/Geometry/Point2D.h>
#include <Core/UserInterface/DirectionCommandInput.h>
#include <Core/UserInterface/DistanceValueCommandInput.h>
#include <Core/UserInterface/TableCommandInput.h>
#include <Core/UserInterface/AngleValueCommandInput.h>

#include <Fusion/BRep/BRepCoEdge.h>
#include <Fusion/BRep/BRepCoEdges.h>
#include <Fusion/BRep/BRepFace.h>
#include <Fusion/BRep/BRepLoop.h>
#include <Fusion/BRep/BRepLoops.h>

#include <memory>
#include <sstream>
#include <string>

#include "Nester/Nester.h"

using namespace adsk::core;
using namespace adsk::fusion;
using namespace nester;
using namespace std;

Ptr<Application> app;
Ptr<UserInterface> ui;

const char* BUTTON_NAME = "FlatpackButton";
const char* PANEL_TO_USE = "SolidScriptsAddinsPanel";
const char* COMMAND_ID = "FlatpackCmdId";
const char* FACES_INPUT = "facesSelection";
const char* BIN_INPUT = "binSelection";

template<typename T>
Ptr<T> getSelection(Ptr<Selection> selection) {
	Ptr<T> result;
	if (selection->entity()->objectType() == BRepFace::classType()) {
		result = selection->entity();
	}
	return result;
}

// CommandExecuted event handler.
class OnExecuteEventHander : public adsk::core::CommandEventHandler
{
public:
	void notify(const Ptr<CommandEventArgs>& eventArgs) override
	{
		Ptr<Command> cmd = eventArgs->command();

		if (cmd) {
			Nester nester;


			Ptr<CommandInputs> inputs = cmd->commandInputs();

			Ptr<SelectionCommandInput> selectionInput = inputs->itemById(FACES_INPUT);
			if (selectionInput) {
				for(size_t i=0 ; i < selectionInput->selectionCount() ; i++) {
					Ptr<BRepFace> face = getSelection<BRepFace>(selectionInput->selection(i));
					if (!face) {
						ui->messageBox("Selection is not a face!");
						return;
					}

					shared_ptr<NesterPart> part = make_shared<NesterPart>();
					nester.addPart(part);

					// Polygon
					std::stringstream s;
					s << "<ol>";
					for (Ptr<BRepLoop> loop : face->loops()) {
						shared_ptr<NesterLoop> nesterLoop = make_shared<NesterLoop>();
						part->addRing(nesterLoop);
						s << "<li>";
						for (Ptr<BRepCoEdge> edge : loop->coEdges()) {
							if (!edge) {
								ui->messageBox("No edge!");
								return;
							}

							Ptr<Curve2D> curve = edge->geometry();
							switch (curve->curveType()) {
							case Arc2DCurveType: 	// 	Transient 2D arc.
								ui->messageBox("Arc2DCurveType not supported");
								return;
								break;
							case Circle2DCurveType: // Transient 2D circle.
								ui->messageBox("Circle2DCurveType not supported");
								return;
								break;
							case Ellipse2DCurveType: // Transient 2D ellipse.
								ui->messageBox("Ellipse2DCurveType not supported");
								return;
								break;
							case EllipticalArc2DCurveType: // Transient 2D elliptical arc.
								ui->messageBox("EllipticalArc2DCurveType not supported");
								return;
								break;
							case InfiniteLine2DCurveType: //	Transient 2D infinite line.
								ui->messageBox("InfiniteLine2DCurveType not supported");
								return;
								break;
							case Line2DCurveType: // Transient 2D line segment.
								ui->messageBox("Line2DCurveType not supported");
								return;
								break;
							case NurbsCurve2DCurveType: // Transient 2D NURBS curve.
								Ptr<NurbsCurve2D> nurbsCurve = curve;

								shared_ptr<NesterNurbs> nurb = make_shared<NesterNurbs>();
								nesterLoop->addEdge(nurb);

								s << "<ol>";
								s << "<li>NurbsCurve2DCurveType: degree=" << nurbsCurve->degree() << " controlPoints=" << nurbsCurve->controlPointCount() << " knotcount=" << nurbsCurve->knotCount() << "</li>\n";


								for (Ptr<Point2D> point : nurbsCurve->controlPoints()) {
									//s << "(" << point->x() << ", " << point->y() << ")  ";
									nurb->addControlPoint(point->x(), point->y());
								}
								nurb->addKnots(nurbsCurve->knots());
								s << "</ol>";
								break;

							}
							
						}
						s << "</li>";
					}
					s << "</ol>";
					ui->messageBox(s.str());

					

				}
			}
		}
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

				// Connect to the execute event.
				Ptr<CommandEvent> onExecute = command->execute();
				if (!onExecute)
					return;
				isOk = onExecute->add(&onExecuteHandler);
				if (!isOk)
					return;

				// Get the CommandInputs collection associated with the command.
				Ptr<CommandInputs> inputs = command->commandInputs();
				if (!inputs)
					return;

				// Create a read only textbox input.
				//inputs->addTextBoxCommandInput("readonly_textBox", "Text Box 1", "This is an example of a read-only text box.", 2, true);

				Ptr<SelectionCommandInput> facesSelectionInput = inputs->addSelectionInput(FACES_INPUT, "Faces to export", "Faces to export");
				if (!facesSelectionInput)
					return;
				facesSelectionInput->addSelectionFilter("PlanarFaces");
				facesSelectionInput->setSelectionLimits(1);

				Ptr<SelectionCommandInput> binSelectionInput = inputs->addSelectionInput(BIN_INPUT, "Bin", "Sketch lines forming the outline of the stock material to fit the parts into");
				if (!binSelectionInput)
					return;
				binSelectionInput->addSelectionFilter("Profiles");
				binSelectionInput->setSelectionLimits(1, 1);

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
