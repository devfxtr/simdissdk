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
#include <osgText/Font>
#include <osgText/Text>
#include <osgEarthSymbology/TextSymbol>
#include "simVis/View.h"
#include "simVis/Utils.h"
#include "simVis/Registry.h"
#include "ClassificationBanner.h"

namespace simVis
{

// Classification banner outline thickness
static const double OUTLINE_THICKNESS = .03;

/** Data Store listener that updates the classification label contents when scenario properties change */
class ClassificationBanner::ScenarioListenerImpl : public simData::DataStore::ScenarioListener
{
public:
  /** Constructor */
  explicit ScenarioListenerImpl(ClassificationBanner* parent) :parent_(parent) {}
  virtual ~ScenarioListenerImpl(){}

  /** Updates label when the scenario properties change */
  void onScenarioPropertiesChange(simData::DataStore *source)
  {
    parent_->updateClassLabel_();
  }

private:
  ClassificationBanner* parent_;
};

//////////////////////////////////////////////

class ClassificationBanner::FrameResizeCallback : public osg::NodeCallback
{
public:
  /** Constructor */
  explicit FrameResizeCallback(ClassificationBanner* parent) :parent_(parent) {}
  virtual ~FrameResizeCallback(){}

  /** Updates banner positions when the screen size changes */
  virtual void operator()(osg::Node *node, osg::NodeVisitor *nv)
  {
    if (nv->getVisitorType() != osg::NodeVisitor::CULL_VISITOR)
    {
      traverse(node, nv);
      return;
    }

    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
    if (!cv)
    {
      traverse(node, nv);
      return;
    }
    // Get the Model-View-Projection-Window matrix (MVPW) from the visitor
    const osg::RefMatrixd* mvpw = cv->getMVPW();
    if (!mvpw)
    {
      traverse(node, nv);
      return;
    }
    // Nothing to do if the MVPW hasn't changed
    if (lastMvpw_ == (*mvpw))
    {
      traverse(node, nv);
      return;
    }
    lastMvpw_ = *mvpw;

    double width = cv->getViewport()->width();
    // Banners should be horizontally centered and ten pixels from the top and bottom
    osg::Vec3 topPixelPos(width / 2, cv->getViewport()->height() - 10, 0);
    osg::Vec3 bottomPixelPos(width / 2, 10, 0);
    // Multiply the desired pixel position of the banners with the inverse mvpw to get the local position to set the banners to
    osg::Matrix inverseMvpw = osg::Matrix::inverse(lastMvpw_);
    osg::Vec3 topLocalPos = topPixelPos * inverseMvpw;
    osg::Vec3 bottomLocalPos = bottomPixelPos * inverseMvpw;
    // Z coordinate should always be 0 for HUD text, but can be something else after multiplying by inverse matrix
    topLocalPos.z() = 0;
    bottomLocalPos.z() = 0;
    parent_->setTopPosition_(topLocalPos);
    parent_->setBottomPosition_(bottomLocalPos);

    traverse(node, nv);
  }

private:
  ClassificationBanner* parent_;
  osg::Matrix lastMvpw_;
};

//////////////////////////////////////////////

ClassificationBanner::ClassificationBanner(simData::DataStore* dataStore, unsigned int fontSize, const std::string& fontFile)
  : osg::Group(),
  dataStore_(dataStore),
  fontSize_(fontSize),
  fontFile_(fontFile)
{
  createClassLabels_();
  listener_ = simData::DataStore::ScenarioListenerPtr(new ScenarioListenerImpl(this));
  if (dataStore_)
    dataStore_->addScenarioListener(listener_);

  resizeCallback_ = new FrameResizeCallback(this);
  addCullCallback(resizeCallback_);
}

ClassificationBanner::~ClassificationBanner()
{
  if (dataStore_)
    dataStore_->removeScenarioListener(listener_);
}

void ClassificationBanner::addToView(simVis::View* managedView)
{
  if (!managedView)
    return;
  managedView->getOrCreateHUD()->addChild(this);
}

void ClassificationBanner::removeFromView(simVis::View* managedView)
{
  if (managedView)
    managedView->getOrCreateHUD()->removeChild(this);
}

void ClassificationBanner::setFontFile(const std::string& fontFile)
{
  fontFile_ = fontFile;
  osgText::Font* foundFont = simVis::Registry::instance()->getOrCreateFont(fontFile_);
  if (classLabelUpper_)
    classLabelUpper_->setFont(foundFont);
  if (classLabelLower_)
    classLabelLower_->setFont(foundFont);
}

void ClassificationBanner::setFontSize(unsigned int fontSize)
{
  fontSize_ = fontSize;
  if (classLabelUpper_)
    classLabelUpper_->setCharacterSize(simVis::osgFontSize(fontSize_));
  if (classLabelLower_)
    classLabelLower_->setCharacterSize(simVis::osgFontSize(fontSize_));
}

void ClassificationBanner::setTopPosition_(const osg::Vec3& topPos)
{
  if (classLabelUpper_)
    classLabelUpper_->setPosition(topPos);
}

void ClassificationBanner::setBottomPosition_(const osg::Vec3& bottomPos)
{
  if (classLabelLower_)
    classLabelLower_->setPosition(bottomPos);
}

void ClassificationBanner::createClassLabels_()
{
  // get current classification
  std::string classLabel;
  osg::Vec4f classColor;
  getCurrentClassification_(classLabel, classColor);
  // load default font, make sure we found the file
  osgText::Font* foundFont = simVis::Registry::instance()->getOrCreateFont(fontFile_);
  assert(foundFont);

  // create upper and lower label controls
  classLabelUpper_ = createText_(classLabel, classColor, foundFont, osgText::Text::CENTER_TOP);
  classLabelUpper_->setName("Classification Banner Upper");
  addChild(classLabelUpper_);
  classLabelLower_ = createText_(classLabel, classColor, foundFont, osgText::Text::CENTER_BOTTOM);
  classLabelLower_->setName("Classification Banner Lower");
  addChild(classLabelLower_);
}

osgText::Text* ClassificationBanner::createText_(const std::string& classLabel,
  const osg::Vec4f& classColor,
  osgText::Font* fontFile,
  osgText::Text::AlignmentType alignment) const
{
  osgText::Text* newText = new osgText::Text;
  newText->setText(classLabel);
  newText->setFont(fontFile);
  newText->setCharacterSize(simVis::osgFontSize(fontSize_));
  newText->setColor(classColor);
  newText->setBackdropType(osgText::Text::OUTLINE);
  // Set opaque black outline color
  newText->setBackdropColor(osg::Vec4f(0.f, 0.f, 0.f, 1.f));
  newText->setBackdropOffset(OUTLINE_THICKNESS);
  newText->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
  newText->setAlignment(alignment);
  newText->setCharacterSizeMode(osgText::TextBase::SCREEN_COORDS);
  newText->setAxisAlignment(osgText::TextBase::SCREEN);
  newText->setDataVariance(osg::Object::DYNAMIC);

  return newText;
}

void ClassificationBanner::getCurrentClassification_(std::string& classLabel, osg::Vec4f& classColor)
{
  if (!dataStore_)
    return;
  simData::DataStore::Transaction transaction;
  const simData::ScenarioProperties_Classification& classification =
    dataStore_->scenarioProperties(&transaction)->classification();
  classLabel = classification.label();
  classColor = simVis::ColorUtils::RgbaToVec4(classification.fontcolor());
}

void ClassificationBanner::updateClassLabel_()
{
  // get current classification
  std::string classLabel;
  osg::Vec4f classColor;
  getCurrentClassification_(classLabel, classColor);
  if (classLabel.empty())
    return;
  if (!classLabelUpper_.valid()) // assume upper and lower are always same state
    return;

  if (classLabelUpper_->getText().createUTF8EncodedString() != classLabel)
    classLabelUpper_->setText(classLabel);
  if (classLabelUpper_->getColor() != classColor)
    classLabelUpper_->setColor(classColor);
  if (classLabelLower_->getText().createUTF8EncodedString() != classLabel)
    classLabelLower_->setText(classLabel);
  if (classLabelLower_->getColor() != classColor)
    classLabelLower_->setColor(classColor);
}

} // namespace simVis
