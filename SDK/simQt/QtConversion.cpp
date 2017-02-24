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
#include "simQt/QtConversion.h"

namespace simQt {

  QColor getQtColorFromOsg(const osg::Vec4f& colorVec)
  {
    // color must be converted from 0.0-1.0 to QColor int of 0-255
    return QColor(static_cast<int>(colorVec[0]*255.0), static_cast<int>(colorVec[1]*255.0),
        static_cast<int>(colorVec[2]*255.0), static_cast<int>(colorVec[3]*255.0));
  }

  osg::Vec4f getOsgColorFromQt(const QColor& color)
  {
    return osg::Vec4f(static_cast<float>(color.red())/255.0, static_cast<float>(color.green())/255.0, static_cast<float>(color.blue())/255.0, static_cast<float>(color.alpha())/255.0);
  }

  QColor getQColorFromQString(const QString& qstr)
  {
    QStringList rgba = qstr.split(",");
    return QColor(rgba[0].toInt(), rgba[1].toInt(), rgba[2].toInt(), rgba[3].toInt());
  }

  QString getQStringFromQColor(const QColor& color)
  {
    return QString("%1,%2,%3,%4").arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
  }

  QString formatTooltip(const QString& title, const QString& desc)
  {
    if (desc.isEmpty())
      return QString("<strong>%1</strong>").arg(title);
    const QString formatBlock("<strong>%1</strong><div style=\"margin-left: 1em; margin-right: 1em;\"><p>%2</p></div>");
    return formatBlock.arg(title, desc);
  }
}
