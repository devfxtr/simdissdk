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
#ifndef SIMVIS_PROJECTOR_MANAGER_H
#define SIMVIS_PROJECTOR_MANAGER_H

#include "simCore/Common/Common.h"
#include "simVis/Projector.h"
#include "osgEarth/MapNode"
#include "osgEarth/MapNodeObserver"

namespace simVis
{

/** Responsible for managing projectors in the scene */
class ProjectorManager : public osg::Group, public osgEarth::MapNodeObserver
{
public:
  /** Default constructor */
  ProjectorManager();

  /**
  * Registers a projector with the manager, so it will be included in the texture projection
  * calculations.
  * @param proj Projector to register.
  */
  void registerProjector(ProjectorNode* proj);

  /**
  * Unregisters a projector.
  * @param proj Projector to remove.
  */
  void unregisterProjector(ProjectorNode* proj);

  /**
  * Clear all projector nodes and group nodes from manager
  */
  void clear();

public: // MapNodeObserver

  /** Gets the map node */
  osgEarth::MapNode* getMapNode() { return mapNode_.get(); }

  /** Sets the map node */
  void setMapNode(osgEarth::MapNode* mapNode);

public: // osg::Node

  /** Override in order to limit node traversals to cull visitor only */
  void traverse(osg::NodeVisitor& nv);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "ProjectorManager"; }

protected:
  /// osg::Referenced-derived
  virtual ~ProjectorManager() {}

private:

  /**
  * Initialize projector manager
  */
  void initialize_();

  /// Current osgEarth MapNode
  osg::observer_ptr<osgEarth::MapNode> mapNode_;

  /// Vector for holding projector nodes created by the scenario
  typedef std::vector< osg::ref_ptr<ProjectorNode> > Projectors;
  Projectors projectors_;

  /// Map for associating projector node Ids to projector state sets
  typedef std::map<simData::ObjectId, osg::ref_ptr<osg::Group> > GroupMap;
  GroupMap groupMap_;

  /// REX Layer for a projector (REX engine only)
  class ProjectorLayer : public osgEarth::Layer
  {
  public:
    ProjectorLayer(simData::ObjectId id);
    /// Return owner entity id
    simData::ObjectId id() const;
  private:
    /// Owner entity id
    simData::ObjectId id_;
  };

  /// Vector of projectorLayers that have been added to the mapNode
  typedef std::vector<osg::ref_ptr<ProjectorLayer> > ProjectorLayerVector;
  ProjectorLayerVector projectorLayers_;
};
};

#endif // SIMVIS_PROJECTOR_MANAGER_H