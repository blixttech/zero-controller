#pragma once

#include <qwt_series_data.h>


namespace zero {
class ZeroDataStream: public QwtArraySeriesData<QPointF> 
{
	public:
		ZeroDataStream(uint max_size = 250) : QwtArraySeriesData<QPointF>(),
		b_rect_stale(true),
		max_size(max_size)
		{
		}

	// return the bouding rect of the data
  virtual QRectF boundingRect() const {
  	if ( b_rect_stale ) {
	  	cachedBoundingRect = qwtBoundingRect( *this );
			b_rect_stale = false;
  	}
  	return cachedBoundingRect;
  }
		
	// append a new point to the data stream
	inline void append( const QPointF &point ) {
		m_samples += point;
		for (int i = 0; m_samples.length() > max_size; ++i)
			m_samples.takeFirst();
		b_rect_stale = true;
	}

	// remove all points from the data stream
	void clear() {
		m_samples.clear();
		m_samples.squeeze();
		// it invalidate the bounding rect
		cachedBoundingRect = QRectF( 0.0, 0.0, -1.0, -1.0 );
		b_rect_stale = false;
	}

	private:
		mutable bool b_rect_stale;
		uint max_size;
};
}
