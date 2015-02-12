/*
	*** Main Window
	*** src/gui/mainwindow.h
	Copyright T. Youngs 2007-2015

	This file is part of Aten.

	Aten is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Aten is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Aten.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ATEN_MAINWINDOW_H
#define ATEN_MAINWINDOW_H

#include "base/dnchar.h"
#include "base/atom.h"
#include "classes/prefs.h"
#include "gui/ui_mainwindow.h"
#include "gui/ui_prefs.h"
#include "gui/useractions.h"
#include "templates/reflist.h"
#include "base/glyph.h"

#define MAXRECENTFILES 10

// Forward Declarations 1 - Main Form and Windows
class AtenAbout;
class AtenPrefs;
class AtenForcefieldEditor;
class AtenProgress;

// Forward Declarations 2 - Dialogs
class AtenLoadModel;
class AtenSelectFilter;
class AtenSelectPattern;
class AtenSelectElement;
class AtenSelectVariable;
class AtenViewBasis;
class AtenViewEigenvector;
class AtenZMatrix;

// Forward Declarations 3 - Dock Widgets and Wizards
class AtomListWidget;
class BuildWidget;
class CellDefinitionWidget;
class CellTransformWidget;
class CommandWidget;
class DisorderWizard;
class ForcefieldsWidget;
class FragmentsWidget;
class GeometryWidget;
class GlyphsWidget;
class GridsWidget;
class MDWidget;
class MessagesWidget;
class ModelListWidget;
class PoresWidget;
class PositionWidget;
class ScriptMovieWidget;
class SelectWidget;
class ToolBoxWidget;
class TrajectoryWidget;
class TransformWidget;
class VibrationsWidget;

// Forward Declarations
class QLabel;
class QProgressBar;
class QPushButton;
class QFrame;
class QSettings;
class QActionGroup;
class QButtonGroup;
class QStringListModel;
class Forest;
class Tree;
class Aten;

class AtenWindow : public QMainWindow
{
	// All Qt declarations must include this macro
	Q_OBJECT

	public:
	// Constructor / Destructor
	AtenWindow(Aten& aten);
	~AtenWindow();
	// Main form declaration
	Ui::AtenWindow ui;


	/*
	 * Aten Reference
	 */
	private:
	// Aten reference
	Aten& aten_;

	public:
	// Return reference to Aten
	Aten& aten();


	/*
	 * Window Functions
	 */
	public:
	// Finalise GUI
	void finaliseUi();
	// Set controls to reflect program variables
	void setControls();

	protected:
	void closeEvent(QCloseEvent *event);


	/*
	 * Methods
	 */
	public:
	// Initialise GUI file filters array
	void initFilters();
	// Add a message to the main window's message output box
	void printMessage(const char*);
	// Save before close
	bool saveBeforeClose();
	// Return the PID of Aten
	int pid();
	// Set interactivity (to full or zero), except for main view camera changes
	void setInteractive(bool interactive);


	/*
	 * Refresh Functions
	 */
	private:
	// Refresh window title
	void updateWindowTitle();

	public:
	// Refresh main window
	void updateMainWindow();
	// Update trajectory control widgets
	void updateTrajectoryMenu();

	
	/*
	 * File Menu / Actions
	 */
	public:
	bool runSaveModelDialog();

	private slots:
	void on_actionFileNew_triggered(bool checked);
	void on_actionFileOpen_triggered(bool checked);
	void on_actionFileSave_triggered(bool checked);
	void on_actionFileSaveAs_triggered(bool checked);
	void on_actionExportOptions_triggered(bool checked);
	void on_actionFileClose_triggered(bool checked);
	void on_actionFileSaveImage_triggered(bool checked);
	void on_actionFileQuit_triggered(bool checked);
	void on_actionFileOpenGrid_triggered(bool checked);

	
	/*
	 * Edit Menu / Actions
	 */
	private slots:
	void on_actionEditUndo_triggered(bool checked);
	void on_actionEditRedo_triggered(bool checked);
	void on_actionEditCut_triggered(bool checked);
	void on_actionEditCopy_triggered(bool checked);
	void on_actionEditPaste_triggered(bool checked);
	void on_actionEditPasteTranslated_triggered(bool checked);
	void on_actionEditDelete_triggered(bool checked);
	void on_actionSelectionAll_triggered(bool checked);
	void on_actionSelectionNone_triggered(bool checked);
	void on_actionSelectionExpand_triggered(bool checked);
	void on_actionSelectionInvert_triggered(bool checked);
	void on_actionEditQuickCommand_triggered(bool checked);


	/*
	 * View Menu / Actions
	 */
	public:
	void setCartesianView(double x, double y, double z);
	void setCellView(double x, double y, double z);
	void setActiveSchemeAction(Prefs::ColouringScheme cs);

	private slots:
	void on_actionViewReset_triggered(bool checked);
	void on_actionViewZoomIn_triggered(bool checked);
	void on_actionViewZoomOut_triggered(bool checked);
	void on_actionViewPerspective_triggered(bool checked);
	void on_actionViewOrthographic_triggered(bool checked);
	void on_actionViewSetCartesianPosX_triggered(bool checked);
	void on_actionViewSetCartesianPosY_triggered(bool checked);
	void on_actionViewSetCartesianPosZ_triggered(bool checked);
	void on_actionViewSetCartesianNegX_triggered(bool checked);
	void on_actionViewSetCartesianNegY_triggered(bool checked);
	void on_actionViewSetCartesianNegZ_triggered(bool checked);
	void on_actionViewSetCellNegX_triggered(bool checked);
	void on_actionViewSetCellNegY_triggered(bool checked);
	void on_actionViewSetCellNegZ_triggered(bool checked);
	void on_actionViewSetCellPosX_triggered(bool checked);
	void on_actionViewSetCellPosY_triggered(bool checked);
	void on_actionViewSetCellPosZ_triggered(bool checked);
	void on_actionSchemeElement_triggered(bool checked);
	void on_actionSchemeCharge_triggered(bool checked);
	void on_actionSchemeForce_triggered(bool checked);
	void on_actionSchemeVelocity_triggered(bool checked);
	void on_actionSchemeCustom_triggered(bool checked);
	void on_actionDetectDisplayHBonds_triggered(bool checked);


	/*
	 * Selection Menu / Actions (doubles as Atom Context Menu)
	 */
	public:
	void activateGlyphActions(int n);

	private:
	// Atom under mouse when context menu was called
	Atom* contextAtom_;
	void setAtomStyle(Atom::DrawStyle ds);
	void setAtomLabel(Atom::AtomLabel al);
	void removeAtomLabels(bool all);
	void setAtomHidden(bool hidden);
	QAction *createGlyphActions[Glyph::nGlyphTypes];

	private slots:
	void on_actionAtomStyleStick_triggered(bool checked);
	void on_actionAtomStyleTube_triggered(bool checked);
	void on_actionAtomStyleSphere_triggered(bool checked);
	void on_actionAtomStyleScaled_triggered(bool checked);
	void on_actionAtomLabelID_triggered(bool checked);
	void on_actionAtomLabelCharge_triggered(bool checked);
	void on_actionAtomLabelFFType_triggered(bool checked);
	void on_actionAtomLabelElement_triggered(bool checked);
	void on_actionAtomLabelFFEquiv_triggered(bool checked);
	void on_actionAtomLabelClear_triggered(bool checked);
	void on_actionAtomLabelClearAll_triggered(bool checked);
	void on_actionAtomColourReset_triggered(bool checked);
	void on_actionAtomColourSet_triggered(bool checked);
	void on_actionOrderShiftUp_triggered(bool checked);
	void on_actionOrderShiftDown_triggered(bool checked);
	void on_actionOrderMoveToStart_triggered(bool checked);
	void on_actionOrderMoveToEnd_triggered(bool checked);
	void on_actionOrderReorder_triggered(bool checked);
	void on_actionAtomHide_triggered(bool checked);
	void on_actionAtomProbe_triggered(bool checked);
	void on_actionAtomFixPosition_triggered(bool checked);
	void on_actionAtomFreePosition_triggered(bool checked);
	void on_actionSetBondLength_triggered(bool checked);
	void on_actionSetBondAngle_triggered(bool checked);
	void on_actionSetTorsionAngle_triggered(bool checked);
	void on_actionCreateFragment_triggered(bool checked);
	void on_actionCentreAtOrigin_triggered(bool checked);
	void createGlyph();


	/*
	 * Model Menu
	 */
	private slots:
	void on_actionModelRename_triggered(bool checked);
	void on_actionModelFoldAtoms_triggered(bool checked);
	void on_actionModelFoldMolecules_triggered(bool checked);
	void on_actionModelNext_triggered(bool checked);
	void on_actionModelPrevious_triggered(bool checked);
	void on_actionModelShowAll_triggered(bool checked);
	void on_actionListMeasurements_triggered(bool checked);

	
	/*
	 * Trajectory Menu
	 */
	private slots:
	void on_actionTrajectoryOpen_triggered(bool checked);
	void on_actionTrajectoryRemove_triggered(bool checked);
	void on_actionTrajectoryInheritParentStyle_triggered(bool checked);
	void on_actionTrajectoryCopyStyleToParent_triggered(bool checked);
	void on_actionTrajectoryPropagateStyleFromHere_triggered(bool checked);
	void on_actionTrajectoryFirstFrame_triggered(bool checked);
	void on_actionTrajectoryLastFrame_triggered(bool checked);
	void on_actionTrajectoryPlayPause_triggered(bool checked);
	void on_actionTrajectoryModel_triggered(bool checked);
	void on_actionTrajectoryFrames_triggered(bool checked);
	void on_actionTrajectorySaveMovie_triggered(bool checked);


	/*
	 * Expression Menu
	 */
	private slots:
	void on_actionOpenForcefield_triggered(bool checked);
	void on_actionOpenExpression_triggered(bool checked);
	void on_actionSaveExpression_triggered(bool checked);
	void on_actionModelCreatePatterns_triggered(bool checked);
	void on_actionModelRemovePatterns_triggered(bool checked);
	void on_actionModelListPatterns_triggered(bool checked);
	void on_actionModelFFType_triggered(bool checked);
	void on_actionModelFFUntype_triggered(bool checked);
	void on_actionModelCreateExpression_triggered(bool checked);
	void on_actionModelAddDefaultPattern_triggered(bool checked);


	/*
	 * Settings Menu / Actions
	 */
	private slots:
	void on_actionPreferences_triggered(bool checked);
	void on_actionReloadFilters_triggered(bool checked);
	void on_actionShowToolBox_triggered(bool checked);
	void on_actionStoreDefaultWindowState_triggered(bool checked);
	void on_actionManualSwapBuffers_triggered(bool checked);


	/*
	 * Help Menu / Actions
	 */
	private slots:
	void on_actionAboutAten_triggered(bool checked);
	void on_actionAboutQt_triggered(bool checked);

	
	/*
	 * Main Toolbar (other actions not already account for by menus)
	 */
	private slots:
	void on_actionStyleStick_triggered(bool checked);
	void on_actionStyleTube_triggered(bool checked);
	void on_actionStyleSphere_triggered(bool checked);
	void on_actionStyleScaled_triggered(bool checked);
	void on_actionStyleIndividual_triggered(bool checked);
	void on_actionSelectAtoms_triggered(bool checked);
	void on_actionSelectMolecules_triggered(bool checked);
	void on_actionSelectElement_triggered(bool checked);

	public:
	void setActiveStyleAction(Atom::DrawStyle ds);


	/*
	 * Mouse Toolbar Actions
	 */
	private slots:
	void on_actionMouseInteract_triggered(bool checked);
	void on_actionMouseRotate_triggered(bool checked);
	void on_actionMouseTranslate_triggered(bool checked);


	/*
	 * Local Widgets and Routines
	 */
	private:
	// List of manually-created QActionGroups
	Reflist<QActionGroup,int> actionGroups_;
	// Action group for main toolbar select actions
	QActionGroup *uaSelectActions_;
	// Dummy button for user-action group
	QToolButton *uaDummyButton_;
	// User-Action button group
	QButtonGroup uaButtons_;
	// Text labels for model information and UI messages in status bar
	QLabel *infoLabel1_, *infoLabel2_, *messageLabel_;
	// Filter set from save model dialog
	Tree* saveModelFilter_;
	// Filename set from save model dialog
	Dnchar saveModelFilename_;

	private slots:
	// Change current user action
	void uaButtonClicked(int id);

	public:
	// Update any controls related to Prefs values etc.
	void updateControls();
	// Update undo/redo labels
	void updateUndoRedo();
	// Set action/button to reflect supplied user action
	void setActiveUserAction(UserAction::Action ua);
	// Set message label text
	void setMessageLabel(const char *s);


	/*
	 * Context Menu
	 */
	public:
	// Update context menu
	void updateContextMenu();
	// Call the atompopup menu
	void callContextMenu(Atom*, int, int);


	/*
	 * Settings
	 */
	private:
	// Settings structure
	QSettings settings_;
	// Load settings
	void loadSettings();
	// Save settings
	void saveSettings();


	/*
	 * Recent files
	 */
	private slots:
	// Load recent file
	void loadRecent();

	private:
	// Pointers to recent file actions
	QAction *actionRecentFile[MAXRECENTFILES];

	public:
	// Add file to top of recent list
	void addRecent(const char*);


	/*
	 * Dock Widgets
	 */
	public:
	// Update Targets
	enum UpdateTarget { AtomsTarget = 1, CellTarget = 2, ForcefieldsTarget = 4, GlyphsTarget = 8, GridsTarget = 16, ModelsTarget = 32, CanvasTarget = 64, StatusBarTarget = 128, GeometryTarget = 256, VibrationsTarget = 512, SelectTarget = 1024, TrajectoryTarget = 2048, AllTarget = 4095 };

	private:
	// List of dock widgets
	QList<QDockWidget*> dockWidgets_;
	
	public:
	// Atom list dock widget
	AtomListWidget *atomListWidget;
	// Build dock widget
	BuildWidget *buildWidget;
	// Cell definition dock widget
	CellDefinitionWidget *cellDefinitionWidget;
	// Cell transform dock widget
	CellTransformWidget *cellTransformWidget;
	// Command dock widget
	CommandWidget *commandWidget;
	// Disorder wizard
	DisorderWizard *disorderWizard;
	// Forcefields dock widget
	ForcefieldsWidget *forcefieldsWidget;
	// Fragment Library dock widget
	FragmentsWidget *fragmentsWidget;
	// Geometry dock widget
	GeometryWidget *geometryWidget;
	// Glyphs dock widget
	GlyphsWidget *glyphsWidget;
	// Grids dock widget
	GridsWidget *gridsWidget;
	// Messages dock widget
	MessagesWidget *messagesWidget;
	// Model List dock widget
	ModelListWidget *modelListWidget;
	// Pore builder dock widget
	PoresWidget *poresWidget;
	// Atom positioning dock widget
	PositionWidget *positionWidget;
	// Scripted movie dock widget
	ScriptMovieWidget *scriptMovieWidget;
	// Atom selection dock widget
	SelectWidget *selectWidget;
	// Trajectory control dock widget
	TrajectoryWidget *trajectoryWidget;
	// Atom transformation dock widget
	TransformWidget *transformWidget;
	// Vibrations dock widget
	VibrationsWidget *vibrationsWidget;


	/*
	 * GUI / Interaction
	 */
	private:
	// Active interaction mode of the main canvas
	UserAction::Action activeMode_;
	// Selected interaction mode (from GUI)
	UserAction::Action selectedMode_;
	// Whether the mouse has moved between begin_mode() and end_mode() calls
	bool hasMoved_;
	// Current drawing depth for certain tools
	double currentDrawDepth_;
	// Selected drawing element
	short int sketchElement_;
	// Whether to accept editing actions (i.e. anything other than view manipulation)
	bool editable_;
	// Number of atoms to pick in PickAtomsAction
	int nAtomsToPick_;
	// User action before picking mode was entered
	UserAction::Action actionBeforePick_;
	// List of picked atoms
	Reflist<Atom,int> pickedAtoms_;
	// Pointer to callback function when PickAtomsAction exits
	void (*pickAtomsCallback_)(Reflist<Atom,int>*);
	// Atom that was clicked at the start of a mouse press event
	Atom* atomClicked_;
	// Whether we are selecting atoms and placing them in the subsel list	
	bool pickEnabled_;
	// Reflist of selected atoms and their positions so manipulations may be un-done
	Reflist< Atom,Vec3<double> > oldPositions_;

	private:
	// Begin an action on the model (called from MouseButtondown)
	void beginMode(Prefs::MouseButton button);
	// End an action on the model (called from MouseButtonup)
	void endMode(Prefs::MouseButton button);
	
	public:
	// Refreshes specified (or all) dock widgets
	void updateWidgets(int targets = 0);
	// Refresh main viewer
	void postRedisplay();
	// Set the active mode to the current user mode
	void useSelectedMode();
	// Sets the currently selected interact mode
	void setSelectedMode(UserAction::Action ua, int atomsToPick = -1, void (*callback)(Reflist<Atom,int>*) = NULL);
	// Cancel any current mode and return to select
	void cancelCurrentMode();
	// Return the currently selected mode
	UserAction::Action selectedMode() const;
	// Return the currently active mode
	UserAction::Action activeMode() const;
	// Set current drawing element
	void setSketchElement(short int el);
	// Return current drawing element
	short int sketchElement() const;
	// Current drawing depth for certain tools
	double currentDrawDepth();
	// Set whether to accept editing actions (i.e. anything other than view manipulation)
	void setEditable(bool b);
	// Return whether to accept editing actions (i.e. anything other than view manipulation)
	bool editable();
	// Returns the clicked atom within a mouse click event
	Atom* atomClicked();
	// Clears the subsel of atoms
	void clearPicked();
	// End manual picking
	void endManualPick(bool resetaction);
	// Return start of picked atom list
	Refitem<Atom,int>* pickedAtoms();
};

#endif
