
#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <CAM/CAMAll.h>

#include <fstream>
#include <string>
#include <filesystem>
#include <ctime>

using namespace adsk::core;
using namespace adsk::fusion;
using namespace adsk::cam;

Ptr<Application> app;
Ptr<UserInterface> ui;

Ptr<BRepFace> selectedFace;

Ptr<Sketch> sketch1;
Ptr<Sketch> sketch2;

Ptr<ValueCommandInput> pieceSizeInput;
Ptr<ValueCommandInput> toleranceInput;
double pieceSizeVal = 20;
double toleranceVal = .3;

Ptr<SelectionCommandInput> faceSelectionInput;

class SVGWriter {
private:
	std::string filepath;
	std::ofstream filestream;
public:
	SVGWriter(std::string filename, double width, double height);

	void createLine(double x1, double y1, double x2, double y2);
	void createCurve(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);
	void createArc(double x1, double y1, double x2, double y2, double radius, double dir);

	std::string getFilePath();
	void endWriting();
};

SVGWriter::SVGWriter(std::string filename, double width, double height) {
	std::string appdata = getenv("APPDATA");
	filestream.open(appdata + '\\' + filename, std::ios::out);
	filepath = appdata + '\\' + filename;
	filestream << "<svg width=\"" + std::to_string(width) + "\" height=\"" + std::to_string(height) + "\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n";
}

void SVGWriter::createLine(double x1, double y1, double x2, double y2) {
	filestream << "<line x1 = \"" + std::to_string(x1) + "\" y1 = \"" + std::to_string(y1) + "\" x2 = \"" + std::to_string(x2) + "\" y2 = \"" + std::to_string(y2) + "\"/>\n";
}

void SVGWriter::createCurve(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4) {
	filestream << "<path d=\"M " + std::to_string(x1) + ' ' + std::to_string(y1) + " C " + std::to_string(x2) + ' ' + std::to_string(y2) + ", " + std::to_string(x3) + ' ' + std::to_string(y3) + ", " + std::to_string(x4) + ' ' + std::to_string(y4) + "\"/>\n";
}

void SVGWriter::createArc(double x1, double y1, double x2, double y2, double radius, double dir) {
	//M ax ay A rx ry x-axis-rotation large-arc-flag sweep-flag bx by
	filestream << "<path d=\"M " + std::to_string(x1) + ' ' + std::to_string(y1) + " A " + std::to_string(radius) + ' ' + std::to_string(radius) + " 0 1 " + std::to_string(dir) + ' ' + std::to_string(x2) + ' ' + std::to_string(y2) + "\"/>\n";
}

std::string SVGWriter::getFilePath() {
	return filepath;
}

void SVGWriter::endWriting() {
	filestream << "</svg>\n";
	filestream.close();
}

SVGWriter drawPuzzle(Ptr<Point3D> min, Ptr<Point3D> max) {

	double xlen = abs((10 * (max->x() - min->x())) / .26458);
	double ylen = abs((10 * (max->y() - min->y())) / .26458);

	int numx = abs(10 * (max->x() - min->x())) / pieceSizeVal;
	int numy = abs(10 * (max->y() - min->y())) / pieceSizeVal;
	double xoffset = xlen / numx;
	double yoffset = ylen / numy;
	double tolerance = toleranceVal / .26458;
	double tolerance2 = tolerance * 2;

	srand(std::time(NULL));

	SVGWriter svg("puzzle.svg", xlen, ylen);

	// X lines
	for (int i = 1; i < numx; ++i) {
		for (int j = 0; j < numy; ++j) {
			double xpos = xoffset * i;
			double tolxpos = xpos + tolerance;
			double ypos = yoffset * j;
			double point0 = ypos;
			double point1 = ypos + (yoffset / 2.5);
			double point2 = ypos + (yoffset - (yoffset / 2.5));
			double point3 = ypos + yoffset;
			double radius = xoffset / 6.0;

			// If this does not border an edge, adjust point0 val
			if (j != 0) point0 += tolerance;

			if ((2 * radius) + tolerance < point2 - point1) radius = ((point2 - point1) / 2.0) + tolerance2;

			int dir = rand() % 2;


			if (dir == 0) {
				svg.createLine(xpos, point0, xpos, point1);
				svg.createLine(tolxpos, point0, tolxpos, point1 + tolerance2);

				svg.createArc(xpos, point1 + tolerance2, xpos, point2 - tolerance2, radius - tolerance, 0);
				svg.createArc(xpos, point1, xpos, point2, radius, 0);

				svg.createLine(xpos, point1 + tolerance2, tolxpos, point1 + tolerance2);
				svg.createLine(xpos, point2 - tolerance2, tolxpos, point2 - tolerance2);

				svg.createLine(xpos, point2, xpos, point3);
				svg.createLine(tolxpos, point2 - tolerance2, tolxpos, point3);
			}
			else if (dir == 1) {
				svg.createLine(xpos, point0, xpos, point1 + tolerance2);
				svg.createLine(tolxpos, point0, tolxpos, point1);

				svg.createArc(tolxpos, point1 + tolerance2, tolxpos, point2 - tolerance2, radius - tolerance, 1);
				svg.createArc(tolxpos, point1, tolxpos, point2, radius, 1);

				svg.createLine(xpos, point1 + tolerance2, tolxpos, point1 + tolerance2);
				svg.createLine(xpos, point2 - tolerance2, tolxpos, point2 - tolerance2);

				svg.createLine(xpos, point2 - tolerance2, xpos, point3);
				svg.createLine(tolxpos, point2, tolxpos, point3);
			}

		}

	}

	// Y lines
	for (int i = 1; i < numy; ++i) {
		for (int j = 0; j < numx; ++j) {
			double xpos = xoffset * j;
			double ypos = yoffset * i;
			double tolypos = ypos + tolerance;
			double point0 = xpos;
			double point1 = xpos + (xoffset / 2.5);
			double point2 = xpos + (xoffset - (xoffset / 2.5));
			double point3 = xpos + xoffset;
			double radius = yoffset / 6.0;

			if ((2 * radius) + tolerance < point2 - point1) radius = ((point2 - point1) / 2.0) + tolerance2;

			if (j != 0) point0 += tolerance;

			// Randomize the direction of the puzzle cut
			int dir = rand() % 2;

			// Draw puzzle lines
			if (dir == 1) {
				svg.createLine(point0, ypos, point1, ypos);
				svg.createLine(point0, tolypos, point1 + tolerance2, tolypos);

				svg.createArc(point1, ypos, point2, ypos, radius, 1);
				svg.createArc(point1 + tolerance2, ypos, point2 - tolerance2, ypos, radius - tolerance, 1);

				svg.createLine(point1 + tolerance2, ypos, point1 + tolerance2, tolypos);
				svg.createLine(point2 - tolerance2, ypos, point2 - tolerance2, tolypos);

				svg.createLine(point2, ypos, point3, ypos);
				svg.createLine(point2 - tolerance2, tolypos, point3, tolypos);
			}
			else if (dir == 0) {
				svg.createLine(point0, tolypos, point1, tolypos);
				svg.createLine(point0, ypos, point1 + tolerance2, ypos);

				svg.createArc(point1, tolypos, point2, tolypos, radius, 0);
				svg.createArc(point1 + tolerance2, tolypos, point2 - tolerance2, tolypos, radius - tolerance, 0);

				svg.createLine(point1 + tolerance2, ypos, point1 + tolerance2, tolypos);
				svg.createLine(point2 - tolerance2, ypos, point2 - tolerance2, tolypos);

				svg.createLine(point2, tolypos, point3, tolypos);
				svg.createLine(point2 - tolerance2, ypos, point3, ypos);
			}

		}
	}

	// Edge lines

	svg.createLine(0, 0, 0, ylen);
	svg.createLine(0, 0, xlen, 0);
	svg.createLine(0, ylen, xlen, ylen);
	svg.createLine(xlen, 0, xlen, ylen);

	svg.endWriting();

	return svg;
}

void positionPuzzle(Ptr<Sketch> sk, Ptr<Sketch> sk2, Ptr<Point3D> min, Ptr<Point3D> max) {
	// Copy objects to new sketch
	Ptr<ObjectCollection> coll = ObjectCollection::create();

	for (Ptr<SketchCurve> skCurve : sk->sketchCurves()) {
		coll->add(skCurve);
	}

	for (Ptr<SketchPoint> skPoint : sk->sketchPoints()) {
		coll->add(skPoint);
	}

	// Generate translation of the svg
	Ptr<Matrix3D> transf = Matrix3D::create();
	Ptr<Point3D> transl;

	if (max->x() - min->x() > 0 && max->y() - min->y() > 0) {
		transl = Point3D::create(min->x(), max->y(), 0);
	}
	else if (max->x() - min->x() < 0 && max->y() - min->y() > 0) {
		transl = Point3D::create(max->x(), max->y(), 0);
	}
	else if (max->x() - min->x() < 0 && max->y() - min->y() < 0) {
		transl = Point3D::create(max->x(), min->y(), 0);
	}
	else if (max->x() - min->x() > 0 && max->y() - min->y() < 0) {
		transl = Point3D::create(min->x(), min->y(), 0);
	}
	transf->translation(sk->originPoint()->geometry()->vectorTo(transl));

	sk->copy(coll, transf, sk2);
	sk->deleteMe();
}

// InputChange event handler.
class OnInputChangedEventHander : public adsk::core::InputChangedEventHandler
{
public:
	void notify(const Ptr<InputChangedEventArgs>& eventArgs) override
	{
		pieceSizeVal = pieceSizeInput->value() * 10;
		// Minimum 10mm piece size
		if (pieceSizeVal < 10) pieceSizeVal = 10;
		pieceSizeInput->value(pieceSizeVal / 10.0);


		toleranceVal = toleranceInput->value() * 10;
		// Minimum 0.1mm tolerance
		if (toleranceVal < 0.1) toleranceVal = 0.1;
		toleranceInput->value(toleranceVal / 10.0);

		if (faceSelectionInput->selectionCount() > 0) {
			selectedFace = faceSelectionInput->selection(0)->entity();
		}
	}
};

// CommandExecuted event handler.
class OnExecuteEventHander : public adsk::core::CommandEventHandler
{
public:
	void notify(const Ptr<CommandEventArgs>& eventArgs) override
	{
		// Get the active design
		Ptr<Design> design = app->activeProduct();

		// Get the root component
		Ptr<Component> rootComp = design->rootComponent();
		Ptr<Sketches> rootCompSketches = rootComp->sketches();
		Ptr<ExtrudeFeatures> rootCompExtrudes = rootComp->features()->extrudeFeatures();

		// Get and body
		Ptr<BRepBody> selectedFaceBody = selectedFace->body();

		// Create a new sketch on the selected face
		Ptr<Sketch> sk = rootCompSketches->addWithoutEdges(selectedFace);

		// Get the bounding box of the selected body
		Ptr<BoundingBox3D> boundingBox = selectedFaceBody->boundingBox();

		// Create min and max bound coordinates from the bounding box
		Ptr<Point3D> minBounding = sk->modelToSketchSpace(boundingBox->minPoint());
		Ptr<Point3D> maxBounding = sk->modelToSketchSpace(boundingBox->maxPoint());

		Ptr<Point3D> minBounding2d = Point3D::create(minBounding->x(), minBounding->y(), 0);
		Ptr<Point3D> maxBounding2d = Point3D::create(maxBounding->x(), maxBounding->y(), 0);

		// Draw puzzle lines
		SVGWriter svg = drawPuzzle(minBounding2d, maxBounding2d);

		// Import the svg into the sketch
		sk->importSVG(svg.getFilePath(), 0, 0, 1);

		// Position the svg on the object
		Ptr<Sketch> sk2 = rootCompSketches->addWithoutEdges(selectedFace);
		positionPuzzle(sk, sk2, minBounding2d, maxBounding2d);
	}
};

// CommandDestroyed event handler
class OnDestroyEventHandler : public adsk::core::CommandEventHandler
{
public:
	void notify(const Ptr<CommandEventArgs>& eventArgs) override
	{

		adsk::terminate();
	}
};

// ExecutePreview event handler
class OnExecutePreviewEventHandler : public adsk::core::CommandEventHandler
{
public:
	void notify(const Ptr<CommandEventArgs>& eventArgs) override
	{
		// Get the active design
		Ptr<Design> design = app->activeProduct();

		// Get the root component
		Ptr<Component> rootComp = design->rootComponent();
		Ptr<Sketches> rootCompSketches = rootComp->sketches();
		Ptr<ExtrudeFeatures> rootCompExtrudes = rootComp->features()->extrudeFeatures();

		// Get and body
		Ptr<BRepBody> selectedFaceBody = selectedFace->body();

		// Create a new sketch on the selected face
		sketch1 = rootCompSketches->addWithoutEdges(selectedFace);

		// Get the bounding box of the selected body
		Ptr<BoundingBox3D> boundingBox = selectedFaceBody->boundingBox();

		// Create min and max bound coordinates from the bounding box
		Ptr<Point3D> minBounding = sketch1->modelToSketchSpace(boundingBox->minPoint());
		Ptr<Point3D> maxBounding = sketch1->modelToSketchSpace(boundingBox->maxPoint());

		Ptr<Point3D> minBounding2d = Point3D::create(minBounding->x(), minBounding->y(), 0);
		Ptr<Point3D> maxBounding2d = Point3D::create(maxBounding->x(), maxBounding->y(), 0);

		// Draw puzzle lines
		SVGWriter svg = drawPuzzle(minBounding2d, maxBounding2d);

		// Import the svg into the sketch
		sketch1->importSVG(svg.getFilePath(), 0, 0, 1);

		// Position the svg on the object
		sketch2 = rootCompSketches->addWithoutEdges(selectedFace);
		positionPuzzle(sketch1, sketch2, minBounding2d, maxBounding2d);
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

				// Connect to the input changed event.
				Ptr<InputChangedEvent> onInputChanged = command->inputChanged();
				if (!onInputChanged)
					return;
				isOk = onInputChanged->add(&onInputChangedHandler);
				if (!isOk)
					return;

				// Connect the execute event
				Ptr<CommandEvent> onExecute = command->execute();
				onExecute->add(&onExecuteHandler);

				Ptr<CommandEvent> onExecutePreview = command->executePreview();
				onExecutePreview->add(&onExecutePreviewHandler);

				// Get the CommandInputs collection associated with the command.
				Ptr<CommandInputs> inputs = command->commandInputs();
				if (!inputs)
					return;

				pieceSizeInput = inputs->addValueInput("piecesize", "Piece Size", "mm", ValueInput::createByReal(2.0));
				toleranceInput = inputs->addValueInput("tolerance", "Tolerance", "mm", ValueInput::createByReal(0.03));

				faceSelectionInput = inputs->addSelectionInput("selection", "Select Cut Face", "Select Face");
				faceSelectionInput->setSelectionLimits(1, 1);
				faceSelectionInput->addSelectionFilter("Faces");

			}
		}
	}
private:
	OnExecutePreviewEventHandler onExecutePreviewHandler;
	OnExecuteEventHander onExecuteHandler;
	OnDestroyEventHandler onDestroyHandler;
	OnInputChangedEventHander onInputChangedHandler;
} _cmdCreatedHandler;

extern "C" XI_EXPORT bool run(const char* context)
{
	app = Application::get();
	if (!app)
		return false;

	ui = app->userInterface();
	if (!ui)
		return false;

	// Create the command definition.
	Ptr<CommandDefinitions> commandDefinitions = ui->commandDefinitions();
	if (!commandDefinitions)
		return nullptr;

	// Get the existing command definition or create it if it doesn't already exist.
	Ptr<CommandDefinition> cmdDef = commandDefinitions->itemById("cmdInputsPuzzleCutter");
	if (!cmdDef)
	{
		cmdDef = commandDefinitions->addButtonDefinition("cmdInputsPuzzleCutter",
			"Puzzle Cutter",
			"Creates a profile to cut a plane into a puzzle.");
	}

	// Connect to the command created event.
	Ptr<CommandCreatedEvent> commandCreatedEvent = cmdDef->commandCreated();
	if (!commandCreatedEvent)
		return false;
	commandCreatedEvent->add(&_cmdCreatedHandler);


	// Execute the command definition.
	cmdDef->execute();

	// Prevent this module from being terminated when the script returns, because we are waiting for event handlers to fire.
	adsk::autoTerminate(false);


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
