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
#include <QVBoxLayout>
#include <QAbstractProxyModel>

#include "simData/DataStoreHelpers.h"
#include "simQt/QtConversion.h"
#include "simQt/ScopedSignalBlocker.h"
#include "simQt/ResourceInitializer.h"
#include "simQt/EntityTreeWidget.h"
#include "simQt/EntityTreeModel.h"
#include "simQt/EntityTreeComposite.h"
#include "simQt/EntityTypeFilter.h"
#include "simQt/EntityCategoryFilter.h"
#include "simQt/EntityFilterLineEdit.h"
#include "simQt/EntityProxyModel.h"
#include "simQt/EntityLineEdit.h"
#include "ui_EntityLineEdit.h"

namespace simQt {

namespace {
  static const QString VALID_ENTITY = "";
  static const QString INVALID_ENTITY = "QLineEdit:enabled { color: red }";
}

EntityDialog::EntityDialog(QWidget* parent, simQt::EntityTreeModel* entityTreeModel, simData::DataStore::ObjectType type)
  : QDialog(parent),
    entityTreeModel_(entityTreeModel)
{
  setWindowTitle("Select Entity");
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  setObjectName("SelectEntity");

  tree_ = new simQt::EntityTreeComposite(this);
  tree_->setModel(entityTreeModel_);
  tree_->setExpandsOnDoubleClick(true);
  tree_->setSelectionMode(QAbstractItemView::SingleSelection);
  tree_->setListTreeButtonDisplayed(false);  // The Entity Line Composite does not support the tree view

  tree_->addEntityFilter(new simQt::EntityTypeFilter(*entityTreeModel_->dataStore(), type, type == simData::DataStore::ALL));
  tree_->addEntityFilter(new simQt::EntityCategoryFilter(entityTreeModel_->dataStore(), true));

  connect(tree_, SIGNAL(itemsSelected(QList<uint64_t>)), this, SLOT(setSelected_(QList<uint64_t>)));
  connect(tree_, SIGNAL(itemDoubleClicked(uint64_t)), this, SLOT(accept())); // Have double click auto close the dialog

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->addWidget(tree_);
  setLayout(layout);
}

EntityDialog::~EntityDialog()
{
}

void EntityDialog::closeEvent(QCloseEvent* ev)
{
  QDialog::closeEvent(ev);
  emit(closedGui());
}

void EntityDialog::setItemSelected(uint64_t id)
{
  tree_->clearSelection();
  if (id != 0)
  {
    tree_->setSelected(id, true);
    tree_->scrollTo(id);
  }
}

void EntityDialog::setSelected_(QList<uint64_t> ids)
{
  if (!ids.isEmpty())
  {
    emit itemSelected(ids.front());
  }
}


//--------------------------------------------------------------------------------------------------

/// notify the tree model about data store changes
class EntityLineEdit::DataStoreListener : public simData::DataStore::DefaultListener
{
public:
  /// constructor
  explicit DataStoreListener(EntityLineEdit *parent)
    : parent_(parent)
  {
  }

  /// entity with the given id and type will be removed after all notifications are processed
  virtual void onRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::DataStore::ObjectType ot)
  {
    if (parent_->uniqueId_ == removedId)
    {
      parent_->composite_->lineEdit->setStyleSheet(INVALID_ENTITY);
      parent_->uniqueId_ = 0;
    }
  }

  /// entity name has changed
  virtual void onNameChange(simData::DataStore *source, simData::ObjectId changeId)
  {
    if (parent_->uniqueId_ == changeId)
      parent_->composite_->lineEdit->setText(simData::DataStoreHelpers::nameOrAliasFromId(changeId, source).c_str());
  }

protected:
  EntityLineEdit *parent_; ///< model which receives notices
};

//--------------------------------------------------------------------------------------------------

EntityLineEdit::EntityLineEdit(QWidget* parent, simQt::EntityTreeModel* entityTreeModel, simData::DataStore::ObjectType type)
: QWidget(parent),
  composite_(NULL),
  entityTreeModel_(NULL), // set below with the setModel call
  entityDialog_(NULL),
  uniqueId_(0),
  needToVerify_(false),
  type_(type)
{
  ResourceInitializer::initialize();  // Needs to be here so that Qt Designer works.

  composite_ = new Ui_EntityLineEdit();
  composite_->setupUi(this);
  composite_->lineEdit->setToolTip(simQt::formatTooltip(tr("Entity Name"), tr("Either type or select an entity name.<p>Select from the popup or from the dialog by clicking the browser button.")));
  composite_->lineEdit->setPlaceholderText("Enter entity name...");
  connect(composite_->toolButton, SIGNAL(clicked()), this, SLOT(showEntityDialog_()));
  connect(composite_->lineEdit, SIGNAL(editingFinished()), this, SLOT(editingFinished_()));
  connect(composite_->lineEdit, SIGNAL(textEdited(const QString&)), this, SLOT(textEdited_(const QString&)));

  setModel(entityTreeModel, type_);
  // Double clicking on an empty text field will display the Entity Dialog
  composite_->lineEdit->installEventFilter(this);
}

EntityLineEdit::~EntityLineEdit()
{
  if ((entityTreeModel_ != NULL) && (dataListenerPtr_ != NULL))
    entityTreeModel_->dataStore()->removeListener(dataListenerPtr_);
  closeEntityDialog();
  delete composite_;
}

void EntityLineEdit::setModel(simQt::EntityTreeModel* model, simData::DataStore::ObjectType type)
{
  type_ = type;

  if (model != NULL)
  {
    entityTreeModel_ = model;
    entityTreeModel_->setToListView();

    proxy_ = new EntityProxyModel(this);
    proxy_->addEntityFilter(new simQt::EntityTypeFilter(*entityTreeModel_->dataStore(), type, type == simData::DataStore::ALL));
    proxy_->setSourceModel(entityTreeModel_);

    QCompleter* completer = new QCompleter(proxy_, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionRole(Qt::DisplayRole);

    QTreeView* view = new QTreeView(this);
    view->header()->hide();  // Need to hide the column headers because the popup height does not account for the header obscuring a line
    view->setRootIsDecorated(false);
    // It would be nice to set the column widths, but the commands seem to be ignored
    completer->setPopup(view);
    // If the EntityLineEdit starts off disabled than the view is always disabled (Qt bug?); if forced enabled here then the view follows the EntityLineEdit enable/disable
    view->setEnabled(true);

    connect(completer, SIGNAL(activated(const QModelIndex &)), this, SLOT(wasActived_(const QModelIndex &)));

    composite_->lineEdit->setCompleter(completer);

    dataListenerPtr_ = simData::DataStore::ListenerPtr(new DataStoreListener(this));
    entityTreeModel_->dataStore()->addListener(dataListenerPtr_);
  }
  else if (entityTreeModel_ != NULL)
  {
    entityTreeModel_->dataStore()->removeListener(dataListenerPtr_);
    dataListenerPtr_.reset();
    entityTreeModel_ = NULL;
  }
}

void EntityLineEdit::wasActived_(const QModelIndex & index)
{
  if (entityTreeModel_ == NULL)
    return;

  QCompleter* completer = composite_->lineEdit->completer();
  QAbstractProxyModel* proxyModel = qobject_cast<QAbstractProxyModel*>(completer->completionModel());
  Q_ASSERT(proxyModel != 0);

  // Unwind the double proxy, ours and the build in proxy of the completer
  QModelIndex modelIndex = proxy_->mapToSource(proxyModel->mapToSource(index));

  const uint64_t newId = entityTreeModel_->uniqueId(modelIndex);
  if (newId == uniqueId_)
    return;
  uniqueId_ = newId;
  needToVerify_ = false;
  composite_->lineEdit->setStyleSheet(VALID_ENTITY);
  emit itemSelected(uniqueId_);
  if (entityDialog_ != NULL)
    entityDialog_->setItemSelected(uniqueId_);
}

uint64_t EntityLineEdit::selected() const
{
  return uniqueId_;
}

QString EntityLineEdit::selectedName() const
{
  QModelIndex index = entityTreeModel_->index(uniqueId_);
  if (!index.isValid())
    return QString();

  return entityTreeModel_->data(index, Qt::DisplayRole).toString();
}

int EntityLineEdit::setSelected(uint64_t id)
{
  if (entityTreeModel_ == NULL || id == uniqueId_)
    return 1;

  QModelIndex index = entityTreeModel_->index(id);
  if (!index.isValid())
    return 1;

  QString name = entityTreeModel_->data(index, Qt::DisplayRole).toString();
  composite_->lineEdit->setText(name);
  uniqueId_ = id;
  needToVerify_ = false;
  composite_->lineEdit->setStyleSheet(VALID_ENTITY);
  if (entityDialog_ != NULL)
    entityDialog_->setItemSelected(uniqueId_);
  emit itemSelected(uniqueId_);
  return 0;
}

void EntityLineEdit::showEntityDialog_()
{
  if (entityTreeModel_ == NULL)
    return;

  if (entityDialog_ == NULL)
  {
    entityDialog_ = new EntityDialog(this, entityTreeModel_, type_);

    connect(entityDialog_, SIGNAL(itemSelected(uint64_t)), this, SLOT(setSelected(uint64_t)));
    connect(entityDialog_, SIGNAL(closedGui()), this, SLOT(closeEntityDialog()));
  }

  entityDialog_->setItemSelected(uniqueId_);
  entityDialog_->show();
}

void EntityLineEdit::closeEntityDialog()
{
  // we own all this memory, so we can delete it
  delete entityDialog_;
  entityDialog_ = NULL;
}

void EntityLineEdit::editingFinished_()
{
  if (entityTreeModel_ == NULL)
    return;

  if (needToVerify_)
  {
    uniqueId_ = simData::DataStoreHelpers::idByName(composite_->lineEdit->text().toStdString(), entityTreeModel_->dataStore());
    if ((uniqueId_ == 0) && (!composite_->lineEdit->text().isEmpty()))
      composite_->lineEdit->setStyleSheet(INVALID_ENTITY);
    else
    {
      composite_->lineEdit->setStyleSheet(VALID_ENTITY);
      if (entityDialog_ != NULL)
        entityDialog_->setItemSelected(uniqueId_);
    }
    emit itemSelected(uniqueId_);
    needToVerify_ = false;
  }
}

void EntityLineEdit::textEdited_(const QString & text)
{
  needToVerify_ = true;
  composite_->lineEdit->setStyleSheet(VALID_ENTITY);
}

QString EntityLineEdit::tooltip() const
{
  return composite_->lineEdit->toolTip();
}

void EntityLineEdit::setTooltip(const QString& tooltip)
{
  composite_->lineEdit->setToolTip(tooltip);
}

QString EntityLineEdit::placeholderText() const
{
  return composite_->lineEdit->placeholderText();
}
void EntityLineEdit::setPlaceholderText(const QString& text)
{
  composite_->lineEdit->setPlaceholderText(text);
}

bool EntityLineEdit::includeDialogButton() const
{
  return composite_->toolButton->isVisible();
}

void EntityLineEdit::setIncludeDialogButton(bool value)
{
  composite_->toolButton->setVisible(value);
}

bool EntityLineEdit::eventFilter(QObject* obj, QEvent* evt)
{
  if (obj == composite_->lineEdit)
  {
    if ((evt->type() == QEvent::MouseButtonDblClick) && composite_->lineEdit->text().isEmpty())
    {
      showEntityDialog_();
      return true;
    }
  }
  return false;
}

}
