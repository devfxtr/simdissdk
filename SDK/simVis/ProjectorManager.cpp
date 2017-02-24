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
#include "osg/Depth"
#include "osg/BlendFunc"
#include "osgEarth/StringUtils"
#include "osgEarth/TerrainEngineNode"
#include "osgEarth/VirtualProgram"
#include "simVis/Shaders.h"
#include "simVis/ProjectorManager.h"
#include "simVis/Utils.h"

#define LC "simVis::ProjectorManager "

using namespace simVis;
using namespace osgEarth;

/// Projector texture unit for shader and projector state sets
static const int PROJECTOR_TEXTURE_UNIT = 5;

ProjectorManager::ProjectorLayer::ProjectorLayer(simData::ObjectId id)
  : osgEarth::Layer(),
    id_(id)
{
#ifdef HAVE_OSGEARTH_MAP_GETLAYERS
  setRenderType(osgEarth::Layer::RENDERTYPE_TILE);
#endif
}

simData::ObjectId ProjectorManager::ProjectorLayer::id() const
{
  return id_;
}

ProjectorManager::ProjectorManager()
{
  setCullingActive(false);
}

void ProjectorManager::setMapNode(MapNode* mapNode)
{
  if (mapNode != mapNode_.get())
  {
    mapNode_ = mapNode;

    // reinitialize the projection system
    if (mapNode_.valid())
    {
      initialize_();
#ifdef HAVE_OSGEARTH_MAP_GETLAYERS
      if (simVis::useRexEngine())
      {
        // Get existing layers in the new map
        osgEarth::LayerVector currentLayers;
        mapNode_->getMap()->getLayers(currentLayers);

        for (ProjectorLayerVector::const_iterator piter = projectorLayers_.begin(); piter != projectorLayers_.end(); ++piter)
        {
          // Check if projector layer already exists
          bool found = false;
          for (osgEarth::LayerVector::const_iterator iter = currentLayers.begin(); iter != currentLayers.end(); ++iter)
          {
            ProjectorLayer* pLayer = dynamic_cast<ProjectorLayer*>((*iter).get());
            if ((*piter) == pLayer)
            {
              found = true;
              break;
            }
          }

          // If not found, add this layer to the map
          if (!found)
            mapNode_->getMap()->addLayer(piter->get());
        }
      }
#endif
    }
  }
}

void ProjectorManager::initialize_()
{
  if (simVis::useRexEngine() == false)
  {
    osg::StateSet* stateSet = getOrCreateStateSet();

    // shader code to render the projectors
    osgEarth::VirtualProgram* vp = new osgEarth::VirtualProgram();
    vp->setInheritShaders(true);
    simVis::Shaders package;
    package.load(vp, package.projectorManagerVertex());
    package.load(vp, package.projectorManagerFragment());

    // the OVERRIDE flag will cause this program of override the terrain engine's programs,
    // but it will still inherit those above (like the log depth buffer shader!)
    stateSet->setAttributeAndModes(vp, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    // tells the shader to always find its sampler on unit 0
    osg::Uniform* samplerUniform = new osg::Uniform(osg::Uniform::SAMPLER_2D, "simProjSampler");
    samplerUniform->set(PROJECTOR_TEXTURE_UNIT);
    stateSet->addUniform(samplerUniform);

    // An LEQUAL depth test lets consecutive passes overwrite each other:
    // Needs help: (TODO) ... can see the skirts
    stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, false), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    // BlendFunc controls how the resulting shader fragment gets combined with the frame buffer.
    // This will "decal" the projected image on the terrain and will preserve alpha in the src image.
    stateSet->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
  }
}

void ProjectorManager::registerProjector(ProjectorNode* proj)
{
  if (simVis::useRexEngine())
  {
    // Check if this ProjectorNode already exists in the map and exit if so
    if (std::find(projectors_.begin(), projectors_.end(), proj) != projectors_.end())
      return;

    projectors_.push_back(proj);

    osg::StateSet* projStateSet = new osg::StateSet();

    // shader code to render the projectors
    osgEarth::VirtualProgram* vp = VirtualProgram::getOrCreate(projStateSet);
    simVis::Shaders package;
    package.load(vp, package.projectorManagerVertex());
    package.load(vp, package.projectorManagerFragment());

    projStateSet->setDefine("SIMVIS_USE_REX");

    // tells the shader where to bind the sampler uniform
    projStateSet->addUniform(new osg::Uniform("simProjSampler", PROJECTOR_TEXTURE_UNIT));

    // Set texture from projector into state set
    projStateSet->setTextureAttribute(PROJECTOR_TEXTURE_UNIT, proj->getTexture());

    projStateSet->addUniform(proj->projectorActive_.get());
    projStateSet->addUniform(proj->projectorAlpha_.get());
    projStateSet->addUniform(proj->texGenMatUniform_.get());
    projStateSet->addUniform(proj->texProjDirUniform_.get());
    projStateSet->addUniform(proj->texProjPosUniform_.get());

#ifdef HAVE_OSGEARTH_MAP_GETLAYERS
    ProjectorLayer* layer = new ProjectorLayer(proj->getId());
    layer->setName("SIMSDK Projector");
    layer->setStateSet(projStateSet);
    projectorLayers_.push_back(layer);

    mapNode_->getMap()->addLayer(layer);
#endif
    return;
  }

  // MP engine

  // Check if this ProjectorNode already exists in the map and exit if so
  if (groupMap_.find(proj->getId()) != groupMap_.end())
    return;

  projectors_.push_back(proj);

  osg::Group* projGroup = new osg::Group();

  osg::StateSet* projStateSet = projGroup->getOrCreateStateSet();

  // set the rendering bin so that this terrain pass happens after the main rendering
  const osg::StateSet* terrainStateSet = mapNode_->getTerrainEngine()->getOrCreateStateSet();

  //TODO: reevaluate this
  projStateSet->setRenderBinDetails(terrainStateSet->getBinNumber() + projectors_.size() + 1, terrainStateSet->getBinName());

  // Set texture from projector into state set
  projStateSet->setTextureAttributeAndModes(PROJECTOR_TEXTURE_UNIT, proj->getTexture(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

  projStateSet->addUniform(proj->projectorActive_.get());
  projStateSet->addUniform(proj->projectorAlpha_.get());
  projStateSet->addUniform(proj->texGenMatUniform_.get());
  projStateSet->addUniform(proj->texProjDirUniform_.get());
  projStateSet->addUniform(proj->texProjPosUniform_.get());

  // install the terrain engine to get the rendering pass:
  projGroup->addChild(mapNode_->getTerrainEngine());

  // install the path for this projector.
  addChild(projGroup);

  // Keep associated record of group to projector node id
  groupMap_[proj->getId()] = projGroup;
}

void ProjectorManager::unregisterProjector(ProjectorNode* proj)
{
  if (simVis::useRexEngine())
  {
    for (ProjectorLayerVector::iterator i = projectorLayers_.begin(); i != projectorLayers_.end(); ++i)
    {
      ProjectorLayer* layer = i->get();
      if (layer->id() == proj->getId())
      {
#ifdef HAVE_OSGEARTH_MAP_GETLAYERS
        // Remove it from the map:
        osg::ref_ptr<MapNode> mapNode;
        if (mapNode_.lock(mapNode))
          mapNode->getMap()->removeLayer(layer);
#endif

        // Remove it from the local vector:
        projectorLayers_.erase(i);
        break;
      }
    }
  }
  else // MP engine
  {
    // Find and remove projector group
    GroupMap::iterator iter = groupMap_.find(proj->getId());
    if (iter != groupMap_.end())
    {
      removeChild(iter->second);
      groupMap_.erase(iter);
    }
  }

  // Remove projector node
  projectors_.erase(std::remove(projectors_.begin(), projectors_.end(), proj), projectors_.end());
}

void ProjectorManager::clear()
{
#ifdef HAVE_OSGEARTH_MAP_GETLAYERS
  if (simVis::useRexEngine())
  {
    // Remove it from the map:
    osg::ref_ptr<MapNode> mapNode;
    if (mapNode_.lock(mapNode))
    {
      for (ProjectorLayerVector::const_iterator i = projectorLayers_.begin(); i != projectorLayers_.end(); ++i)
      {
        mapNode->getMap()->removeLayer(i->get());
      }
    }
    projectorLayers_.clear();
  }
  else // MP engine
#endif
  {
    groupMap_.clear();
    removeChildren(0, getNumChildren());
  }

  projectors_.clear();
}

void ProjectorManager::traverse(osg::NodeVisitor& nv)
{
  // cull only. the terrain was already traversed my osgearth so there's no need for
  // update/event travs again. (It would be nice to make this more efficient by restricting
  // the culling frustum to the projector's frustum -gw)
  if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
  {
    osg::Group::traverse(nv);
  }
}