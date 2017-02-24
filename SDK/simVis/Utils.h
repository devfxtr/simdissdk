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
#ifndef SIMVIS_UTILS_H
#define SIMVIS_UTILS_H

#include "simCore/Common/Common.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/Vec3.h"
#include "simCore/Time/Clock.h"
#include "simData/DataTypes.h"
#include "simUtil/PlatformSimulator.h"

#include "osg/NodeCallback"
#include "osg/Geometry"
#include "osg/Quat"
#include "osg/Matrix"
#include "osg/BoundingBox"
#include "osgGA/GUIEventHandler"
#include "osgText/Text"

#include "osgEarth/GeoData"
#include "osgEarth/MapNode"
#include "osgEarth/SpatialReference"
#include "osgEarth/Units"
#include "osgEarth/Registry"
#include "osgEarth/ShaderFactory"
#include "osgEarthSymbology/Color"

// MACROS to test for changes in protobuf properties.

#define PB_HAS_FIELD(a, field) ( \
  ((a)!=NULL) && ((a)->has_##field()) )

#define PB_DOESNT_HAVE_FIELD(a, field) ( \
  ((a)==NULL) || (!(a)->has_##field()) )

#define PB_FIELD_APPEARED(a, b, field) ( \
  PB_DOESNT_HAVE_FIELD((a), field) && \
  PB_HAS_FIELD((b), field) )

#define PB_FIELD_DISAPPEARED(a, b, field) ( \
  PB_HAS_FIELD((a), field) && \
  PB_DOESNT_HAVE_FIELD((b), field) )

#define PB_FIELD_STATUS_CHANGED(a, b, field) ( \
  PB_HAS_FIELD((a), field) != PB_HAS_FIELD((b), field) )

#define PB_BOTH_HAVE_FIELD(a, b, field) ( \
  PB_HAS_FIELD((a), field) && PB_HAS_FIELD((b), field) )

#define PB_FIELD_CHANGED(a, b, field) ( \
  PB_FIELD_STATUS_CHANGED((a), (b), field) || ( \
    PB_BOTH_HAVE_FIELD((a), (b), field) && ((a)->field() != (b)->field()) ) )


#define PB_HAS_SUBFIELD(a, first, second) ( \
  PB_HAS_FIELD((a), first) && (a)->first().has_##second() )

#define PB_DOESNT_HAVE_SUBFIELD(a, first, second) ( \
  PB_DOESNT_HAVE_FIELD((a), first) || (!(a)->first().has_##second()) )

#define PB_BOTH_HAVE_SUBFIELD(a, b, first, second) ( \
  PB_HAS_SUBFIELD((a), first, second) && PB_HAS_SUBFIELD((b), first, second) )

#define PB_SUBFIELD_APPEARED(a, b, first, second) ( \
  PB_DOESNT_HAVE_SUBFIELD((a), first, second) && PB_HAS_SUBFIELD((b), first, second) )

#define PB_SUBFIELD_STATUS_CHANGED(a, b, first, second) ( \
  PB_HAS_SUBFIELD((a), first, second) != PB_HAS_SUBFIELD((b), first, second) )

#define PB_SUBFIELD_CHANGED(a, b, first, second) ( \
  PB_SUBFIELD_STATUS_CHANGED((a), (b), first, second) || ( \
    PB_BOTH_HAVE_SUBFIELD((a), (b), first, second) && \
    (a)->first().second() != (b)->first().second() ) )


namespace simVis
{
  class PlatformModelNode;

  /**
   * Whether to use the REX terrain engine.
   */
  SDKVIS_EXPORT bool useRexEngine();

  /**
   * Enable or disable lighting on a state set. We must set both the
   * FFP mode and the uniform to support shader-based and non-shader-based
   * nodes.
   */
  SDKVIS_EXPORT void setLighting(osg::StateSet* stateset, osg::StateAttribute::GLModeValue value);

  /**
   * remove the lighting directive from a state set so that it inherits
   * its lighting setting.
   */
  SDKVIS_EXPORT void setLightingToInherit(osg::StateSet* stateset);

  /** gets the lighting state if there is one (true if there is, false if not) */
  inline bool getLighting(osg::StateSet* stateset, osg::StateAttribute::OverrideValue& out_value)
  {
    if (!stateset) return false;
    osg::StateAttribute::GLModeValue value = stateset->getMode(GL_LIGHTING);
    out_value = stateset->getMode(value);
    return out_value != osg::StateAttribute::INHERIT;
  }

  /**
   * Internal update template callback - binds an update callback to the
   * template-class's "update()" method.
   * @tparam T osg::Node that has a method matching the signature update(const osg::FrameStamp*).
   */
  template<typename T>
  struct NodeUpdateCallback : public osg::NodeCallback
  {
    /** Calls update on the node and continues traversal */
    void operator()(osg::Node *node, osg::NodeVisitor *nv)
    {
      static_cast<T*>(node)->update(nv->getFrameStamp());
      traverse(node, nv);
    }

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }

    /** Return the class name */
    virtual const char* className() const { return "NodeUpdateCallback<T>"; }

  protected:
    /// osg::Referenced-derived
    virtual ~NodeUpdateCallback() {}
  };

  /// convert a simCore::Coordinate to a GeoPoint, if possible
  SDKVIS_EXPORT bool convertCoordToGeoPoint(
    const simCore::Coordinate&        input,
    osgEarth::GeoPoint&               output,
    const osgEarth::SpatialReference* mapSRS);

  /// convert a GeoPoint to a simCore::Coordinate, if possible
 SDKVIS_EXPORT bool convertGeoPointToCoord(
    const osgEarth::GeoPoint&         input,
    simCore::Coordinate&              output,
    osgEarth::MapNode*                mapNode);

  /// whether a simCore::CoordinateSystem represents world-space coords.
  inline bool isWorldCoordSys(const simCore::CoordinateSystem& cs)
  {
    return
      cs == simCore::COORD_SYS_ECEF ||
      cs == simCore::COORD_SYS_LLA  ||
      cs == simCore::COORD_SYS_ECI;
  }

  /// whether a coordinate system represents localized coords.
  inline bool isLocalCoordSys(const simCore::CoordinateSystem& cs)
  {
    return
      cs == simCore::COORD_SYS_ENU ||
      cs == simCore::COORD_SYS_GTP ||
      cs == simCore::COORD_SYS_NED ||
      cs == simCore::COORD_SYS_NWU ||
      cs == simCore::COORD_SYS_XEAST;
  }

  /// convert an OSG vec3 to a SIM vec3
  inline simCore::Vec3 convertToSim(const osg::Vec3d& in)
  {
    return simCore::Vec3(in.x(), in.y(), in.z());
  }

  /// convert a SIM vec2 to an OSG vec3
  inline simCore::Coordinate convertOSGtoSimCoord(const osg::Vec3d& in, const simCore::CoordinateSystem& cs)
  {
    return simCore::Coordinate(cs, simCore::Vec3(in.x(), in.y(), in.z()));
  }

  /// convert a SIM vec2 to an OSG vec3
  inline osg::Vec3d convertToOSG(const simCore::Vec3& in)
  {
    return osg::Vec3d(in.x(), in.y(), in.z());
  }

  /**
   * Converts a model from NWU to ENU (in place).
   */
  SDKVIS_EXPORT void convertNWUtoENU(osg::Node* node);

  /**
   * True if the filename is that of a known image type; false otherwise.
   * @param[in ] location Filename or URI.
   * @return     True or false
   */
  SDKVIS_EXPORT bool isImageFile(const std::string& location);

#ifdef USE_DEPRECATED_SIMDISSDK_API
  /**
  * Finds the full path of the font file, searching in SIMDIS installed fonts locations.
  * Provided as a convenience method to simVis::Registry::instance()->findFontFile().
  * This method is deprecated and may be removed in a future release.  Please use the
  * Registry method instead.
  * @param fontFile file name
  * @return string  full path of file
  * @deprecated Use simVis::Registry::findFontFile() instead.
  */
  SDKVIS_EXPORT std::string findFontFile(const std::string& fontFile);
#endif

  /**
   * Convert simData DistanceUnits to osgEarth::Units.
   */
  SDKVIS_EXPORT osgEarth::Units convertUnitsToOsgEarth(const simData::DistanceUnits& input);

  /**
   * Convert simData SpeedUnits to osgEarth::Units.
   */
  SDKVIS_EXPORT osgEarth::Units convertUnitsToOsgEarth(const simData::SpeedUnits& input);

  /// Returns the thickness associated with the TextOutline setting in pixels.
  SDKVIS_EXPORT float outlineThickness(simData::TextOutline outline);

  /// Returns font size that scales relative to SIMDIS 9
  SDKVIS_EXPORT float osgFontSize(float simFontSize);

  /// Returns font size that scales from OSG to SIMDIS 9
  SDKVIS_EXPORT float simdisFontSize(float osgFontSize);

  /// Converts from protobuf label backdrop type to OSG backdrop type
  SDKVIS_EXPORT osgText::Text::BackdropType backdropType(simData::BackdropType type);

  /// Converts from protobuf label backdrop implementation to OSG backdrop implementation
  SDKVIS_EXPORT osgText::Text::BackdropImplementation backdropImplementation(simData::BackdropImplementation implementation);

#if 0
  /**
  * Builds an ellipsoid geometry.
  */
  SDKVIS_EXPORT osg::Geometry* createEllipsoid(
    double major,
    double minor,
    int    segments,
    const  osg::Vec4& color);
#endif

  /// Update a platform simulator using the OSG frame timer.
  struct SimulatorEventHandler : public osgGA::GUIEventHandler
  {
  public:
    /// Constructs a new SimulatorEventHandler
    SimulatorEventHandler(simUtil::PlatformSimulatorManager *simMgr, double startTime, double endTime, bool loop = true)
      : simMgr_(simMgr), startTime_(startTime), endTime_(endTime),
      currentTime_(startTime),
      lastEventTime_(-1.0),
      loop_(loop),
      playing_(true)
    {
      //nop
    }

    /// handle an event
    bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
    {
      // handle FRAME events
      if (ea.getEventType() == ea.FRAME)
      {
        double t = ea.getTime();

        if (lastEventTime_ < 0.0)
          lastEventTime_ = t;

        if (playing_)
        {
          double delta = t - lastEventTime_;

          if (simMgr_->getDataStore()->getBoundClock() &&
              simMgr_->getDataStore()->getBoundClock()->timeDirection() == simCore::REVERSE)
          {
              currentTime_ -= delta;
          }
          else
          {
              currentTime_ += delta;
          }

          double simTime = loop_ ? fmod(currentTime_, (endTime_-startTime_)) : currentTime_;

          simMgr_->play(simTime);
        }

        lastEventTime_ = t;
      }

      // PLAY/PAUSE
      else if (ea.getEventType() == ea.KEYDOWN)
      {
        if (ea.getKey() == '.')
        {
          playing_ = !playing_;
        }
      }

      return false;
    }

    /// Changes the current time
    void setTime(double t)
    {
      currentTime_ = simCore::sdkMax(t, startTime_);
      lastEventTime_ = -1.0;
    }

    /// Retrieves the current time
    double getTime() const
    {
      return currentTime_;
    }

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }

    /** Return the class name */
    virtual const char* className() const { return "SimulatorEventHandler"; }

  protected:
    /// osg::Referenced-derived
    virtual ~SimulatorEventHandler() {}

  private:
    osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr_;
    double startTime_, endTime_, currentTime_, lastEventTime_;
    bool loop_;
    bool playing_;
  };

  /// Math helper functions
  struct SDKVIS_EXPORT Math
  {
    /**
    * @name simVis::eulerDegToQuat() and simVis::eulerRadToQuat()
    * Convert Euler angles to a quaternion.
    *
    * The input Euler angles must follow the simCore semantics. That is:
    *
    *   +azimuth   => right turn
    *   +elevation => nose up
    *   +roll      => right wing down
    *
    * Rotation order is Azim-Elev-Roll.
    *
    * @param hpr_deg Orientation angles
    * @return        Quaternion
    * @deprecated    Use simCore::d3EulertoQ instead.
    */
    static osg::Quat eulerDegToQuat(double h, double p, double r);
    static osg::Quat eulerDegToQuat(const osg::Vec3d& hpr) { return eulerDegToQuat(hpr[0], hpr[1], hpr[2]); }
    static osg::Quat eulerRadToQuat(double h, double p, double r);
    static osg::Quat eulerRadToQuat(const osg::Vec3d& hpr) { return eulerRadToQuat(hpr[0], hpr[1], hpr[2]); }
    ///@}

    /**
    * @name simVis::quatToEulerRad() and simVis::quatToEulerDeg()
    * Convert a quaternion to Euler angles (HPR in degrees)
    * @{
    * @param quat Quaternion
    * @return     Euler angles (HPR, degrees)
    * @deprecated Use simCore::d3QtoEuler instead.
    */
    static osg::Vec3d quatToEulerRad(const osg::Quat& quat);
    static osg::Vec3d quatToEulerDeg(const osg::Quat& quat);
    ///@}

    /**
    * Clamp the orientation of a matrix to the specified Euler angles.
    * The matrix cannot be scaled, else this will return incorrect values.
    * @param mat         Matrix within which to clamp values (input/output)
    * @param min_hpr_deg Minimum allowable Euler angles (HPR, degrees)
    * @param max_hpr_deg Maximum allowable Euler angles (HRP, degrees)
    */
    static void clampMatrixOrientation(osg::Matrixd& mat, osg::Vec3d& min_hpr_deg, osg::Vec3d& max_hpr_deg);

    /**
    * Quaternion spherical linear interpolator - for sweeping one quat onto another
    * when creating 3D arcs (pie slices)
    */
    struct QuatSlerp
    {
      /** Current interpolated state */
      osg::Quat quat_;
      /** Initial state from which to base interpolation */
      osg::Quat from_;
      /** Target state to which to interpolate */
      osg::Quat to_;
      /** Flags whether to take the long way */
      bool longWay_;
      /** Cos of the angle of rotation */
      double cosAngle_;
      /** Caches the sin of angle of rotation */
      double sinAngle_;
      /** Angle of rotation */
      double angle_;

      /** Constructs a new QuatSlerp between two angles */
      QuatSlerp(const osg::Quat& from, const osg::Quat& to, bool longWay)
        : from_(from), to_(to), longWay_(longWay), sinAngle_(0.0), angle_(0)
      {
        cosAngle_ = from.asVec4() * to.asVec4();
        if ((cosAngle_ < 0.0 && !longWay_) || (cosAngle_ > 0.0 && longWay_))
        {
          cosAngle_ = -cosAngle_;
          to_ = -to_;
        }

        if ((1.0-cosAngle_) > 0.00001)
        {
          angle_ = acos(cosAngle_);
          sinAngle_ = sin(angle_);
        }
      }

      /// functor operation
      osg::Quat& operator()(double t)
      {
        double scaleFrom, scaleTo;

        if ((1.0-cosAngle_) > 0.00001)
        {
          assert(sinAngle_ != 0.0);
          scaleFrom = sin((1.0-t)*angle_)/sinAngle_;
          scaleTo   = sin(t*angle_)/sinAngle_;
        }
        else
        {
          scaleFrom = 1.0-t;
          scaleTo   = t;
        }

        quat_ = (from_*scaleFrom) + (to_*scaleTo);
        return quat_;
      }
    };
  };

  /** Map of color index to OSG (RGBA) color */
  typedef std::map<int, osg::Vec4> ColorMap;

  /** Color helper functions */
  struct SDKVIS_EXPORT ColorUtils
  {
    /** Maps colors for gain threshold */
    ColorMap gainThresholdColorMap_;

    ///Default constructor for use when accessing non-static members
    ColorUtils(float gainAlpha=1.0);

    /**
    * Convert the 0xRRGGBBAA color value used by SIMDIS to an OSG Vec4f object. With this representation,
    * red is stored in bits (24-31), blue is stored in bits (16-23), green is stored in bits (8-15), and
    * alpha is stored in bits (0-7) of an unsigned integer.
    * @param color Integer value containing a color value encoded with the SIMDIS 0xRRGGBBAA representation
    * @return      osg::Vec4f representation of the specified color value
    */
    static osg::Vec4f RgbaToVec4(unsigned int color);

    /// set the color for the gain algorithmically
    static void GainThresholdColor(int gain, osg::Vec4f &color, float alpha=0.3);
    /// set the color for the gain using a table
    const osg::Vec4f& GainThresholdColor(int gain);
  };


  /** Helper class to unify platform-vector scaling factors */
  class SDKVIS_EXPORT VectorScaling
  {
  public:
    /** Generates scene points between start and end (inclusive), using numPointsPerLine */
    static void generatePoints(osg::Vec3Array& vertices, const osg::Vec3& start, const osg::Vec3& end, int numPointsPerLine);
    /** Returns true if one of the prefs has changed that impacts vector scaling (requiring line rebuild) */
    static bool fieldsChanged(const simData::PlatformPrefs& lastPrefs, const simData::PlatformPrefs& newPrefs);
    /** Returns the line length of the platform node's vector, based on axis scale and model size */
    static float lineLength(const simVis::PlatformModelNode* node, float axisScale=1.f);

    /** Returns the size of the bounding box passed in */
    static osg::Vec3f boundingBoxSize(const osg::BoundingBox& bbox);
    /** Returns the maximum dimension of the bounding box */
    static float boundingBoxMaxDimension(const osg::BoundingBox& bbox);
  };


  /// makes a big red "X" square image for the given size in pixels
  SDKVIS_EXPORT osg::Image* makeBrokenImage(int size=32);

  /**
   * Computes the world matrix for a node, using its local matrix.
   * @param node Node for which to get the world coordinates
   * @return 4x4 matrix from OSG representing the world coordinates for the node
   */
  SDKVIS_EXPORT osg::Matrix computeLocalToWorld(const osg::Node* node);

  /**
   * Calculates the geodetic position of a node in the scene (radians and meters)
   * @param node Scene graph node for which to calculate the geodetic position
   * @return Geodetic position (latitude, longitude, altitude) in radians and meters
   */
  SDKVIS_EXPORT simCore::Vec3 computeNodeGeodeticPosition(const osg::Node* node);


  /**
   * Node callback that fakes an always-increasing frame stamp.  Intended to be used to update Sequence nodes.
   * Sequence nodes requiring a strictly increasing simulation time stamp.  Time stamps that decrease will
   * break the Sequence update logic (as of 3.4.0 at least).  However, other software like osgEarth's Triton
   * extension require the ability to tie simulation time to the real simulation time, which in SIMDIS is
   * not strictly increasing.
   *
   * This class attaches to the update operation and replaces the time stamp with one that is strictly
   * increasing based on the system time.  This impacts Sequence nodes and all children of Sequence nodes.
   */
  class SDKVIS_EXPORT SequenceTimeUpdater : public osg::NodeCallback
  {
  public:
    /** Initializes the Sequence Time Updater */
    SequenceTimeUpdater(osg::FrameStamp* replacementStamp);

    /** Changes the frame stamp pointer being used for the scene */
    void setFrameStamp(osg::FrameStamp* frameStamp);

    /** Applies a strictly increasing time stamp to the sequence */
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

  protected:
    /** Protected osg::Referenced-derived destructor */
    virtual ~SequenceTimeUpdater();

  private:
    /** Copies the scene stamp data to modified stamp, then modifies the sim time to match ref time */
    void updateModifiedStamp_();

    osg::observer_ptr<osg::FrameStamp> sceneStamp_;
    osg::ref_ptr<osg::FrameStamp> modifiedStamp_;
  };

} // namespace simVis

#endif // SIMVIS_UTILS_H
