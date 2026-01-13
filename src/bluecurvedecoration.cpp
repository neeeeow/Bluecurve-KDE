#include "bluecurvedecoration.h"
#include "bitmaps.h"

#include <KDecoration3/Decoration>
#include <KDecoration3/DecorationSettings>
#include <KPluginFactory>

#include <QPainter>
#include <QBitmap>
#include <qdrawutil.h>

#define BASE_BUTTON_SIZE  17
#define TITLE_HEIGHT      17
#define BORDER_WIDTH      6
#define CORNER_RADIUS     12

#define BUTTON_DIAM       12
#define TOP_GRABBAR_WIDTH 2
#define BOTTOM_CORNER     5

QPixmap* titlePix;
QPixmap* aUpperGradient;
QPixmap* iUpperGradient;

QPixmap* pinDownPix;
QPixmap* pinUpPix;
QPixmap* ipinDownPix;
QPixmap* ipinUpPix;

QPixmap* btnUpPix;
QPixmap* btnDownPix;
QPixmap* ibtnUpPix;
QPixmap* ibtnDownPix;

QPixmap* bottomLeftPix;
QPixmap* bottomRightPix;
QPixmap* abottomLeftPix;
QPixmap* abottomRightPix;

K_PLUGIN_FACTORY_WITH_JSON(
	BluecurveDecorationFactory,
	"metadata.json",
	registerPlugin<BluecurveDecoration>();
	)

static void
pixmapGradient(QPixmap *pixmap, QColor c1, QColor c2, qreal x1, qreal y1, qreal x2, qreal y2)
{
	// helper function for drawing gradients to pixmaps, replaces KPixmapEffect::gradient
	QLinearGradient gradient(x1, y1, x2, y2);
	gradient.setColorAt(0.0, c1);
	gradient.setColorAt(1.0, c2);

	QPainter gradientPainter(pixmap);
	gradientPainter.fillRect(pixmap->rect(), gradient);
	gradientPainter.end();
}

BluecurveDecoration::BluecurveDecoration(QObject *parent, const QVariantList &args) : KDecoration3::Decoration(parent, args)
{

}

BluecurveDecoration::~BluecurveDecoration() = default;

bool
BluecurveDecoration::init()
{
	createPixmaps();
	updateBorders();
	updateTitleBar();
	
	return true;
}


void
BluecurveDecoration::createPixmaps()
{
	QPainter p;
	
	// Titlebar stipple
	QPainter maskPainter;
	int x, y;
	titlePix = new QPixmap(132, TITLE_HEIGHT+2);
	titlePix->fill(Qt::red);
	QBitmap mask(132, TITLE_HEIGHT+2);

	mask.fill(Qt::color0);
	maskPainter.begin(&mask);
	maskPainter.setPen(Qt::color1);

	QColor lighterColor(window()->color(KDecoration3::ColorGroup::Active,
										KDecoration3::ColorRole::TitleBar).lighter(150));
	int h, s, v;
	lighterColor.getHsv (&h, &s, &v);
	s /= 2;
	s = (s > 255) ? 255 : (int) s;
    QColor satColor = QColor::fromHsv(h, s, v);

	pixmapGradient(titlePix, satColor, satColor.darker(150),
				   0, 0, 0, titlePix->height());

	for(y = 0; y < (TITLE_HEIGHT+2); y++) {
		for(x = (3 - y) % 4; x < 132; x += 4) {
			maskPainter.drawPoint(x, y);
		}
	}
	
	maskPainter.end();
	titlePix->setMask(mask);

	// Create titlebar gradient images if required
	aUpperGradient = NULL;
	iUpperGradient = NULL;

	auto colorBitmapLayer = [&](const unsigned char *bits, const QColor &color) {
		// lambda to draw in each layer that kColorBitmaps would draw in
		// on KDE 3.

		if (!bits)
			return;

		QBitmap mask = QBitmap::fromData(
			QSize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE),
			bits,
			QImage::Format_MonoLSB
			);

		p.setPen(color);
		p.drawPixmap(0, 0, mask);
	};

	// Active pins
	pinUpPix = new QPixmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	pinUpPix->fill(Qt::transparent);
	p.begin( pinUpPix );
	colorBitmapLayer(pinup_white_bits, window()->color(QPalette::Active, QPalette::Light));
	colorBitmapLayer(pinup_gray_bits, window()->color(QPalette::Active, QPalette::Button));
	colorBitmapLayer(pinup_dgray_bits, window()->color(QPalette::Active, QPalette::Dark));
	p.end();
	pinUpPix->setMask(QBitmap::fromData(
						  QSize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE),
						  pinup_mask_bits, QImage::Format_MonoLSB));

	pinDownPix = new QPixmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	pinDownPix->fill(Qt::transparent);
	p.begin( pinDownPix );
	colorBitmapLayer(pindown_white_bits, window()->color(QPalette::Active, QPalette::Light));
	colorBitmapLayer(pindown_gray_bits, window()->color(QPalette::Active, QPalette::Button));
	colorBitmapLayer(pindown_dgray_bits, window()->color(QPalette::Active, QPalette::Dark));
	p.end();
	pinDownPix->setMask(QBitmap::fromData(
						  QSize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE),
						  pindown_mask_bits, QImage::Format_MonoLSB));

	// Inactive pins
	ipinUpPix = new QPixmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	ipinUpPix->fill(Qt::transparent);
	p.begin( ipinUpPix );
	colorBitmapLayer(pinup_white_bits, window()->color(QPalette::Inactive, QPalette::Light));
	colorBitmapLayer(pinup_gray_bits, window()->color(QPalette::Inactive, QPalette::Button));
	colorBitmapLayer(pinup_dgray_bits, window()->color(QPalette::Inactive, QPalette::Dark));
	p.end();
	ipinUpPix->setMask(QBitmap::fromData(
						   QSize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE),
						   pinup_mask_bits, QImage::Format_MonoLSB));

	ipinDownPix = new QPixmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	ipinDownPix->fill(Qt::transparent);
	p.begin( ipinDownPix );
	colorBitmapLayer(pindown_white_bits, window()->color(QPalette::Inactive, QPalette::Light));
	colorBitmapLayer(pindown_gray_bits, window()->color(QPalette::Inactive, QPalette::Button));
	colorBitmapLayer(pindown_dgray_bits, window()->color(QPalette::Inactive, QPalette::Dark));
	p.end();
	ipinDownPix->setMask(QBitmap::fromData(
							 QSize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE),
							 pindown_mask_bits, QImage::Format_MonoLSB));

	// Cache all possible button states
	btnUpPix = new QPixmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	btnUpPix->fill(Qt::transparent);
	btnDownPix = new QPixmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	btnDownPix->fill(Qt::transparent);
	ibtnUpPix = new QPixmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	ibtnUpPix->fill(Qt::transparent);
	ibtnDownPix = new QPixmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	ibtnDownPix->fill(Qt::transparent);

	auto drawButtonBackground = [&](QPixmap *pixmap, bool sunken, bool active) {
		
		QColor c = window()->color(QPalette::Active, QPalette::Button);
		if (active) {
			pixmapGradient(pixmap, c, Qt::white,
						   0, 0, pixmap->width(), pixmap->height());
		} else {
			QColor inactiveTitleColor1(window()->color(KDecoration3::ColorGroup::Inactive,
													   KDecoration3::ColorRole::TitleBar));
		    QColor inactiveTitleColor2(window()->color(KDecoration3::ColorGroup::Inactive,
													   KDecoration3::ColorRole::TitleBar).darker(110));

			pixmapGradient(pixmap, inactiveTitleColor2, inactiveTitleColor1,
						   0, 0, pixmap->width(), pixmap->height());
		}
	};

	drawButtonBackground( btnUpPix, false, true );
	drawButtonBackground( btnDownPix, true, true );
	drawButtonBackground( ibtnUpPix, false, false );
	drawButtonBackground( ibtnDownPix, true, false );

	QImage bottomleft(bottom_left_xpm);
	QImage bottomright(bottom_right_xpm);
	QImage abottomleft(bottom_left_xpm);
	QImage abottomright(bottom_right_xpm);

	auto recolor = [&](QImage &img, const QColor &color) {
		int hue = -1, sat = 0, val = 228;
		if ( color.isValid() )
			color.getHsv( &hue, &sat, &val );
		QVector<QRgb> colorTable = img.colorTable();
        int pixels = colorTable.size();
        for (int i = 0; i < pixels; ++i) {
            QColor c(colorTable[i]);
            int h, s, v;
            c.getHsv(&h, &s, &v);
            h = hue;
            s = sat;
            v = v * val / 145;
            c.setHsv(h, qMin(s, 255), qMin(v, 255));
            colorTable[i] = (c.rgb() & RGB_MASK) | (colorTable[i] & ~RGB_MASK);
        }
        img.setColorTable(colorTable);
		
	};

	recolor(bottomleft, window()->color(KDecoration3::ColorGroup::Inactive,
										KDecoration3::ColorRole::TitleBar).lighter(95));
	recolor(bottomright, window()->color(KDecoration3::ColorGroup::Inactive,
										 KDecoration3::ColorRole::TitleBar).lighter(95));
	recolor(abottomleft, window()->color(KDecoration3::ColorGroup::Active,
										 KDecoration3::ColorRole::TitleBar).lighter(135));
	recolor(abottomright, window()->color(KDecoration3::ColorGroup::Active,
										  KDecoration3::ColorRole::TitleBar).lighter(135));

	bottomLeftPix 	= new QPixmap();
	bottomLeftPix->fill(Qt::transparent);
	bottomRightPix	= new QPixmap();
	bottomRightPix->fill(Qt::transparent);
	abottomLeftPix	= new QPixmap();
	abottomLeftPix->fill(Qt::transparent);
	abottomRightPix	= new QPixmap();
	abottomRightPix->fill(Qt::transparent);
	bottomLeftPix->convertFromImage(bottomleft);
	bottomRightPix->convertFromImage(bottomright);
	abottomLeftPix->convertFromImage(abottomleft);
	abottomRightPix->convertFromImage(abottomright);
}

void
BluecurveDecoration::updateBorders()
{
	int left, right, top, bottom;
	left = right = bottom = BORDER_WIDTH;
	top = TITLE_HEIGHT + 3;
	setBorders(QMargins(left, top, right, bottom)); 
}

void
BluecurveDecoration::updateTitleBar()
{
	const int width = window()->width();
	const int height = TITLE_HEIGHT;
	const int x = 0;
	const int y = 0;
	setTitleBar(QRect(x,y,width,height));
}

void
BluecurveDecoration::paint(QPainter *p, const QRectF &repaintRegion)
{
	
	bool drawLeftDivider = true; 
	bool drawRightDivider = true;

	// Obtain widget bounds
	int x = rect().x();
	int y = rect().y();
	int x2 = x + rect().width() - 1;
	int y2 = y + rect().height() - 1;
	int w  = rect().width();
	int h  = rect().height();
	
	// Titlebar rectangle
	QRectF r(QRect(0, 0, w, TITLE_HEIGHT));
	
	/*QColor c2 = window()->color(window()->isActive() // this color doesn't seem to be used anywhere
								? KDecoration3::ColorGroup::Active
								: KDecoration3::ColorGroup::Inactive,
								KDecoration3::ColorRole::Frame);*/

	// Buffer for the decoration (which we apply the mask later)
	QPixmap decoBuffer = QPixmap(w, h);
	decoBuffer.fill(Qt::transparent);

	// Create a disposable pixmap buffer for the titlebar
	// very early before drawing begins so there is no lag
	// during painting pixels.
	QPixmap titleBuffer = QPixmap(w, TITLE_HEIGHT + TOP_GRABBAR_WIDTH);
	titleBuffer.fill(Qt::transparent);

	// Obtain titlebar blend colours
	QColor c1 = window()->color(window()->isActive()
								? KDecoration3::ColorGroup::Active
								: KDecoration3::ColorGroup::Inactive,
								KDecoration3::ColorRole::TitleBar);

	QPainter p2(&titleBuffer);
	QColor activeTitleColor1(window()->color(KDecoration3::ColorGroup::Active,
											 KDecoration3::ColorRole::TitleBar));
	QColor activeTitleColor2(window()->color(KDecoration3::ColorGroup::Active,
											 KDecoration3::ColorRole::TitleBar).darker(110));
	
	QColor inactiveTitleColor1(window()->color(KDecoration3::ColorGroup::Inactive,
											   KDecoration3::ColorRole::TitleBar));
	QColor inactiveTitleColor2(window()->color(KDecoration3::ColorGroup::Inactive,
											   KDecoration3::ColorRole::TitleBar).darker(110));

	// Old theme checked for highcolor, but we assume always true.
	static QSize oldsize(0,0);
	QSize titleBufferSize(w, TITLE_HEIGHT + TOP_GRABBAR_WIDTH);

	if (oldsize != titleBufferSize) {
		oldsize = titleBufferSize;
		if (aUpperGradient)
		{
			delete aUpperGradient;
			aUpperGradient = NULL;
		}
		if (iUpperGradient)
		{
			delete iUpperGradient;
			iUpperGradient = NULL;
		}

		// Create the titlebar gradients
		if (activeTitleColor1 != activeTitleColor2)
		{
			aUpperGradient = new QPixmap(oldsize);
			aUpperGradient->fill(Qt::transparent);
			pixmapGradient(aUpperGradient, activeTitleColor2, activeTitleColor1,
						   0, 0, 0, aUpperGradient->height());
		}

		if (inactiveTitleColor1 != inactiveTitleColor2)
		{
			iUpperGradient = new QPixmap(oldsize);
			iUpperGradient->fill(Qt::transparent);
			pixmapGradient(iUpperGradient, inactiveTitleColor2, inactiveTitleColor1,
						   0, 0, 0, iUpperGradient->height());
		}
	}

	QPixmap* upperGradient = window()->isActive() ? aUpperGradient : iUpperGradient;
	
	if (upperGradient)
		p2.drawPixmap(0, TOP_GRABBAR_WIDTH, *upperGradient);
	else
		p2.fillRect(0, TOP_GRABBAR_WIDTH, w, TITLE_HEIGHT, c1);

	QFont fnt = settings()->font();
	p2.setFont( fnt );

	// Draw the titlebar stipple if active and available
	if (window()->isActive() && titlePix) {
		QFontMetrics fm(fnt);
		int captionWidth = fm.horizontalAdvance(window()->caption()) + 1;
		p2.drawTiledPixmap( r.x() + 2 + 2 + captionWidth, TOP_GRABBAR_WIDTH,
							r.width() - 2 - 4 - captionWidth, 
							TITLE_HEIGHT+1, *titlePix );
	}

	if (window()->isActive()) {
		p2.setPen(window()->color(KDecoration3::ColorGroup::Active,
								  KDecoration3::ColorRole::TitleBar).darker(110).darker());		
		p2.drawText(r.x() + 2 + 1, TOP_GRABBAR_WIDTH + 1,
					r.width() - 2 - 1, r.height(),
					Qt::AlignLeft | Qt::AlignVCenter, window()->caption() );
	}

    p2.setPen(window()->color(window()->isActive()
							  ? QPalette::ColorGroup::Active
							  : QPalette::ColorGroup::Inactive,
							  QPalette::ColorRole::Text));
	p2.drawText(r.x() + 2, TOP_GRABBAR_WIDTH,
				r.width() - 2, r.height(),
				Qt::AlignLeft | Qt::AlignVCenter, window()->caption() );

	// Main Title Bar background area
	p2.setPen(Qt::white);
	p2.drawLine(x + 1, y + 1, x2 - 1, y + 1);
	// This is kind of broken...
	// We fill in the inner part of the circle here.  This is dependent on BUTTON_DIAM
	p2.drawLine(x + 1, y + 1, x + 1, y + TOP_GRABBAR_WIDTH + TITLE_HEIGHT);
	p2.drawLine(x + 2, y + 2, x + 3, y + 2);
	p2.drawLine(x + 2, y + 2, x + 2, y + 3);
	p2.drawLine(x + w - 2 , y + 1, x + w - 2, y + TOP_GRABBAR_WIDTH + TITLE_HEIGHT);
	p2.drawLine(x + w - 3, y + 2, x + w - 3, y + 5);
	p2.drawLine(x + w - 4, y + 2, x + w - 3, y + 2);

	if (window()->isActive()) {
		QColor lighterColor (window()->color(KDecoration3::ColorGroup::Active,
											 KDecoration3::ColorRole::TitleBar).lighter(150));
		p2.setPen (lighterColor);
		p2.drawLine (r.x(), 2, r.x() + r.width(), 2);
		int h, s, v;
		lighterColor.getHsv (&h, &s, &v);
		s /= 2;
		s = (s > 255) ? 255 : (int) s;

		QColor satColor = QColor::fromHsv(h, s, v);
		p2.setPen (satColor);
		p2.drawLine (r.x(), 1, r.x() + r.width() - 2, 1);
	}

	p2.setPen(Qt::white);
	/*if (isActive()) TODO: port this code once button logic is added
	{
		for (int i = 0; i < BtnCount; i ++)
		{
			if (button[i] == NULL)
				continue;
			if (!button[i]->isVisible())
			{
				if (button[i]->pos == ButtonRight)
					drawRightDivider = false;
				// FIXME: Should be LeftButtonLeft if we had it
				if (button[i]->pos == LeftButtonRight)
					drawLeftDivider = false;
				continue;
			}
			QRect buttonSize = button[i]->geometry ();
			p2.setPen(Qt::white);
			p2.drawLine (buttonSize.x() - 1, TOP_GRABBAR_WIDTH,
						 buttonSize.x() - 1, TOP_GRABBAR_WIDTH + titleHeight);
			if (button[i]->pos == ButtonRight)
				continue;
			else if (button[i]->pos == LeftButtonRight)
				p2.setPen(g.mid().light(120));
			else
				p2.setPen(g.dark());
			p2.drawLine (buttonSize.x() + buttonSize.width(), TOP_GRABBAR_WIDTH - 1,
						 buttonSize.x() + buttonSize.width(), TOP_GRABBAR_WIDTH + titleHeight);
		}
		}*/

	// Top Left Button Area
	if (drawLeftDivider)
	{
		if (window()->isActive())
			p2.setPen(window()->color(KDecoration3::ColorGroup::Active,
									  KDecoration3::ColorRole::TitleBar).darker(150));
		else
			p2.setPen(window()->palette().mid().color());
		p2.drawLine (r.x() , y + 1, r.x() , y + TITLE_HEIGHT + TOP_GRABBAR_WIDTH);
	}

	// Top Right Button Area
	if (drawRightDivider)
	{
		if (window()->isActive())
			p2.setPen(window()->color(KDecoration3::ColorGroup::Active,
									  KDecoration3::ColorRole::TitleBar).darker(150));
		else
		    p2.setPen(window()->palette().mid().color());
		p2.drawLine (r.x() + r.width() - 2, y + 1,
					 r.x() + r.width() - 2 , y + TITLE_HEIGHT + TOP_GRABBAR_WIDTH);
	}

	// Black outer line
	p2.setPen(Qt::black);
	p2.drawRect(0,0,w-1,h-1);
	p2.drawArc(x, y, BUTTON_DIAM, BUTTON_DIAM, 90*16, 90*16);
	p2.drawArc(x + w - BUTTON_DIAM , y, BUTTON_DIAM, BUTTON_DIAM, 0*16, 90*16);
	p2.end();

	QPainter p1(&decoBuffer); // Painter for the main decoration buffer
	
	int sideStart = TITLE_HEIGHT + TOP_GRABBAR_WIDTH + 1;
					
	// Draw the left and right sides
  
	// Fill the left side first
	qDrawShadePanel(&p1,
					// We compensate for the top and bottom parts of the bevel
					// by drawing 1 pixel below and above the frame part
					x + 1, y + (sideStart - 1),
					BORDER_WIDTH - 1, h - (sideStart + 2),
					window()->palette(), false, 1, &window()->palette().window());

	
	// Right Side
	qDrawShadePanel(&p1,
					x2 - (BORDER_WIDTH - 2), y + (sideStart - 1),
					BORDER_WIDTH - 2, h - (sideStart + 2),
					window()->palette(), false, 1, &window()->palette().window());

	
	p1.setPen(window()->palette().dark().color());
	p1.drawLine(x2 - (BORDER_WIDTH - 1), sideStart, x2 - (BORDER_WIDTH - 1), h - sideStart);

	// Draw the bottom
	qDrawShadePanel(&p1,
					x, y2 - (BORDER_WIDTH - 2),
					w, (BORDER_WIDTH - 2),
				    window()->palette(), false, 1, &window()->palette().window());
    p1.setPen(window()->palette().dark().color());
	p1.drawLine(x, y2 - (BORDER_WIDTH - 1), x2, y2 - (BORDER_WIDTH - 1));

	// Line above the app and below the title bar
    p1.setPen(window()->palette().dark().color());
	p1.drawLine(x, y + TITLE_HEIGHT + TOP_GRABBAR_WIDTH,
				x2, y + TITLE_HEIGHT + TOP_GRABBAR_WIDTH);

	// Draw the title buffer
	p1.drawPixmap(x, y, titleBuffer);

	// Draw an outer black frame
	p1.setPen(Qt::black);
	p1.drawRect(0,0,w-1,h-1);

	// Put on the bottom corners
	p1.drawPixmap(0, h - bottomLeftPix->height(),
				  window()->isActive() ? *abottomLeftPix : *bottomLeftPix);
	p1.drawPixmap(w - bottomRightPix->width(), h - bottomRightPix->height(), 
				  window()->isActive() ? *abottomRightPix : *bottomRightPix);

	p1.end();
	
	// Apply the mask to the decoration buffer and draw it
	decoBuffer.setMask(decorationMask());
	p->drawPixmap(x,y, decoBuffer);
	
}

QBitmap
BluecurveDecoration::decorationMask()
{
	// Obtain widget bounds
	int x = rect().x();
	int y = rect().y();
	int w  = rect().width();
	int h  = rect().height();

	int rad = BUTTON_DIAM / 2;
	int dm = BUTTON_DIAM;

	QBitmap mask(w, h);
	mask.clear();

	QPainter p(&mask);

	p.fillRect(x, y, w, h, Qt::color1);

	p.eraseRect(x, y, rad, rad);
	p.eraseRect(w-rad+1, 0, rad, rad);

	p.eraseRect(x, h-BOTTOM_CORNER, BOTTOM_CORNER, BOTTOM_CORNER);
	p.eraseRect(w-BOTTOM_CORNER, h-BOTTOM_CORNER, BOTTOM_CORNER, BOTTOM_CORNER);

	p.setPen(Qt::color1);
	p.setBrush(Qt::color1);

	p.drawPie(x, y, dm, dm, 90*16, 90*16);
	p.drawArc(x, y, dm, dm, 90*16, 90*16);

	p.drawPie(w-dm, 0, dm, dm, 0*16, 90*16);
	p.drawArc(w-dm, 0, dm, dm, 0*16, 90*16);

	p.drawPixmap(x, h - bottomLeftPix->height(), bottomLeftPix->mask());

	p.drawPixmap(w-bottomRightPix->width(), h - bottomRightPix->height(), 
				 bottomRightPix->mask());

	p.fillRect(x+BOTTOM_CORNER, h - bottomLeftPix->height(),
			   bottomLeftPix->width()-BOTTOM_CORNER,
			   bottomLeftPix->height()-BOTTOM_CORNER,
			   Qt::color1);

	p.fillRect(w-bottomRightPix->width(), h - bottomRightPix->height(), 
			   bottomRightPix->width()-BOTTOM_CORNER,
			   bottomRightPix->height()-BOTTOM_CORNER,
			   Qt::color1);

	p.end();
	return mask;
}

BluecurveButton::BluecurveButton(KDecoration3::DecorationButtonType type,
								 KDecoration3::Decoration *decoration,
								 QObject *parent)
	: KDecoration3::DecorationButton(type, decoration, parent)
{

}

BluecurveButton::~BluecurveButton() = default;

void
BluecurveButton::paint(QPainter *p, const QRectF &repaintRegion)
{

}

#include "bluecurvedecoration.moc"
