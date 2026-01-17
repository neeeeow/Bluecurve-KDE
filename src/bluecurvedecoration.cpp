#include "bluecurvedecoration.h"
#include "bitmaps.h"

#include <KDecoration3/Decoration>
#include <KDecoration3/DecorationSettings>
#include <KPluginFactory>

#include <QPainter>
#include <QBitmap>
#include <QTimer>
#include <qdrawutil.h>

#define BASE_BUTTON_SIZE  17
#define TITLE_HEIGHT      17
#define BORDER_WIDTH      6
#define CORNER_RADIUS     12

#define BUTTON_DIAM       12
#define TOP_GRABBAR_WIDTH 2
#define BOTTOM_CORNER     5

/* these pixmaps are needed across every button, so we just store them globally for convenience */
QPixmap pinDownPix;
QPixmap pinUpPix;
QPixmap ipinDownPix;
QPixmap ipinUpPix;

QPixmap btnUpPix;
QPixmap btnDownPix;
QPixmap ibtnUpPix;
QPixmap ibtnDownPix;

K_PLUGIN_FACTORY_WITH_JSON(
	BluecurveDecorationFactory,
	"metadata.json",
	registerPlugin<BluecurveDecoration>();
	)

enum GradientType { VerticalGradient, HorizontalGradient,
	DiagonalGradient }; // we only need these three

static QPixmap&
pixmapGradient(QPixmap &pixmap, const QColor &ca, const QColor &cb,
					 GradientType eff, int ncols=3)
{
	/* Reimplementation of KDE 3's KPixmapEffect/KImageEffect::gradient
	   Copyright (C) 1998, 1999, 2001, 2002 Daniel M. Duley <mosfet@kde.org>
	   (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
	   (C) 1998, 1999 Dirk Mueller <mueller@kde.org>
	   (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
	   (C) 2000 Josef Weidendorfer <weidendo@in.tum.de>
	   (C) 2004 Zack Rusin <zack@kde.org>
	*/
	if (pixmap.depth()>8 &&
		(eff == VerticalGradient || eff == HorizontalGradient)) {

		int rDiff, gDiff, bDiff;
		int rca, gca, bca /*, rcb, gcb, bcb*/;

		int x, y;

		rDiff = (/*rcb = */ cb.red())   - (rca = ca.red());
		gDiff = (/*gcb = */ cb.green()) - (gca = ca.green());
		bDiff = (/*bcb = */ cb.blue())  - (bca = ca.blue());

		int rl = rca << 16;
		int gl = gca << 16;
		int bl = bca << 16;

		int rcdelta = ((1<<16) / (eff == VerticalGradient ? pixmap.height() : pixmap.width())) * rDiff;
		int gcdelta = ((1<<16) / (eff == VerticalGradient ? pixmap.height() : pixmap.width())) * gDiff;
		int bcdelta = ((1<<16) / (eff == VerticalGradient ? pixmap.height() : pixmap.width())) * bDiff;

		QPainter p(&pixmap);

		// these for-loops could be merged, but the if's in the inner loop
        // would make it slow
		switch(eff) {
		case VerticalGradient:
			for ( y = 0; y < pixmap.height(); y++ ) {
				rl += rcdelta;
				gl += gcdelta;
				bl += bcdelta;

				p.setPen(QColor(rl>>16, gl>>16, bl>>16));
				p.drawLine(0, y, pixmap.width()-1, y);
			}
			break;
		case HorizontalGradient:
			for( x = 0; x < pixmap.width(); x++) {
				rl += rcdelta;
				gl += gcdelta;
				bl += bcdelta;

				p.setPen(QColor(rl>>16, gl>>16, bl>>16));
				p.drawLine(x, 0, x, pixmap.height()-1);
			}
			break;
		default:
			;
		}

		p.end();
	} else if (eff==DiagonalGradient) {
		QImage image = pixmap.toImage();
		int rDiff, gDiff, bDiff;
		int rca, gca, bca, rcb, gcb, bcb;

		int x, y;

		rDiff = (rcb = cb.red())   - (rca = ca.red());
		gDiff = (gcb = cb.green()) - (gca = ca.green());
		bDiff = (bcb = cb.blue())  - (bca = ca.blue());

		float rfd, gfd, bfd;
		float rd = rca, gd = gca, bd = bca;

		unsigned char *xtable[3];
		unsigned char *ytable[3];

		unsigned int w = pixmap.width(), h = pixmap.height();
		xtable[0] = new unsigned char[w];
		xtable[1] = new unsigned char[w];
		xtable[2] = new unsigned char[w];
		ytable[0] = new unsigned char[h];
		ytable[1] = new unsigned char[h];
		ytable[2] = new unsigned char[h];
		w*=2, h*=2;

		rfd = (float)rDiff/w;
		gfd = (float)gDiff/w;
		bfd = (float)bDiff/w;

		int dir;
		for (x = 0; x < pixmap.width(); x++, rd+=rfd, gd+=gfd, bd+=bfd) {
			dir = eff == DiagonalGradient? x : pixmap.width() - x - 1;
			xtable[0][dir] = (unsigned char) rd;
			xtable[1][dir] = (unsigned char) gd;
			xtable[2][dir] = (unsigned char) bd;
		}

		rfd = (float)rDiff/h;
		gfd = (float)gDiff/h;
		bfd = (float)bDiff/h;
		rd = gd = bd = 0;
		for (y = 0; y < pixmap.height(); y++, rd+=rfd, gd+=gfd, bd+=bfd) {
			ytable[0][y] = (unsigned char) rd;
			ytable[1][y] = (unsigned char) gd;
			ytable[2][y] = (unsigned char) bd;
		}

		for (y = 0; y < pixmap.height(); y++) {
			unsigned int *scanline = (unsigned int *)image.scanLine(y);
			for (x = 0; x < pixmap.width(); x++) {
				scanline[x] = qRgb(xtable[0][x] + ytable[0][y],
								   xtable[1][x] + ytable[1][y],
								   xtable[2][x] + ytable[2][y]);
			}
		}

		delete [] xtable[0];
		delete [] xtable[1];
		delete [] xtable[2];
		delete [] ytable[0];
		delete [] ytable[1];
		delete [] ytable[2];

		// assume dithering isn't necessary, everyone has truecolor by now

		pixmap = QPixmap::fromImage(image);
	}

	return pixmap;
}

static QPixmap&
pixmapIntensity(QPixmap &pixmap, float percent)
{
	/* Reimplementation of KDE 3's KPixmapEffect/KImageEffect::intensity
	   Copyright (C) 1998, 1999, 2001, 2002 Daniel M. Duley <mosfet@kde.org>
	   (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
	   (C) 1998, 1999 Dirk Mueller <mueller@kde.org>
	   (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
	   (C) 2000 Josef Weidendorfer <weidendo@in.tum.de>
	   (C) 2004 Zack Rusin <zack@kde.org>
	*/

	QImage image = pixmap.toImage();

	int segColors = image.depth() > 8 ? 256 : image.colorCount();
	int pixels = image.depth() > 8 ? image.width() * image.height() :
		image.colorCount();
	unsigned int *data = image.depth() > 8 ? (unsigned int *)image.bits() :
		(unsigned int *)image.colorTable().data();
	
	bool brighten = (percent >= 0);
	if (percent < 0)
		percent = -percent;

	unsigned char *segTbl = new unsigned char[segColors];
	int tmp;
	if(brighten){ // keep overflow check out of loops
		for(int i=0; i < segColors; ++i){
			tmp = (int)(i*percent);
			if(tmp > 255)
				tmp = 255;
			segTbl[i] = tmp;
		}
	}
	else{
		for(int i=0; i < segColors; ++i){
			tmp = (int)(i*percent);
			if(tmp < 0)
				tmp = 0;
			segTbl[i] = tmp;
		}
	}

	if(brighten){ // same here
		for(int i=0; i < pixels; ++i){
			int r = qRed(data[i]);
			int g = qGreen(data[i]);
			int b = qBlue(data[i]);
			int a = qAlpha(data[i]);
			r = r + segTbl[r] > 255 ? 255 : r + segTbl[r];
			g = g + segTbl[g] > 255 ? 255 : g + segTbl[g];
			b = b + segTbl[b] > 255 ? 255 : b + segTbl[b];
			data[i] = qRgba(r, g, b,a);
		}
	}
	else{
		for(int i=0; i < pixels; ++i){
			int r = qRed(data[i]);
			int g = qGreen(data[i]);
			int b = qBlue(data[i]);
			int a = qAlpha(data[i]);
			r = r - segTbl[r] < 0 ? 0 : r - segTbl[r];
			g = g - segTbl[g] < 0 ? 0 : g - segTbl[g];
			b = b - segTbl[b] < 0 ? 0 : b - segTbl[b];
			data[i] = qRgba(r, g, b, a);
		}
	}
	delete [] segTbl;
	pixmap = QPixmap::fromImage(image);

	return pixmap;
}

static QColor
shade (const QColor &ca, double k) {
	float h, s, l;
	ca.getHslF(&h, &s, &l);

	s *= k;
	l *= k;

	QColor cb;
	cb.setHslF(h, qBound(0.0,s,1.0), qBound(0.0,l,1.0));
	return cb;
}

BluecurveDecoration::BluecurveDecoration(QObject *parent, const QVariantList &args) : KDecoration3::Decoration(parent, args)
{

}

BluecurveDecoration::~BluecurveDecoration() = default;

bool
BluecurveDecoration::init()
{
	createPixmaps();

	// borders are constant in size regardless of setting or state, so just set them here
    setBorders(QMargins(BORDER_WIDTH, TITLE_HEIGHT + 3, BORDER_WIDTH, BORDER_WIDTH));
	
	updateTitleBar();
	auto s = settings();

	/* Settings changes */
	// buttons
    connect(s.get(), &KDecoration3::DecorationSettings::spacingChanged, this, &BluecurveDecoration::updateButtonsGeometryDelayed);
    connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsLeftChanged, this, &BluecurveDecoration::updateButtonsGeometryDelayed);
    connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsRightChanged, this, &BluecurveDecoration::updateButtonsGeometryDelayed);

	/* Window state changes */
	// Update() signals
	connect(window(), &KDecoration3::DecoratedWindow::activeChanged, this, [this]{ update(); });

	// titleBar signals
    connect(window(), &KDecoration3::DecoratedWindow::widthChanged, this, &BluecurveDecoration::updateTitleBar);
    connect(window(), &KDecoration3::DecoratedWindow::maximizedChanged, this, &BluecurveDecoration::updateTitleBar);
	
	// Button signals. as a reminder: update() is called in updateButtonsGeometry
	connect(window(), &KDecoration3::DecoratedWindow::widthChanged, this, &BluecurveDecoration::updateButtonsGeometry);
    connect(window(), &KDecoration3::DecoratedWindow::maximizedChanged, this, &BluecurveDecoration::updateButtonsGeometry);
    connect(window(), &KDecoration3::DecoratedWindow::adjacentScreenEdgesChanged, this, &BluecurveDecoration::updateButtonsGeometry);
    connect(window(), &KDecoration3::DecoratedWindow::shadedChanged, this, &BluecurveDecoration::updateButtonsGeometry);

	// Create buttons
	m_leftButtons = new KDecoration3::DecorationButtonGroup(KDecoration3::DecorationButtonGroup::Position::Left,
															this, &BluecurveButton::create);
    m_rightButtons = new KDecoration3::DecorationButtonGroup(KDecoration3::DecorationButtonGroup::Position::Right,
															 this, &BluecurveButton::create);	
	updateButtonsGeometry();

	update();
	return true;
}


void
BluecurveDecoration::createPixmaps()
{
	QPainter p;
	
	// Titlebar stipple
	QPainter maskPainter;
	int x, y;
	titlePix = QPixmap(132, TITLE_HEIGHT+2);
	titlePix.fill(Qt::transparent);
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
				   VerticalGradient);

	for(y = 0; y < (TITLE_HEIGHT+2); y++) {
		for(x = (3 - y) % 4; x < 132; x += 4) {
			maskPainter.drawPoint(x, y);
		}
	}
	
	maskPainter.end();
	titlePix.setMask(mask);

	// Titlebar gradient images
	aTitleGradient = QPixmap(8, TITLE_HEIGHT + TOP_GRABBAR_WIDTH);
	iTitleGradient = QPixmap(8, TITLE_HEIGHT + TOP_GRABBAR_WIDTH);
    QColor activeTitleColor(window()->color(KDecoration3::ColorGroup::Active,
											KDecoration3::ColorRole::TitleBar));
	QColor inactiveTitleColor(window()->color(KDecoration3::ColorGroup::Inactive,
											  KDecoration3::ColorRole::TitleBar));
	pixmapGradient(aTitleGradient, shade(activeTitleColor, 1.4),
				   shade(activeTitleColor, 0.8), VerticalGradient);
	pixmapGradient(iTitleGradient, inactiveTitleColor, shade(inactiveTitleColor, 0.8),
				   VerticalGradient);

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
	pinUpPix = QPixmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	pinUpPix.fill(Qt::transparent);
	p.begin( &pinUpPix );
	colorBitmapLayer(pinup_white_bits, window()->color(QPalette::Active, QPalette::Light));
	colorBitmapLayer(pinup_gray_bits, window()->color(QPalette::Active, QPalette::Button));
	colorBitmapLayer(pinup_dgray_bits, window()->color(QPalette::Active, QPalette::Dark));
	p.end();
	pinUpPix.setMask(QBitmap::fromData(
						  QSize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE),
						  pinup_mask_bits, QImage::Format_MonoLSB));

	pinDownPix = QPixmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	pinDownPix.fill(Qt::transparent);
	p.begin( &pinDownPix );
	colorBitmapLayer(pindown_white_bits, window()->color(QPalette::Active, QPalette::Light));
	colorBitmapLayer(pindown_gray_bits, window()->color(QPalette::Active, QPalette::Button));
	colorBitmapLayer(pindown_dgray_bits, window()->color(QPalette::Active, QPalette::Dark));
	p.end();
	pinDownPix.setMask(QBitmap::fromData(
						  QSize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE),
						  pindown_mask_bits, QImage::Format_MonoLSB));

	// Inactive pins
	ipinUpPix = QPixmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	ipinUpPix.fill(Qt::transparent);
	p.begin( &ipinUpPix );
	colorBitmapLayer(pinup_white_bits, window()->color(QPalette::Inactive, QPalette::Light));
	colorBitmapLayer(pinup_gray_bits, window()->color(QPalette::Inactive, QPalette::Button));
	colorBitmapLayer(pinup_dgray_bits, window()->color(QPalette::Inactive, QPalette::Dark));
	p.end();
	ipinUpPix.setMask(QBitmap::fromData(
						   QSize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE),
						   pinup_mask_bits, QImage::Format_MonoLSB));

	ipinDownPix = QPixmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	ipinDownPix.fill(Qt::transparent);
	p.begin( &ipinDownPix );
	colorBitmapLayer(pindown_white_bits, window()->color(QPalette::Inactive, QPalette::Light));
	colorBitmapLayer(pindown_gray_bits, window()->color(QPalette::Inactive, QPalette::Button));
	colorBitmapLayer(pindown_dgray_bits, window()->color(QPalette::Inactive, QPalette::Dark));
	p.end();
	ipinDownPix.setMask(QBitmap::fromData(
							 QSize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE),
							 pindown_mask_bits, QImage::Format_MonoLSB));

	// Cache all possible button states
	btnUpPix = QPixmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	btnUpPix.fill(Qt::transparent);
	btnDownPix = QPixmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	btnDownPix.fill(Qt::transparent);
	ibtnUpPix = QPixmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	ibtnUpPix.fill(Qt::transparent);
	ibtnDownPix = QPixmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	ibtnDownPix.fill(Qt::transparent);

	auto drawButtonBackground = [&](QPixmap &pixmap, bool sunken, bool active) {
		
		QColor c = window()->color(QPalette::Active, QPalette::Button);
		if (active) {
			pixmapGradient(pixmap, c, Qt::white,
						   DiagonalGradient);
		} else {
			pixmapGradient(pixmap, inactiveTitleColor, shade(inactiveTitleColor, 0.8),
						   VerticalGradient);
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

	bottomLeftPix 	= QPixmap();
	bottomLeftPix.fill(Qt::transparent);
	bottomRightPix	= QPixmap();
	bottomRightPix.fill(Qt::transparent);
	abottomLeftPix = QPixmap();
	abottomLeftPix.fill(Qt::transparent);
	abottomRightPix	= QPixmap();
	abottomRightPix.fill(Qt::transparent);
	bottomLeftPix.convertFromImage(bottomleft);
	bottomRightPix.convertFromImage(bottomright);
	abottomLeftPix.convertFromImage(abottomleft);
	abottomRightPix.convertFromImage(abottomright);
}

void
BluecurveDecoration::updateTitleBar()
{
	int x, width;
	if (m_leftButtons && m_rightButtons) {
		x = m_leftButtons->geometry().right();
		width = m_rightButtons->geometry().left() - x;
	} else {
		x = 0;
		width = window()->width();
	}
	
	setTitleBar(QRect(x,TOP_GRABBAR_WIDTH,width,TITLE_HEIGHT));
}

void
BluecurveDecoration::updateButtonsGeometryDelayed()
{
	QTimer::singleShot(0, this, &BluecurveDecoration::updateButtonsGeometry);
}

void
BluecurveDecoration::updateButtonsGeometry()
{
	m_leftButtons->setSpacing(2);
	m_rightButtons->setSpacing(2);

	m_leftButtons->setPos(QPointF(1, TOP_GRABBAR_WIDTH));
	m_rightButtons->setPos(QPointF(size().width() - m_rightButtons->geometry().width() - 1, TOP_GRABBAR_WIDTH));

	updateTitleBar();
	update();
}

void
BluecurveDecoration::paint(QPainter *p, const QRectF &repaintRegion)
{
	
	bool drawLeftDivider = true; 
	bool drawRightDivider = true;

	// Obtain widget bounds for buffer painting
	int x2 = rect().width() - 1;
	int y2 = rect().height() - 1;
	int w  = rect().width();
	int h  = rect().height();
	
	// Titlebar rectangle
	QRectF r(titleBar());

	// Buffer for the decoration (which we apply the mask later)
	QPixmap decoBuffer = QPixmap(w, h);
	decoBuffer.fill(Qt::transparent);

	// Create a disposable pixmap buffer for the titlebar
	// very early before drawing begins so there is no lag
	// during painting pixels.
	QPixmap titleBuffer = QPixmap(w, TITLE_HEIGHT + TOP_GRABBAR_WIDTH);
	titleBuffer.fill(Qt::transparent);

	// Draw title gradient
	QPainter p2(&titleBuffer);
	p2.drawTiledPixmap(0, TOP_GRABBAR_WIDTH, w, TITLE_HEIGHT+TOP_GRABBAR_WIDTH,
					   window()->isActive() ? aTitleGradient : iTitleGradient);

	QFont fnt = settings()->font();
	p2.setFont( fnt );

	// Draw the titlebar stipple if active and available
	if (window()->isActive() && !titlePix.isNull()) {
		QFontMetrics fm(fnt);
		int captionWidth = fm.horizontalAdvance(window()->caption()) + 1;
		p2.drawTiledPixmap( r.x() + 2 + 2 + captionWidth, TOP_GRABBAR_WIDTH,
							r.width() - 2 - 4 - captionWidth, 
							TITLE_HEIGHT+1, titlePix );
	}

	if (window()->isActive()) {
		p2.setPen(shade(window()->color(KDecoration3::ColorGroup::Active,
										KDecoration3::ColorRole::TitleBar),1.4).darker());
		p2.drawText(r.x() + 2 + 1, TOP_GRABBAR_WIDTH + 1,
					r.width() - 2 - 1, r.height(),
					Qt::AlignLeft | Qt::AlignVCenter, window()->caption() );
	}

    p2.setPen(window()->color(window()->isActive()
							  ? KDecoration3::ColorGroup::Active
							  : KDecoration3::ColorGroup::Inactive,
							  KDecoration3::ColorRole::Foreground));
	p2.drawText(r.x() + 2, TOP_GRABBAR_WIDTH,
				r.width() - 2, r.height(),
				Qt::AlignLeft | Qt::AlignVCenter, window()->caption() );

	// Main Title Bar background area
	p2.setPen(Qt::white);
	p2.drawLine(1, 1, x2 - 1, 1);
	// This is kind of broken...
	// We fill in the inner part of the circle here.  This is dependent on BUTTON_DIAM
	p2.drawLine(1, 1, 1, TOP_GRABBAR_WIDTH + TITLE_HEIGHT);
	p2.drawLine(2, 2, 3, 2);
	p2.drawLine(2, 2, 2, 3);
	p2.drawLine(w - 2 , 1, w - 2, TOP_GRABBAR_WIDTH + TITLE_HEIGHT);
	p2.drawLine(w - 3, 2, w - 3, 5);
	p2.drawLine(w - 4, 2, w - 3, 2);

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

	if (window()->isActive()) {
		const auto buttonList = m_leftButtons->buttons() + m_rightButtons->buttons();
		for (KDecoration3::DecorationButton *button : buttonList) {
			if (!button)
				continue;
			
			bool isLeftButtonRight = (!m_leftButtons->buttons().isEmpty() &&
								 button == m_leftButtons->buttons().last());
			bool isButtonRight = (!m_rightButtons->buttons().isEmpty() &&
								  button == m_rightButtons->buttons().last());

			if (!button->isVisible()) {
				if (isButtonRight)
					drawRightDivider = false;
				// FIXME: Should be LeftButtonLeft if we had it
				if (isLeftButtonRight)
					drawLeftDivider = false;
				continue;
			}
			
			QRectF buttonSize = button->geometry();
			p2.setPen(Qt::white);
			p2.drawLine (buttonSize.x() - 1, TOP_GRABBAR_WIDTH,
						 buttonSize.x() - 1, TOP_GRABBAR_WIDTH + TITLE_HEIGHT);
			if (isButtonRight)
				continue;
			else if (isLeftButtonRight)
				p2.setPen(window()->palette().mid().color().lighter(120));
			else
				p2.setPen(window()->palette().dark().color());
			p2.drawLine (buttonSize.x() + buttonSize.width(), TOP_GRABBAR_WIDTH - 1,
						 buttonSize.x() + buttonSize.width(), TOP_GRABBAR_WIDTH + TITLE_HEIGHT);
		}
	}

	// Top Left Button Area
	if (drawLeftDivider)
	{
		if (window()->isActive())
			p2.setPen(window()->color(KDecoration3::ColorGroup::Active,
									  KDecoration3::ColorRole::TitleBar).darker(150));
		else
			p2.setPen(window()->palette().mid().color());
		p2.drawLine (r.x() , 1, r.x() , TITLE_HEIGHT + TOP_GRABBAR_WIDTH);
	}

	// Top Right Button Area
	if (drawRightDivider)
	{
		if (window()->isActive())
			p2.setPen(window()->color(KDecoration3::ColorGroup::Active,
									  KDecoration3::ColorRole::TitleBar).darker(150));
		else
		    p2.setPen(window()->palette().mid().color());
		p2.drawLine (r.x() + r.width() - 2, 1,
					 r.x() + r.width() - 2 , TITLE_HEIGHT + TOP_GRABBAR_WIDTH);
	}

	// Black outer line
	p2.setPen(Qt::black);
	p2.drawRect(0,0,w-1,h-1);
	p2.drawArc(0, 0, BUTTON_DIAM, BUTTON_DIAM, 90*16, 90*16);
	p2.drawArc(w - BUTTON_DIAM , 0, BUTTON_DIAM, BUTTON_DIAM, 0*16, 90*16);
	p2.end();

	QPainter p1(&decoBuffer); // Painter for the main decoration buffer
	
	int sideStart = TITLE_HEIGHT + TOP_GRABBAR_WIDTH + 1;
					
	// Draw the left and right sides

	// Fill the left side first
	qDrawShadePanel(&p1,
					// We compensate for the top and bottom parts of the bevel
					// by drawing 1 pixel below and above the frame part
					1, sideStart - 1,
					BORDER_WIDTH - 1, h - (sideStart + 2),
					window()->palette(), false, 1, &window()->palette().window());

	
	// Right Side
	qDrawShadePanel(&p1,
					x2 - (BORDER_WIDTH - 2), sideStart - 1,
					BORDER_WIDTH - 2, h - (sideStart + 2),
					window()->palette(), false, 1, &window()->palette().window());

	
	p1.setPen(window()->palette().dark().color());
	p1.drawLine(x2 - (BORDER_WIDTH - 1), sideStart, x2 - (BORDER_WIDTH - 1), h - sideStart);

	// Draw the bottom
	qDrawShadePanel(&p1,
					0, y2 - (BORDER_WIDTH - 2),
					w, (BORDER_WIDTH - 2),
				    window()->palette(), false, 1, &window()->palette().window());
    p1.setPen(window()->palette().dark().color());
	p1.drawLine(0, y2 - (BORDER_WIDTH - 1), x2, y2 - (BORDER_WIDTH - 1));

	// Line above the app and below the title bar
    p1.setPen(window()->palette().dark().color());
	p1.drawLine(0, TITLE_HEIGHT + TOP_GRABBAR_WIDTH,
				x2, TITLE_HEIGHT + TOP_GRABBAR_WIDTH);

	// Draw the title buffer
	p1.drawPixmap(0, 0, titleBuffer);

	// Draw an outer black frame
	p1.setPen(Qt::black);
	p1.drawRect(0,0,w-1,h-1);

	// Put on the bottom corners
	p1.drawPixmap(0, h - bottomLeftPix.height(),
				  window()->isActive() ? abottomLeftPix : bottomLeftPix);
	p1.drawPixmap(w - bottomRightPix.width(), h - bottomRightPix.height(), 
				  window()->isActive() ? abottomRightPix : bottomRightPix);

	p1.end();
	
	// Apply the mask to the decoration buffer and draw it
	decoBuffer.setMask(decorationMask());
	p->drawPixmap(rect().x(),rect().y(), decoBuffer);

	m_leftButtons->paint(p, repaintRegion);
	m_rightButtons->paint(p, repaintRegion);
	
}

QBitmap
BluecurveDecoration::decorationMask()
{
	// Obtain widget bounds
	int w  = rect().width();
	int h  = rect().height();

	int rad = BUTTON_DIAM / 2;
	int dm = BUTTON_DIAM;

	QBitmap mask(w, h);
	mask.clear();

	QPainter p(&mask);

	p.fillRect(0, 0, w, h, Qt::color1);

	p.eraseRect(0, 0, rad, rad);
	p.eraseRect(w-rad+1, 0, rad, rad);

	p.eraseRect(0, h-BOTTOM_CORNER, BOTTOM_CORNER, BOTTOM_CORNER);
	p.eraseRect(w-BOTTOM_CORNER, h-BOTTOM_CORNER, BOTTOM_CORNER, BOTTOM_CORNER);

	p.setPen(Qt::color1);
	p.setBrush(Qt::color1);

	p.drawPie(0, 0, dm, dm, 90*16, 90*16);
	p.drawArc(0, 0, dm, dm, 90*16, 90*16);

	p.drawPie(w-dm, 0, dm, dm, 0*16, 90*16);
	p.drawArc(w-dm, 0, dm, dm, 0*16, 90*16);

	p.drawPixmap(0, h - bottomLeftPix.height(), bottomLeftPix.mask());

	p.drawPixmap(w-bottomRightPix.width(), h - bottomRightPix.height(), 
				 bottomRightPix.mask());

	p.fillRect(BOTTOM_CORNER, h - bottomLeftPix.height(),
			   bottomLeftPix.width()-BOTTOM_CORNER,
			   bottomLeftPix.height()-BOTTOM_CORNER,
			   Qt::color1);

	p.fillRect(w-bottomRightPix.width(), h - bottomRightPix.height(), 
			   bottomRightPix.width()-BOTTOM_CORNER,
			   bottomRightPix.height()-BOTTOM_CORNER,
			   Qt::color1);

	p.end();
	return mask;
}

BluecurveButton::BluecurveButton(KDecoration3::DecorationButtonType type,
								 KDecoration3::Decoration *decoration,
								 QObject *parent)
	: KDecoration3::DecorationButton(type, decoration, parent)
{

	setGeometry(QRectF(0,0,BASE_BUTTON_SIZE,BASE_BUTTON_SIZE));
	
	// Set decoration bitmap to be drawn
	// Note: if button is maximize, we need to remember to change bits
	// based on window minimised/maximised state.
	switch (type) {
	case KDecoration3::DecorationButtonType::Menu:
		iconBits = QBitmap::fromData(QSize(14,14), menu_bits);
		break;
	case KDecoration3::DecorationButtonType::Minimize:
		iconBits = QBitmap::fromData(QSize(14,14), iconify_bits);
		break;
	case KDecoration3::DecorationButtonType::Maximize:
		iconBits = QBitmap::fromData(QSize(14,14), maximize_bits);
		break;
	case KDecoration3::DecorationButtonType::Close:
		iconBits = QBitmap::fromData(QSize(14,14), close_bits);
		break;
	case KDecoration3::DecorationButtonType::ContextHelp:
		iconBits = QBitmap::fromData(QSize(14,14), question_bits);
		break;
	default:
		break;
	}
}

BluecurveButton::~BluecurveButton() = default;

BluecurveButton
*BluecurveButton::create(KDecoration3::DecorationButtonType type,
						 KDecoration3::Decoration *decoration,
						 QObject *parent)
{
	if (auto d = qobject_cast<KDecoration3::Decoration *>(decoration)) {
		BluecurveButton *b = new BluecurveButton(type, d, parent);
		return b;
	} else
		return nullptr;	
}

void
BluecurveButton::paint(QPainter *p, const QRectF &repaintRegion)
{	
	// Obtain button bounds
	int x = geometry().x();
	int y = geometry().y();
	int w  = geometry().width();
	int h  = geometry().height();

	// Buffer for the decoration (which we apply the mask later)
	QPixmap buttonBuffer = QPixmap(w, h);
	buttonBuffer.fill(Qt::transparent);
	QPainter p1(&buttonBuffer);

	if (!iconBits.isNull()) {
		// Button background
		QPixmap btnbg;

		if (isPressed() || isChecked())
			btnbg = decoration()->window()->isActive() ? btnDownPix : ibtnDownPix;
		else
			btnbg = decoration()->window()->isActive() ? btnUpPix : ibtnUpPix;

		if (isHovered())
			pixmapIntensity(btnbg, 0.8);
		p1.drawPixmap(0,0,btnbg);

		// Button icon
		bool darkDeco = qGray(decoration()->window()->color(
								  decoration()->window()->isActive() ?
								  QPalette::ColorGroup::Active :
								  QPalette::ColorGroup::Inactive,
								  QPalette::ColorRole::Button).rgb()) > 127;

		QColor bgc = decoration()->window()->color(
			decoration()->window()->isActive() ?
			KDecoration3::ColorGroup::Active :
			KDecoration3::ColorGroup::Inactive,
			KDecoration3::ColorRole::TitleBar);

		/*if (isHovered())
			p->setPen( darkDeco ? bgc.darker(120) : bgc.lighter(120) );
		else
		p->setPen( darkDeco ? bgc.darker(150) : bgc.lighter(150) );*/

		int xOff = (w-14)/2;
		int yOff = (h-14)/2;

		QPixmap icon(iconBits.size());
		if (isHovered())
			icon.fill( darkDeco ? bgc.darker(120) : bgc.lighter(120) );
		else
			icon.fill( darkDeco ? bgc.darker(150) : bgc.lighter(150) );
		icon.setMask(iconBits);
		
		p1.drawPixmap((isPressed() || isChecked()) ? xOff+1: xOff,
					  (isPressed() || isChecked()) ? yOff+1 : yOff,
					  icon);
	} else {
		
	}
	
	p1.end();
	buttonBuffer.setMask(buttonMask());
	p->drawPixmap(x,y,buttonBuffer);
}

QBitmap
BluecurveButton::buttonMask()
{
	// Obtain button bounds
	int w  = geometry().width();
	int h  = geometry().height();
	int r = BUTTON_DIAM / 2;
	int dm = BUTTON_DIAM;

	QBitmap mask(w, h);
	mask.clear();

	QPainter p(&mask);
	p.fillRect(0, 0, w, h, Qt::color1);

	p.setPen(Qt::color1);
	p.setBrush(Qt::color1);

	if (geometry().x()==1) {
		p.eraseRect(0, -TOP_GRABBAR_WIDTH, r, r);
		p.drawPie(0, -TOP_GRABBAR_WIDTH, dm-1, dm-1, 90*16, 90*16);
		p.drawArc(0, -TOP_GRABBAR_WIDTH, dm-1, dm-1, 90*16, 90*16);
	} else if (geometry().x()==decoration()->size().width() - w - 1) {
		p.eraseRect(w-r , -TOP_GRABBAR_WIDTH, r,r);
		p.drawPie(w-dm, -TOP_GRABBAR_WIDTH, dm-1, dm-1, 0*16, 90*16);
		p.drawArc(w-dm, -TOP_GRABBAR_WIDTH, dm-1, dm-1, 0*16, 90*16);
	}

	p.end();
	return mask;
}

#include "bluecurvedecoration.moc"
