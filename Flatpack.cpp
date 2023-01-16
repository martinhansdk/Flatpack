#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include <boost/algorithm/string/predicate.hpp>

#include <memory>
#include <sstream>
#include <string>

#include "Nester/Nester.hpp"
#include "Nester/DXFWriter.hpp"
#include "Nester/SVGWriter.hpp"

using namespace adsk::core;
using namespace adsk::fusion;
using namespace nester;
using namespace std;

Ptr<Application> app;
Ptr<UserInterface> ui;

const char* BUTTON_NAME = "FlatpackButton";
const char* PANEL_TO_USE = "SolidMakePanel";
const char* COMMAND_ID = "FlatpackCmdId";
const char* FACES_INPUT = "facesSelection";
//const char* BIN_INPUT = "binSelection";
const char* TOLERANCE_INPUT = "toleranceInput";
const char* OUTPUT_FILE_TEXT_BOX_INPUT = "outputFileTextBoxInput";
const char* OUTPUT_FILE_INPUT = "fileInput";
const char* ATTRIBUTE_GROUP = "MH-Flatpack";
const char* ATTRIBUTE_SELECTED_FACES = "ExportedFace";
//const char* ATTRIBUTE_BIN = "Bin";
const char* ATTRIBUTE_TOLERANCE = "Tolerance";
const char* ATTRIBUTE_OUTPUT_FILE = "OutputFile";

template<typename T>
Ptr<T> getSelection(Ptr<Selection> selection) {
	Ptr<T> result;
	if (selection->entity()->objectType() == BRepFace::classType()) {
		result = selection->entity();
	}
	return result;
}

bool hasEndingCaseInsensitive(std::string const &fullString, std::string const &ending) {
	if (fullString.length() >= ending.length()) {
		string substring = fullString.substr(fullString.length() - ending.length(), ending.length());
		return boost::iequals(substring, ending);
	}
	else {
		return false;
	}
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

			Ptr<Design> design = app->activeProduct();
			if (!design) {
				ui->messageBox("No active Fusion design", "No Design");
				return;
			}

			Ptr<CommandInputs> inputs = cmd->commandInputs();

			Ptr<SelectionCommandInput> selectionInput = inputs->itemById(FACES_INPUT);
			//Ptr<SelectionCommandInput> binInput = inputs->itemById(BIN_INPUT);
			Ptr<ValueCommandInput> toleranceInput = inputs->itemById(TOLERANCE_INPUT);
			Ptr<TextBoxCommandInput> filenameInput = inputs->itemById(OUTPUT_FILE_TEXT_BOX_INPUT);

			// Check that a valid tolerance was entered.
			if (!toleranceInput->isValidExpression())
			{
				// Invalid expression so display an error and set the flag to allow them
				// to enter a value again.
				ui->messageBox(toleranceInput->expression() + " is not a valid length expression.", "Invalid entry",
					OKButtonType, CriticalIconType);
				return;
			}
			double tolerance = toleranceInput->value();

			if (selectionInput == nullptr || !selectionInput->isValid()) {
				// to enter a value again.
				ui->messageBox("Face selection is not valid.", "Invalid entry",
					OKButtonType, CriticalIconType);
				return;
			}

			/*
			// save face selection for next time
			// find existing marked faces
			vector<Ptr<Attribute> > selectedFaces = design->findAttributes(ATTRIBUTE_GROUP, ATTRIBUTE_SELECTED_FACES);
			for (Ptr<Attribute> a : selectedFaces) {
				if (a->parent() != nullptr) {
					a->deleteMe();
				}
			}
			
			*/
			
			// collect all the selected faces before doing anything with them
			// this is to avoid invalidating the selection by manipulating attributes
			vector<Ptr<BRepFace> > faces(selectionInput->selectionCount());
			for (size_t i = 0; i < selectionInput->selectionCount(); i++) {
				Ptr<BRepFace> face = getSelection<BRepFace>(selectionInput->selection(i));
				if (!face) {
					ui->messageBox("Selection is not a face!");
					return;
				}
				faces[i] = face;
			}

			// iterate over the selected faces
			for(Ptr<BRepFace> face : faces) {
				face->attributes()->add(ATTRIBUTE_GROUP, ATTRIBUTE_SELECTED_FACES, "1");


				shared_ptr<NesterPart> part = make_shared<NesterPart>();
				nester.addPart(part);

				for (Ptr<BRepLoop> loop : face->loops()) {
					shared_ptr<NesterLoop> nesterLoop = make_shared<NesterLoop>();

					if (loop->isOuter()) {
						part->setOuterRing(nesterLoop);
					}
					else {
						part->addInnerRing(nesterLoop);
					}

					for (Ptr<BRepCoEdge> edge : loop->coEdges()) {
						if (!edge) {
							ui->messageBox("No edge!");
							return;
						}

						Ptr<Curve2D> curve = edge->geometry();
						Ptr<CurveEvaluator2D> curveEvaluator = curve->evaluator();

						// get range of curve parameters
						double startParameter;
						double endParameter;
						bool ok = curveEvaluator->getParameterExtents(startParameter, endParameter);
						if (!ok) {
							ui->messageBox("Failed to get parameter extents for curve!");
							return;
						}

						vector<Ptr<Point2D> > vertexCoordinates;

						ok = curveEvaluator->getStrokes(startParameter, endParameter, tolerance, vertexCoordinates);

						if (!ok) {
							ui->messageBox("Failed to get approximation of curve!");
							return;
						}

						Ptr<Point2D> previousPoint;
						for (Ptr<Point2D> point : vertexCoordinates) {
							if (previousPoint != nullptr) {
								shared_ptr<NesterLine> line = make_shared<NesterLine>();
								
								line->setStartPoint(point_t(previousPoint->x(), previousPoint->y()));
								line->setEndPoint(point_t(point->x(), point->y()));

								nesterLoop->addEdge(line);
							}
							previousPoint = point;
						}

					}
				}
			}

			// bin
			/*
			if (binInput->selectionCount() == 1) {

			ui->messageBox(binInput->selection(0)->entity()->classType());
			*/
			/*
			if (!face) {
			ui->messageBox("Selection is not a face!");
			return;
			}
			

			vector<Ptr<Attribute> > selectedBins = design->findAttributes(ATTRIBUTE_GROUP, ATTRIBUTE_BIN);
			for (Ptr<Attribute> a : selectedBins) {
			if (a->parent() != nullptr) {
			a->deleteMe();
			}
			}


			Ptr<SketchProfile>
			design->attributes()->add(ATTRIBUTE_GROUP, ATTRIBUTE_BIN, "1");
			*/
			/*
			}
			*/

			// remember the tolerance setting for next time
			// we have to do this after the selection above
			design->attributes()->add(ATTRIBUTE_GROUP, ATTRIBUTE_TOLERANCE, toleranceInput->expression());

			// write output files
			string outputFilename = filenameInput->text();
			design->attributes()->add(ATTRIBUTE_GROUP, ATTRIBUTE_OUTPUT_FILE, outputFilename);

			shared_ptr<FileWriter> writer;
			if (hasEndingCaseInsensitive(outputFilename, ".svg")) {
				writer = make_shared<SVGWriter>(outputFilename);
			}
			else {
				writer = make_shared<DXFWriter>(outputFilename);
			}
			
			nester.write(writer);
		}
	}
};

// Input validation event handler.
class OnValidateEventHandler : public adsk::core::ValidateInputsEventHandler
{
public:
	void notify(const Ptr<ValidateInputsEventArgs>& eventArgs) override
	{
		Ptr<CommandInputs> inputs = eventArgs->inputs();
		if (!inputs)
			return;

		Ptr<SelectionCommandInput> selectionInput = inputs->itemById(FACES_INPUT);
		//Ptr<SelectionCommandInput> binInput = inputs->itemById(BIN_INPUT);
		Ptr<ValueCommandInput> toleranceInput = inputs->itemById(TOLERANCE_INPUT);
		Ptr<TextBoxCommandInput> filenameInput = inputs->itemById(OUTPUT_FILE_TEXT_BOX_INPUT);

		if (selectionInput->selectionCount() == 0) {
			eventArgs->areInputsValid(false);
		}

		if (!toleranceInput->isValidExpression() || toleranceInput->value() <= 0.0) {
			eventArgs->areInputsValid(false);
		}

		if (filenameInput->text().length() <= 0) {
			eventArgs->areInputsValid(false);
		}

		eventArgs->areInputsValid(true);
	}
};

// InputChange event handler.
class OnInputChangedEventHandler : public adsk::core::InputChangedEventHandler
{
public:
	void notify(const Ptr<InputChangedEventArgs>& eventArgs) override
	{
		Ptr<CommandInputs> inputs = eventArgs->inputs();
		if (!inputs)
			return;

		Ptr<CommandInput> cmdInput = eventArgs->input();
		if (!cmdInput)
			return;

		if (cmdInput->id() == OUTPUT_FILE_INPUT) {
			Ptr<BoolValueCommandInput> button = cmdInput;
			Ptr<TextBoxCommandInput> filenameField = inputs->itemById(OUTPUT_FILE_TEXT_BOX_INPUT);

			Ptr<FileDialog> fileDialog = ui->createFileDialog();
			if (!fileDialog) {
				return;
			}

			fileDialog->isMultiSelectEnabled(false);
			fileDialog->title("Specify output file");
			fileDialog->filter("DXF format (*.dxf);;SVG format (*.svg);;All files (*.*)");
			fileDialog->filterIndex(0);
			fileDialog->initialFilename(filenameField->text());
			DialogResults dialogResult = fileDialog->showSave();
			if (DialogResults::DialogOK == dialogResult)
			{
				filenameField->text(fileDialog->filename());
			}
		}


	}
};

// CommandActivate event handler
class OnActivateEventHandler : public adsk::core::CommandEventHandler
{
public:
	void notify(const Ptr<CommandEventArgs>& eventArgs) override
	{
		Ptr<Document> doc = app->activeDocument();
		if (!doc)
			return;

		Ptr<Design> design = app->activeProduct();
		if (!design)
			return;

		Ptr<Command> cmd = eventArgs->command();

		if (cmd) {

			Ptr<Design> design = app->activeProduct();
			if (!design) {
				ui->messageBox("No active Fusion design", "No Design");
				return;
			}

			Ptr<CommandInputs> inputs = cmd->commandInputs();

			Ptr<SelectionCommandInput> selectionInput = inputs->itemById(FACES_INPUT);
			//Ptr<SelectionCommandInput> binInput = inputs->itemById(BIN_INPUT);
			Ptr<ValueCommandInput> toleranceInput = inputs->itemById(TOLERANCE_INPUT);
			Ptr<TextBoxCommandInput> filenameInput = inputs->itemById(OUTPUT_FILE_TEXT_BOX_INPUT);

			// find already selected faces
			vector<Ptr<Attribute> > selectedFaces = design->findAttributes(ATTRIBUTE_GROUP, ATTRIBUTE_SELECTED_FACES);
			for (Ptr<Attribute> a : selectedFaces) {
				if (a->parent() != nullptr) {
					selectionInput->addSelection(a->parent());
				}
			}

			/*
			vector<Ptr<Attribute> > selectedBin = design->findAttributes(ATTRIBUTE_GROUP, ATTRIBUTE_BIN);
			for (Ptr<Attribute> a : selectedBin) {
				if (a->parent() != nullptr) {
					binInput->addSelection(a->parent());
					break;
				}
			}
			*/
			Ptr<Attribute> toleranceAttribute = design->attributes()->itemByName(ATTRIBUTE_GROUP, ATTRIBUTE_TOLERANCE);
			if (toleranceAttribute != nullptr) {
				toleranceInput->expression(toleranceAttribute->value());
			}

			Ptr<Attribute> filenameAttribute = design->attributes()->itemByName(ATTRIBUTE_GROUP, ATTRIBUTE_OUTPUT_FILE);
			if (filenameAttribute != nullptr) {
				filenameInput->text(filenameAttribute->value());
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

				// Connect to the activate event.
				Ptr<CommandEvent> onActivate = command->activate();
				if (!onActivate)
					return;
				isOk = onActivate->add(&onActivateHandler);
				if (!isOk)
					return;

				// Connect to the execute event.
				Ptr<CommandEvent> onExecute = command->execute();
				if (!onExecute)
					return;
				isOk = onExecute->add(&onExecuteHandler);
				if (!isOk)
					return;

				// Connect to the input changed event.
				Ptr<InputChangedEvent> onInputChanged = command->inputChanged();
				if (!onInputChanged)
					return;
				isOk = onInputChanged->add(&onInputChangedHandler);
				if (!isOk)
					return;

				// Connect to the input validation handler
				Ptr<ValidateInputsEvent> onValidate = command->validateInputs();
				if (!onValidate)
					return;
				isOk = onValidate->add(&onValidateHandler);
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
				facesSelectionInput->tooltip("The faces to be exported.");
				facesSelectionInput->tooltipDescription("The selected faces have to be planar.");

				/*
				Ptr<SelectionCommandInput> binSelectionInput = inputs->addSelectionInput(BIN_INPUT, "Bin", "Sketch lines forming the outline of the stock material to fit the parts into.");
				if (!binSelectionInput)
					return;
				binSelectionInput->addSelectionFilter("Profiles");
				binSelectionInput->setSelectionLimits(0, 1);
				binSelectionInput->tooltip("Sketch profile describing the bin for the nesting process.");
				binSelectionInput->tooltipDescription("The nesting process tries to fit parts into the bounds of the material to be cut to produce the parts. Select a sketch profile that "
					"describes the size and shape of the material that the parts should be cut from. If a clearance to the edge of the material is wanted, draw this profile a little smaller.");
				//binSelectionInput->isVisible(false);
				*/

				Ptr<ValueCommandInput> toleranceInput = inputs->addValueInput(TOLERANCE_INPUT, "Conversion tolerance", "mm", ValueInput::createByReal(0.01));
				if (!toleranceInput)
					return;
				toleranceInput->tooltip("Accuracy of conversion to line segments.");
				toleranceInput->tooltipDescription("During the export process, the circles, arcs, ellipses, and splines are exported as straight line segments. This setting specifies the "
					"maximum distance tolerance between the ideal curve and the exported line segments. Choosing a smaller size results in more smooth "
					"curves, but at the expense of a larger output file and a longer run time.");

				// Create bool value input with button style that can be clicked.				
				Ptr<BoolValueCommandInput> button = inputs->addBoolValueInput(OUTPUT_FILE_INPUT, "Output file", false, "", true);
				button->text("Select file...");
				inputs->addTextBoxCommandInput(OUTPUT_FILE_TEXT_BOX_INPUT, "", "", 1, true);
			}
		}
	}
private:
	OnExecuteEventHander onExecuteHandler;
	OnValidateEventHandler onValidateHandler;
	OnActivateEventHandler onActivateHandler;
	OnDestroyEventHandler onDestroyHandler;
	OnInputChangedEventHandler onInputChangedHandler;
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
		"Export faces to DXF or SVG",
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
		if (cmdDef) {
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
