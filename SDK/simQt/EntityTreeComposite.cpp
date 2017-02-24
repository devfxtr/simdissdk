/* -*- mode: c++ -*- */
/****************************************************************************
 *****                                                                  *****
 *****                   Classification: UNCLASSIFIED                   *****
 *****                    Classified By:                                *****
 *****                    Declassify On:                                *****
 *****                                                                  *****
 ****************************************************************************
 *
 *
 * Developed by: Naval Research Laboratory, Tactical Electronic Warfare Div.
 *               EW Modeling & Simulation, Code 5773
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <QDialog>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QClipboard>
#include "simQt/QtConversion.h"
#include "simQt/ScopedSignalBlocker.h"
#include "simQt/ResourceInitializer.h"
#include "simQt/EntityTreeWidget.h"
#include "simQt/EntityFilterLineEdit.h"
#include "simQt/EntityTreeComposite.h"
#include "simQt/AbstractEntityTreeModel.h"
#include "ui_EntityTreeComposite.h"

namespace simQt {

FilterDialog::FilterDialog(QWidget* parent)
  :QDialog(parent)
{}

void FilterDialog::closeEvent(QCloseEvent* ev)
{
  QDialog::closeEvent(ev);
  emit(closedGui());
}

EntityTreeComposite::EntityTreeComposite(QWidget* parent)
: QWidget(parent),
  composite_(NULL),
  entityTreeWidget_(NULL),
  model_(NULL),
  filterDialog_(NULL)
{
  ResourceInitializer::initialize();  // Needs to be here so that Qt Designer works.

  composite_ = new Ui_EntityTreeComposite();
  composite_->setupUi(this);
  composite_->pushButton->setEnabled(false);
  composite_->filterButton->hide(); // start out hidden until filters are added
  entityTreeWidget_ = new EntityTreeWidget(composite_->treeView);
  connect(entityTreeWidget_, SIGNAL(itemsSelected(QList<uint64_t>)), this, SLOT(onItemsChanged_(QList<uint64_t>)));
  connect(entityTreeWidget_, SIGNAL(itemsSelected(QList<uint64_t>)), this, SIGNAL(itemsSelected(QList<uint64_t>)));  // Echo out the signal
  connect(entityTreeWidget_, SIGNAL(itemDoubleClicked(uint64_t)), this, SIGNAL(itemDoubleClicked(uint64_t))); // Echo out the signal
  connect(entityTreeWidget_, SIGNAL(filterSettingsChanged(QMap<QString, QVariant>)), this, SIGNAL(filterSettingsChanged(QMap<QString, QVariant>))); // Echo out the signal

  connect(composite_->pushButton, SIGNAL(toggled(bool)), entityTreeWidget_, SLOT(toggleTreeView(bool)));
  connect(composite_->lineEdit, SIGNAL(changed(QString, Qt::CaseSensitivity, QRegExp::PatternSyntax)), this, SLOT(textFilterChanged_(QString, Qt::CaseSensitivity, QRegExp::PatternSyntax)));
  connect(composite_->filterButton, SIGNAL(clicked()), this, SLOT(showFilters_()));
  connect(entityTreeWidget_, SIGNAL(numFilteredItemsChanged(int, int)), this, SLOT(setNumFilteredItemsLabel_(int, int)));

  // handle right-context menu (any actions will appear there)
  composite_->treeView->addAction(composite_->actionCollapse_All);
  composite_->treeView->addAction(composite_->actionExpand_All);

  // Create a new QAction for copying data from the clipboard
  copyAction_ = new QAction(tr("&Copy"), composite_->treeView);
  copyAction_->setEnabled(false); // Should only be enabled when selections made
  copyAction_->setShortcut(QKeySequence::Copy);
  copyAction_->setShortcutContext(Qt::WidgetShortcut);
  connect(copyAction_, SIGNAL(triggered()), this, SLOT(copySelection_()));
  composite_->treeView->addAction(copyAction_);

  // Set tooltips
  composite_->pushButton->setToolTip(simQt::formatTooltip(tr("Filter View"),
  tr("Toggles the display of entity types between a tree and a list view.")));
  composite_->filterButton->setToolTip(simQt::formatTooltip(tr("Entity Filter"),
  tr("Opens the Entity Filter dialog.<p>Used for filtering the display of entities shown in the Entity List.")));
  // Note: tool tip applied to magnifying glass icon (label); the lineEdit already has a comment in the text field
  composite_->label->setToolTip(simQt::formatTooltip(tr("Name Filter"),
  tr("Performs filtering based on entity names.<p>Right click in the text field to modify filtering options.")));
}

EntityTreeComposite::~EntityTreeComposite()
{
  closeFilters_(); // clean up filter dialog
  delete composite_;
  delete entityTreeWidget_;
  // we don't own model_ so don't delete it
}

void EntityTreeComposite::addEntityFilter(EntityFilter* entityFilter)
{
  entityTreeWidget_->addEntityFilter(entityFilter);
  // If filter button is hidden see if this filter will transition it to shown
  if (composite_->filterButton->isHidden())
  {
    QList<QWidget*> filterWidgets = entityTreeWidget_->filterWidgets(this);
    if (!filterWidgets.empty())
      composite_->filterButton->show();
    // Since we own the filter widgets, we should remove them now so they don't leak or accumulate
    qDeleteAll(filterWidgets);
  }
}

void EntityTreeComposite::setModel(AbstractEntityTreeModel* model)
{
  // Must pass in a valid model
  assert(model != NULL);

  model_ = model;
  entityTreeWidget_->setModel(model_);
  // If the tree is pre-loaded, enable the tree/list button
  if (model_->rowCount() != 0)
    composite_->pushButton->setEnabled(true);
  connect((QObject*)model, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(rowsInserted_(QModelIndex, int, int)));
}

/** Sets/clears the selected ID in the entity list */
void EntityTreeComposite::setSelected(uint64_t id, bool selected)
{
  entityTreeWidget_->setSelected(id, selected);
}

void EntityTreeComposite::setSelected(QList<uint64_t> list, bool selected)
{
  entityTreeWidget_->setSelected(list, selected);
}

void EntityTreeComposite::scrollTo(uint64_t id, QAbstractItemView::ScrollHint hint)
{
  entityTreeWidget_->scrollTo(id, hint);
}

QAbstractItemView::SelectionMode EntityTreeComposite::selectionMode() const
{
  return entityTreeWidget_->selectionMode();
}

void EntityTreeComposite::setSelectionMode(QAbstractItemView::SelectionMode mode)
{
  entityTreeWidget_->setSelectionMode(mode);
}

QTreeView* EntityTreeComposite::view() const
{
  return entityTreeWidget_->view();
}

simData::ObjectId EntityTreeComposite::alwaysShow() const
{
  return entityTreeWidget_->alwaysShow();
}

void EntityTreeComposite::setAlwaysShow(simData::ObjectId id)
{
  entityTreeWidget_->setAlwaysShow(id);
}

void EntityTreeComposite::getFilterSettings(QMap<QString, QVariant>& settings) const
{
  settings.insert("RegExp", entityTreeWidget_->regExp());
  entityTreeWidget_->getFilterSettings(settings);
}

void EntityTreeComposite::setFilterSettings(const QMap<QString, QVariant>& settings)
{
  simQt::ScopedSignalBlocker blockSignals(*this);
  QMap<QString, QVariant>::const_iterator it = settings.find("RegExp");
  if (it != settings.end())
  {
    QRegExp regExp = it.value().toRegExp();
    // Update the GUI and signals will take care of the rest
    composite_->lineEdit->configure(regExp.pattern(), regExp.caseSensitivity(), regExp.patternSyntax());
  }
  entityTreeWidget_->setFilterSettings(settings);
}

/** Clears all selections */
void EntityTreeComposite::clearSelection()
{
  entityTreeWidget_->clearSelection();
}

/** Gets a list of all the selected IDs in the entity list */
QList<uint64_t> EntityTreeComposite::selectedItems() const
{
  return entityTreeWidget_->selectedItems();
}

/** Allows the developer to customize the look by adding buttons after the filter text **/
void EntityTreeComposite::addButton(QWidget* button)
{
  composite_->horizontalLayout->addWidget(button);
}

void EntityTreeComposite::setListTreeButtonDisplayed(bool value)
{
  composite_->pushButton->setVisible(value);
}

void EntityTreeComposite::setSettings(SettingsPtr settings)
{
  entityTreeWidget_->setSettings(settings);

  // make sure the composite's treeview/listview pushbutton state matches widget treeview/listview state, suppress signal since the widget will have already done the toggle
  bool treeView = entityTreeWidget_->isTreeView();
  simQt::ScopedSignalBlocker blockSignals(*composite_->pushButton);
  composite_->pushButton->setChecked(treeView);
}

void EntityTreeComposite::initializeSettings(SettingsPtr settings)
{
  EntityTreeWidget::initializeSettings(settings);
}

void EntityTreeComposite::textFilterChanged_(QString filter, Qt::CaseSensitivity caseSensitive, QRegExp::PatternSyntax syntax)
{
  QRegExp regExp(filter, caseSensitive, syntax);
  entityTreeWidget_->setRegExp(regExp);
}

void EntityTreeComposite::rowsInserted_(const QModelIndex & parent, int start, int end)
{
  composite_->pushButton->setEnabled(true);
}

void EntityTreeComposite::showFilters_()
{
  if (filterDialog_ != NULL)
  {
    filterDialog_->show();
    return;
  }
  // create a new filter dialog, using the filter widgets from the EntityTreeWidget's proxy model
  filterDialog_ = new FilterDialog(this);
  QList<QWidget*> filterWidgets = entityTreeWidget_->filterWidgets(filterDialog_);
  filterDialog_->setMinimumWidth(200);
  filterDialog_->setWindowTitle("Entity Filters");
  filterDialog_->setWindowFlags(filterDialog_->windowFlags() ^ Qt::WindowContextHelpButtonHint);
  QVBoxLayout* layout = new QVBoxLayout(filterDialog_);
  layout->setContentsMargins(2, 2, 2, 2);
  Q_FOREACH(QWidget* widget, filterWidgets)
  {
    // create a label for each widget, using the widget WindowTitle as text
    QGroupBox* groupBox = new QGroupBox(widget->windowTitle(), filterDialog_);
    QVBoxLayout* gbLayout = new QVBoxLayout(groupBox);
    gbLayout->setContentsMargins(2, 2, 2, 2);
    gbLayout->addWidget(widget);
    groupBox->setLayout(gbLayout);
    layout->addWidget(groupBox);
  }

  // connect to the close signal, to clean up resources
  connect(filterDialog_, SIGNAL(closedGui()), this, SLOT(closeFilters_()));
  filterDialog_->setLayout(layout);
  filterDialog_->show();
}

void EntityTreeComposite::closeFilters_()
{
  // we own all this memory, so we can delete it
  delete filterDialog_;
  filterDialog_ = NULL;
}

void EntityTreeComposite::setNumFilteredItemsLabel_(int numFilteredItems, int numTotalItems)
{
  composite_->countLabel->setText(QString("%1 of %2 Filtered Entity Names").arg(numFilteredItems).arg(numTotalItems));
}

void EntityTreeComposite::setExpandsOnDoubleClick(bool value)
{
  composite_->treeView->setExpandsOnDoubleClick(value);
}

bool EntityTreeComposite::expandsOnDoubleClick() const
{
  return composite_->treeView->expandsOnDoubleClick();
}

void EntityTreeComposite::onItemsChanged_(QList<uint64_t> ids)
{
  copyAction_->setEnabled(!ids.isEmpty());
}

void EntityTreeComposite::copySelection_()
{
  QList<uint64_t> ids =  entityTreeWidget_->selectedItems();

  if (ids.isEmpty() || (model_ == NULL))
    return;

  QString clipboardText;
  Q_FOREACH(uint64_t id, ids)
  {
    if (!clipboardText.isEmpty())
      clipboardText.append("\n");

    QModelIndex index = model_->index(id);
    clipboardText.append(model_->data(index, Qt::DisplayRole).toString());
  }

  QApplication::clipboard()->setText(clipboardText);
}

}
