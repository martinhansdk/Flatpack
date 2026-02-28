#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include <algorithm>
#include <cctype>
#include <memory>
#include <sstream>
#include <string>

#ifdef XI_WIN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "Nester/DXFWriter.hpp"
#include "Nester/Nester.hpp"
#include "Nester/SVGWriter.hpp"

using namespace adsk::core;
using namespace adsk::fusion;
using namespace nester;
using namespace std;

Ptr<Application> app;
Ptr<UserInterface> ui;

// Directory containing the add-in DLL (with trailing path separator).
// Set once in run(), used to locate FlatpackPreview.html.
static string addinDir;
const char *PREVIEW_PALETTE_ID = "FlatpackPreviewPalette";

const char *BUTTON_NAME = "FlatpackButton";
const char *COMMAND_ID = "FlatpackCmdId";
const char *PREFERRED_PANEL = "MakePanel";  // Utilities > Make
const char *FALLBACK_PANEL = "AddInsPanel"; // Add-Ins panel
const char *FACES_INPUT = "facesSelection";
// const char* BIN_INPUT = "binSelection";
const char *TOLERANCE_INPUT = "toleranceInput";
const char *OUTPUT_FILE_TEXT_BOX_INPUT = "outputFileTextBoxInput";
const char *OUTPUT_FILE_INPUT = "fileInput";
const char *ATTRIBUTE_GROUP = "MH-Flatpack";
const char *ATTRIBUTE_SELECTED_FACES = "ExportedFace";
// const char* ATTRIBUTE_BIN = "Bin";
const char *ATTRIBUTE_TOLERANCE = "Tolerance";
const char *ATTRIBUTE_OUTPUT_FILE = "OutputFile";
const char *KERF_INPUT = "kerfInput";
const char *ATTRIBUTE_KERF = "Kerf";
const char *LAYOUT_INPUT = "layoutInput";
const char *ATTRIBUTE_LAYOUT = "Layout";

template <typename T> Ptr<T> getSelection(Ptr<Selection> selection) {
    Ptr<T> result;
    if (selection->entity()->objectType() == BRepFace::classType()) {
        result = selection->entity();
    }
    return result;
}

bool hasEndingCaseInsensitive(std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        auto it1 = fullString.end() - ending.length();
        auto it2 = ending.begin();
        return std::equal(it1, fullString.end(), it2,
                          [](char a, char b) { return std::tolower(a) == std::tolower(b); });
    } else {
        return false;
    }
}

// CommandExecuted event handler.
class OnExecuteEventHander : public adsk::core::CommandEventHandler {
  public:
    void notify(const Ptr<CommandEventArgs> &eventArgs) override {
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
            // Ptr<SelectionCommandInput> binInput = inputs->itemById(BIN_INPUT);
            Ptr<ValueCommandInput> toleranceInput = inputs->itemById(TOLERANCE_INPUT);
            Ptr<ValueCommandInput> kerfInput = inputs->itemById(KERF_INPUT);
            Ptr<TextBoxCommandInput> filenameInput = inputs->itemById(OUTPUT_FILE_TEXT_BOX_INPUT);

            // Check that a valid tolerance was entered.
            if (!toleranceInput->isValidExpression()) {
                // Invalid expression so display an error and set the flag to allow them
                // to enter a value again.
                ui->messageBox(toleranceInput->expression() + " is not a valid length expression.",
                               "Invalid entry", OKButtonType, CriticalIconType);
                return;
            }
            double tolerance = toleranceInput->value();

            if (selectionInput == nullptr || !selectionInput->isValid()) {
                // to enter a value again.
                ui->messageBox("Face selection is not valid.", "Invalid entry", OKButtonType,
                               CriticalIconType);
                return;
            }

            /*
            // save face selection for next time
            // find existing marked faces
            vector<Ptr<Attribute> > selectedFaces = design->findAttributes(ATTRIBUTE_GROUP,
            ATTRIBUTE_SELECTED_FACES); for (Ptr<Attribute> a : selectedFaces) { if (a->parent() !=
            nullptr) { a->deleteMe();
                }
            }

            */

            // collect all the selected faces before doing anything with them
            // this is to avoid invalidating the selection by manipulating attributes
            vector<Ptr<BRepFace>> faces(selectionInput->selectionCount());
            for (size_t i = 0; i < selectionInput->selectionCount(); i++) {
                Ptr<BRepFace> face = getSelection<BRepFace>(selectionInput->selection(i));
                if (!face) {
                    ui->messageBox("Selection is not a face!");
                    return;
                }
                faces[i] = face;
            }

            // iterate over the selected faces
            for (Ptr<BRepFace> face : faces) {
                face->attributes()->add(ATTRIBUTE_GROUP, ATTRIBUTE_SELECTED_FACES, "1");

                shared_ptr<NesterPart> part = make_shared<NesterPart>();
                nester.addPart(part);

                for (Ptr<BRepLoop> loop : face->loops()) {
                    shared_ptr<NesterLoop> nesterLoop = make_shared<NesterLoop>();

                    if (loop->isOuter()) {
                        part->setOuterRing(nesterLoop);
                    } else {
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

                        vector<Ptr<Point2D>> vertexCoordinates;

                        ok = curveEvaluator->getStrokes(startParameter, endParameter, tolerance,
                                                        vertexCoordinates);

                        if (!ok) {
                            ui->messageBox("Failed to get approximation of curve!");
                            return;
                        }

                        Ptr<Point2D> previousPoint;
                        for (Ptr<Point2D> point : vertexCoordinates) {
                            if (previousPoint != nullptr) {
                                shared_ptr<NesterLine> line = make_shared<NesterLine>();

                                line->setStartPoint(
                                    point_t(previousPoint->x(), previousPoint->y()));
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


            vector<Ptr<Attribute> > selectedBins = design->findAttributes(ATTRIBUTE_GROUP,
            ATTRIBUTE_BIN); for (Ptr<Attribute> a : selectedBins) { if (a->parent() != nullptr) {
            a->deleteMe();
            }
            }


            Ptr<SketchProfile>
            design->attributes()->add(ATTRIBUTE_GROUP, ATTRIBUTE_BIN, "1");
            */
            /*
            }
            */

            Ptr<DropDownCommandInput> layoutInput = inputs->itemById(LAYOUT_INPUT);
            bool packTightly = !layoutInput || !layoutInput->selectedItem() ||
                               layoutInput->selectedItem()->index() == 0;

            // remember the tolerance setting for next time
            // we have to do this after the selection above
            design->attributes()->add(ATTRIBUTE_GROUP, ATTRIBUTE_TOLERANCE,
                                      toleranceInput->expression());
            design->attributes()->add(ATTRIBUTE_GROUP, ATTRIBUTE_KERF, kerfInput->expression());
            design->attributes()->add(ATTRIBUTE_GROUP, ATTRIBUTE_LAYOUT, packTightly ? "0" : "1");

            // write output files
            string outputFilename = filenameInput->text();
            design->attributes()->add(ATTRIBUTE_GROUP, ATTRIBUTE_OUTPUT_FILE, outputFilename);

            shared_ptr<FileWriter> writer;
            if (hasEndingCaseInsensitive(outputFilename, ".svg")) {
                writer = make_shared<SVGWriter>(outputFilename);
            } else {
                writer = make_shared<DXFWriter>(outputFilename);
            }

            nester.setKerf(kerfInput->value());
            if (packTightly) {
                // Open (or reuse) the preview palette.
                Ptr<Palette> previewPalette = ui->palettes()->itemById(PREVIEW_PALETTE_ID);
                if (!previewPalette && !addinDir.empty()) {
#ifdef XI_WIN
                    string htmlUrl = "file:///" + addinDir + "FlatpackPreview.html";
#else
                    string htmlUrl = "file://" + addinDir + "FlatpackPreview.html";
#endif
                    previewPalette = ui->palettes()->add(PREVIEW_PALETTE_ID, "Flatpack Preview",
                                                         htmlUrl, true, true, true, 460, 560);
                } else if (previewPalette) {
                    previewPalette->isVisible(true);
                }

                // Writes the current best layout to flatpack_preview.svg so the
                // palette's JavaScript can poll and display it.  Uses an atomic
                // write (tmp + rename) to avoid the palette reading a partial file.
                auto writePreview = [&]() {
                    if (addinDir.empty())
                        return;
                    string finalPath = addinDir + "flatpack_preview.svg";
                    string tmpPath = finalPath + ".tmp";
                    auto svgWriter = make_shared<SVGStringWriter>();
                    nester.write(svgWriter);
                    {
                        ofstream f(tmpPath);
                        f << svgWriter->toString();
                    }
#ifdef XI_WIN
                    MoveFileExA(tmpPath.c_str(), finalPath.c_str(),
                                MOVEFILE_REPLACE_EXISTING);
#else
                    rename(tmpPath.c_str(), finalPath.c_str());
#endif
                };

                Ptr<ProgressDialog> prog = ui->createProgressDialog();
                prog->cancelButtonText("Cancel");
                prog->show("Flatpack", "Optimising layout...", 0, 1000, 1);
                nester.run([&](int current, int total) -> bool {
                    prog->progressValue(current);
                    // Update preview every 50 iterations (~20 refreshes total).
                    if (current % 50 == 0)
                        writePreview();
                    return !prog->wasCancelled();
                });
                prog->hide();
                writePreview(); // final state
            }

            // Safety check: verify the layout before writing.
            // An error here indicates a bug in the nesting algorithm.
            auto validationErrors = nester.validate();
            if (!validationErrors.empty()) {
                string msg = "Layout validation failed — output not written.\n\n";
                for (const auto &e : validationErrors)
                    msg += "\u2022 " + e + "\n";
                msg += "\nPlease report this at github.com/your-repo/Flatpack/issues.";
                ui->messageBox(msg, "Flatpack error");
                return;
            }

            nester.write(writer);
        }
    }
};

// Input validation event handler.
class OnValidateEventHandler : public adsk::core::ValidateInputsEventHandler {
  public:
    void notify(const Ptr<ValidateInputsEventArgs> &eventArgs) override {
        Ptr<CommandInputs> inputs = eventArgs->inputs();
        if (!inputs)
            return;

        Ptr<SelectionCommandInput> selectionInput = inputs->itemById(FACES_INPUT);
        // Ptr<SelectionCommandInput> binInput = inputs->itemById(BIN_INPUT);
        Ptr<ValueCommandInput> toleranceInput = inputs->itemById(TOLERANCE_INPUT);
        Ptr<TextBoxCommandInput> filenameInput = inputs->itemById(OUTPUT_FILE_TEXT_BOX_INPUT);

        if (selectionInput->selectionCount() == 0) {
            eventArgs->areInputsValid(false);
        }

        if (!toleranceInput->isValidExpression() || toleranceInput->value() <= 0.0) {
            eventArgs->areInputsValid(false);
        }

        Ptr<ValueCommandInput> kerfInput = inputs->itemById(KERF_INPUT);
        if (!kerfInput || !kerfInput->isValidExpression() || kerfInput->value() < 0.0) {
            eventArgs->areInputsValid(false);
        }

        if (filenameInput->text().length() <= 0) {
            eventArgs->areInputsValid(false);
        }

        eventArgs->areInputsValid(true);
    }
};

// InputChange event handler.
class OnInputChangedEventHandler : public adsk::core::InputChangedEventHandler {
  public:
    void notify(const Ptr<InputChangedEventArgs> &eventArgs) override {
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
            if (DialogResults::DialogOK == dialogResult) {
                filenameField->text(fileDialog->filename());
            }
        }
    }
};

// CommandActivate event handler
class OnActivateEventHandler : public adsk::core::CommandEventHandler {
  public:
    void notify(const Ptr<CommandEventArgs> &eventArgs) override {
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
            // Ptr<SelectionCommandInput> binInput = inputs->itemById(BIN_INPUT);
            Ptr<ValueCommandInput> toleranceInput = inputs->itemById(TOLERANCE_INPUT);
            Ptr<TextBoxCommandInput> filenameInput = inputs->itemById(OUTPUT_FILE_TEXT_BOX_INPUT);

            // find already selected faces
            vector<Ptr<Attribute>> selectedFaces =
                design->findAttributes(ATTRIBUTE_GROUP, ATTRIBUTE_SELECTED_FACES);
            for (Ptr<Attribute> a : selectedFaces) {
                if (a->parent() != nullptr) {
                    selectionInput->addSelection(a->parent());
                }
            }

            /*
            vector<Ptr<Attribute> > selectedBin = design->findAttributes(ATTRIBUTE_GROUP,
            ATTRIBUTE_BIN); for (Ptr<Attribute> a : selectedBin) { if (a->parent() != nullptr) {
                    binInput->addSelection(a->parent());
                    break;
                }
            }
            */
            Ptr<Attribute> toleranceAttribute =
                design->attributes()->itemByName(ATTRIBUTE_GROUP, ATTRIBUTE_TOLERANCE);
            if (toleranceAttribute != nullptr) {
                toleranceInput->expression(toleranceAttribute->value());
            }

            Ptr<ValueCommandInput> kerfInput = inputs->itemById(KERF_INPUT);
            Ptr<Attribute> kerfAttribute =
                design->attributes()->itemByName(ATTRIBUTE_GROUP, ATTRIBUTE_KERF);
            if (kerfAttribute != nullptr && kerfInput) {
                kerfInput->expression(kerfAttribute->value());
            }

            Ptr<DropDownCommandInput> layoutInput = inputs->itemById(LAYOUT_INPUT);
            Ptr<Attribute> layoutAttribute =
                design->attributes()->itemByName(ATTRIBUTE_GROUP, ATTRIBUTE_LAYOUT);
            if (layoutAttribute != nullptr && layoutInput) {
                int idx = (layoutAttribute->value() == "1") ? 1 : 0;
                layoutInput->listItems()->item(idx)->isSelected(true);
            }

            Ptr<Attribute> filenameAttribute =
                design->attributes()->itemByName(ATTRIBUTE_GROUP, ATTRIBUTE_OUTPUT_FILE);
            if (filenameAttribute != nullptr) {
                filenameInput->text(filenameAttribute->value());
            }
        }
    }
};

// CommandDestroyed event handler
class OnDestroyEventHandler : public adsk::core::CommandEventHandler {
  public:
    void notify(const Ptr<CommandEventArgs> &eventArgs) override {}
};

// CommandCreated event handler.
class CommandCreatedEventHandler : public adsk::core::CommandCreatedEventHandler {
  public:
    void notify(const Ptr<CommandCreatedEventArgs> &eventArgs) override {
        if (eventArgs) {

            // Get the command that was created.
            Ptr<Command> command = eventArgs->command();
            if (command) {
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
                // inputs->addTextBoxCommandInput("readonly_textBox", "Text Box 1", "This is an
                // example of a read-only text box.", 2, true);

                Ptr<SelectionCommandInput> facesSelectionInput =
                    inputs->addSelectionInput(FACES_INPUT, "Faces to export", "Faces to export");
                if (!facesSelectionInput)
                    return;
                facesSelectionInput->addSelectionFilter("PlanarFaces");
                facesSelectionInput->setSelectionLimits(1);
                facesSelectionInput->tooltip("The faces to be exported.");
                facesSelectionInput->tooltipDescription("The selected faces have to be planar.");

                /*
                Ptr<SelectionCommandInput> binSelectionInput = inputs->addSelectionInput(BIN_INPUT,
                "Bin", "Sketch lines forming the outline of the stock material to fit the parts
                into."); if (!binSelectionInput) return;
                binSelectionInput->addSelectionFilter("Profiles");
                binSelectionInput->setSelectionLimits(0, 1);
                binSelectionInput->tooltip("Sketch profile describing the bin for the nesting
                process."); binSelectionInput->tooltipDescription("The nesting process tries to fit
                parts into the bounds of the material to be cut to produce the parts. Select a
                sketch profile that " "describes the size and shape of the material that the parts
                should be cut from. If a clearance to the edge of the material is wanted, draw this
                profile a little smaller.");
                //binSelectionInput->isVisible(false);
                */

                Ptr<DropDownCommandInput> layoutInput = inputs->addDropDownCommandInput(
                    LAYOUT_INPUT, "Layout", DropDownStyles::TextListDropDownStyle);
                if (!layoutInput)
                    return;
                layoutInput->listItems()->add("Pack parts tightly", true);
                layoutInput->listItems()->add("Place parts on a line", false);

                Ptr<ValueCommandInput> toleranceInput = inputs->addValueInput(
                    TOLERANCE_INPUT, "Conversion tolerance", "mm", ValueInput::createByReal(0.01));
                if (!toleranceInput)
                    return;
                toleranceInput->tooltip("Accuracy of conversion to line segments.");
                toleranceInput->tooltipDescription(
                    "During the export process, the circles, arcs, ellipses, and splines are "
                    "exported as straight line segments. This setting specifies the "
                    "maximum distance tolerance between the ideal curve and the exported line "
                    "segments. Choosing a smaller size results in more smooth "
                    "curves, but at the expense of a larger output file and a longer run time.");

                Ptr<ValueCommandInput> kerfInput = inputs->addValueInput(
                    KERF_INPUT, "Kerf (part separation)", "mm", ValueInput::createByReal(0.01));
                if (!kerfInput)
                    return;
                kerfInput->tooltip("Minimum gap between nested parts.");

                // Create bool value input with button style that can be clicked.
                Ptr<BoolValueCommandInput> button =
                    inputs->addBoolValueInput(OUTPUT_FILE_INPUT, "Output file", false, "", true);
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

extern "C" XI_EXPORT bool run(const char *context) {
    app = Application::get();
    if (!app)
        return false;

    ui = app->userInterface();
    if (!ui)
        return false;

    // Find the directory containing this DLL/dylib so we can locate FlatpackPreview.html.
    // We use a static local as an anchor address — it is guaranteed to be in this module.
    {
        static int anchor;
#ifdef XI_WIN
        HMODULE hModule = NULL;
        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                   GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               (LPCSTR)&anchor, &hModule)) {
            char buf[MAX_PATH];
            if (GetModuleFileNameA(hModule, buf, MAX_PATH)) {
                string path(buf);
                replace(path.begin(), path.end(), '\\', '/');
                addinDir = path.substr(0, path.rfind('/') + 1);
            }
        }
#else
        Dl_info info;
        if (dladdr((void *)&anchor, &info) != 0) {
            string path(info.dli_fname);
            addinDir = path.substr(0, path.rfind('/') + 1);
        }
#endif
    }

    // Create a button command definition.
    Ptr<CommandDefinitions> cmdDefs = ui->commandDefinitions();
    Ptr<CommandDefinition> cmdDef =
        cmdDefs->addButtonDefinition(COMMAND_ID, "Export faces to DXF or SVG", "");

    // Try to add to preferred panel first
    Ptr<ToolbarPanel> targetPanel = ui->allToolbarPanels()->itemById(PREFERRED_PANEL);
    bool usedFallback = false;

    // If preferred panel not found, try fallback
    if (!targetPanel) {
        targetPanel = ui->allToolbarPanels()->itemById(FALLBACK_PANEL);
        usedFallback = true;
    }

    // Add the command to the panel
    if (targetPanel) {
        Ptr<ToolbarControl> cntrl = targetPanel->controls()->itemById(COMMAND_ID);
        if (!cntrl) {
            targetPanel->controls()->addCommand(cmdDef);
        }

        // Warn if using fallback location
        if (usedFallback) {
            ui->messageBox("Flatpack: The expected menu location (Utilities > Make) was not found. "
                           "The add-in has been placed in the Add-Ins menu instead.\n\n"
                           "This may indicate a change in Fusion 360's menu structure. "
                           "Please report this at: https://github.com/martinhansdk/Flatpack/issues",
                           "Flatpack Menu Location Warning");
        }
    } else {
        // No panel found - show error
        ui->messageBox(
            "Flatpack: Unable to find a suitable menu location. "
            "The command is registered but may not be accessible from the UI.\n\n"
            "Please report this issue at: https://github.com/martinhansdk/Flatpack/issues",
            "Flatpack Installation Error");
    }

    // Connect to the Command Created event.
    Ptr<CommandCreatedEvent> commandCreatedEvent = cmdDef->commandCreated();
    commandCreatedEvent->add(&_cmdCreatedHandler);

    return true;
}

extern "C" XI_EXPORT bool stop(const char *context) {
    if (ui) {
        // Remove the preview palette if it exists.
        Ptr<Palette> previewPalette = ui->palettes()->itemById(PREVIEW_PALETTE_ID);
        if (previewPalette)
            previewPalette->deleteMe();

        // Clean up the temporary preview SVG file.
        if (!addinDir.empty())
            remove((addinDir + "flatpack_preview.svg").c_str());

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

        // Try to remove from preferred panel
        Ptr<ToolbarPanel> panel = ui->allToolbarPanels()->itemById(PREFERRED_PANEL);
        if (panel) {
            Ptr<ToolbarControl> cntrl = panel->controls()->itemById(COMMAND_ID);
            if (cntrl) {
                cntrl->deleteMe();
            }
        }

        // Try to remove from fallback panel
        panel = ui->allToolbarPanels()->itemById(FALLBACK_PANEL);
        if (panel) {
            Ptr<ToolbarControl> cntrl = panel->controls()->itemById(COMMAND_ID);
            if (cntrl) {
                cntrl->deleteMe();
            }
        }

        ui = nullptr;
        return true;
    }

    return true;
}

#ifdef XI_WIN

BOOL APIENTRY DllMain(HMODULE hmodule, DWORD reason, LPVOID reserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

#endif // XI_WIN
