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
#include "osgEarthAnnotation/LocalGeometryNode"
#include "osgEarthSymbology/GeometryFactory"
#include "osgEarthFeatures/GeometryCompiler"
#include "simCore/Common/Common.h"
#include "simVis/GOG/Circle.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/Utils.h"
#include "simNotify/Notify.h"

using namespace simVis::GOG;
using namespace osgEarth::Features;

GogNodeInterface* Circle::deserialize(const osgEarth::Config&  conf,
                    simVis::GOG::ParserData& p,
                    const GOGNodeType&       nodeType,
                    const GOGContext&        context,
                    const GogMetaData&       metaData,
                    MapNode*                 mapNode)
{
  Distance radius(conf.value("radius", 1000.), p.units_.rangeUnits_);

  osgEarth::Symbology::GeometryFactory gf;
  Geometry* shape = gf.createCircle(osg::Vec3d(0, 0, 0), radius);

  osgEarth::Annotation::LocalGeometryNode* node = NULL;

  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    // Try to prevent terrain z-fighting.
    if (p.geometryRequiresClipping())
      Utils::configureStyleForClipping(p.style_);

    node = new osgEarth::Annotation::LocalGeometryNode(shape, p.style_);
    node->setMapNode(mapNode);
  }
  else
    node = new HostedLocalGeometryNode(shape, p.style_);
  node->setName("GOG Circle Position");

  GogNodeInterface* rv = NULL;
  if (node)
  {
    Utils::applyLocalGeometryOffsets(*node, p, nodeType);
    rv = new LocalGeometryNodeInterface(node, metaData);
    rv->applyConfigToStyle(conf, p.units_);
  }
  return rv;
}
