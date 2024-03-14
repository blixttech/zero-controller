#pragma once

#include <QString>
#include <QTime>

#include <QwtScaleDraw>
#include <QwtText>

namespace zero {
	
class TimeScaleDraw : public QwtScaleDraw
{
  public:
    TimeScaleDraw( const QTime& base, const QString& format = "hh:mm:ss")
        : baseTime( base )
    {
    }
    virtual QwtText label( double v ) const QWT_OVERRIDE
    {
        QTime upTime = baseTime.addMSecs( static_cast< int >( v ) );
        return upTime.toString();
    }
  private:
    QTime baseTime;
};
	
}

