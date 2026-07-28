/*
  Bluecurve KDecoration 3 theme.

  Copyright (c) 2026 neeeeow
  Author: neeeeow (https://github.com/neeeeow/Bluecurve-Qt)

  Painting logic based on the Bluecurve KDE 3 theme:
  Copyright (c) 1999, 2001 Red Hat, Inc.
  Authors: Daniel Duley <mosfet@kde.org>,
           Matthias Ettrich <ettrich@kde.org>
		   Karol Szwed <gallium@kde.org>
		   Than Ngo <than@redhat.com>
  
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/ 

#include "bluecurvedecoration.h"
#include "bitmaps.h"

#include <KDecoration3/Decoration>
#include <KDecoration3/DecorationSettings>
#include <KPluginFactory>

#include <QPainter>
#include <QBitmap>
#include <QTimer>
#include <qdrawutil.h>

#define BORDER_WIDTH      6
#define CORNER_HEIGHT     22

#define BUTTON_DIAM       12
#define TOP_GRABBAR_WIDTH 2
#define BOTTOM_CORNER     5

#define INTENSITY(r, g, b) ((r) * 0.30 + (g) * 0.59 + (b) * 0.11)

/* these pixmaps are needed across every button, so we just store them globally for convenience */
QPixmap pinDownPix;
QPixmap pinUpPix;
QPixmap ipinDownPix;
QPixmap ipinUpPix;

QPixmap btnPix;
QPixmap ibtnPix;

K_PLUGIN_FACTORY_WITH_JSON(
	BluecurveDecorationFactory,
	"metadata.json",
	registerPlugin<BluecurveDecoration>();
	)

static void
drawGradient(QPixmap &pixmap, const QColor &ca, const QColor &cb,
			 qreal x1=0, qreal y1=0, qreal x2=0, qreal y2=1, qreal opacity=1)
{
	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	
	QLinearGradient gradient(x1, y1, x2, y2);
	gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
	gradient.setColorAt(0, ca);
	gradient.setColorAt(1, cb);
	
	painter.setOpacity(opacity);
	painter.fillRect(pixmap.rect(), gradient);
	painter.end();
}

static void
expAlphaGradient(QPixmap &pixmap, const QColor &c,
				 qreal x1=0, qreal y1=0, qreal x2=0, qreal y2=1, qreal opacity=1)
{
	/* Draws an exponentially decaying alpha gradient to a pixmap */

	int r, g, b, alpha0;
	c.getRgb(&r, &g, &b, &alpha0);

	const qreal k = 2; // decay constant
	const int stops = 20; // number of stops in the gradient

	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing);

	QLinearGradient gradient(x1, y1, x2, y2);
	gradient.setCoordinateMode(QGradient::ObjectBoundingMode);

	for (int i=0; i<=stops; i++) {
		qreal stop = (qreal)i / stops;
		int alpha = qRound(alpha0 * std::pow(1.0 - stop, k));
		gradient.setColorAt(stop, QColor(r,g,b,alpha));
	}

	painter.setOpacity(opacity);
	painter.fillRect(pixmap.rect(), gradient);
	painter.end();
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

static QColor
blend (const QColor &bg, const QColor &fg, qreal alpha)
{
	alpha = qBound(0.0, alpha, 1.0);

    int r = qRound((fg.red()   * alpha) + (bg.red()   * (1.0 - alpha)));
    int g = qRound((fg.green() * alpha) + (bg.green() * (1.0 - alpha)));
    int b = qRound((fg.blue()  * alpha) + (bg.blue()  * (1.0 - alpha)));

    return QColor(r, g, b);	
}

static void
colorize(QImage &img, const QColor &color)
{
	// New colors
	qreal nr = color.redF();
    qreal ng = color.greenF();
    qreal nb = color.blueF();

	// Color table used by the image
	QVector<QRgb> colorTable = img.colorTable();
	int pixels = colorTable.size();

	for (int i = 0; i < pixels; ++i) {
		// Extract pixel r, g, b components
		QColor c(colorTable[i]);
		qreal r = c.redF();
		qreal g = c.greenF();
		qreal b = c.blueF();
		
		qreal intensity = INTENSITY(r, g, b);

		// Go from black at intensity = 0 to original color
		// at intensity = 0.5 to white at intensity = 1
		if (intensity <= 0.5) {
			r = nr * intensity * 2.0;
			g = ng * intensity * 2.0;
			b = nb * intensity * 2.0;
		} else {
			r = nr + (1.0 - nr) * (intensity - 0.5) * 2.0;
			g = ng + (1.0 - ng) * (intensity - 0.5) * 2.0;
			b = nb + (1.0 - nb) * (intensity - 0.5) * 2.0;
		}

		// Update the color of the pixel
		c.setRgbF(r, g, b);
		colorTable[i] = (c.rgb() & RGB_MASK) | (colorTable[i] & ~RGB_MASK);
	}
	img.setColorTable(colorTable);	
}

static void
colorBitmaps(QPainter *p, const QPalette &palette, int x, int y, int w,
			 int h, bool isXBitmaps, const uchar *lightColor,
			 const uchar *midColor, const uchar *midlightColor,
			 const uchar *darkColor, const uchar *blackColor,
			 const uchar *whiteColor)
{

	const uchar *data[]={lightColor, midColor, midlightColor, darkColor,
		blackColor, whiteColor};
	
	QColor colors[]={palette.light().color(), palette.mid().color(), palette.midlight().color(),
		palette.dark().color(), Qt::black, Qt::white};

	int i;
	QBitmap b;
	for(i=0; i < 6; ++i){
		if(data[i]){
			b = QBitmap::fromData(QSize(w,h), data[i],
								  isXBitmaps ? QImage::Format_MonoLSB
								  : QImage::Format_Mono);
			b.setMask(b);
			p->setPen(colors[i]);
			p->drawPixmap(x, y, b);
		}
	}
}

BluecurveDecoration::BluecurveDecoration(QObject *parent, const QVariantList &args) : KDecoration3::Decoration(parent, args)
{

}

BluecurveDecoration::~BluecurveDecoration() = default;

bool
BluecurveDecoration::init()
{
	updateBorders();	
	createPixmaps();

	// Create buttons
	m_leftButtons = new KDecoration3::DecorationButtonGroup(KDecoration3::DecorationButtonGroup::Position::Left,
															this, &BluecurveButton::create);
    m_rightButtons = new KDecoration3::DecorationButtonGroup(KDecoration3::DecorationButtonGroup::Position::Right,
															 this, &BluecurveButton::create);
	m_leftButtons->setSpacing(1);
	m_rightButtons->setSpacing(1);
	
	auto s = settings();

	/* Settings changes */
	// buttons
    connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsLeftChanged, this, &BluecurveDecoration::updateButtonsGeometryDelayed);
	connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsRightChanged, this, &BluecurveDecoration::updateButtonsGeometryDelayed);

	// full reconfiguration
	connect(s.get(), &KDecoration3::DecorationSettings::reconfigured, this, &BluecurveDecoration::reconfigure);

	/* Window state changes */
	connect(window(), &KDecoration3::DecoratedWindow::paletteChanged, this, &BluecurveDecoration::createPixmaps);
	
	// Update() signals
	connect(window(), &KDecoration3::DecoratedWindow::activeChanged, this, [this]() { update(); });

	// titleBar signals
	connect(window(), &KDecoration3::DecoratedWindow::captionChanged, this, [this]() {
		// update the caption area
		update(titleBar());
    });

	// Add / remove borders when maximized state is changed
	connect(window(), &KDecoration3::DecoratedWindow::maximizedChanged, this, &BluecurveDecoration::updateBorders);
	
	// Button signals. as a reminder: update() and updateTitleBar() is called in updateButtonsGeometry
	connect(window(), &KDecoration3::DecoratedWindow::sizeChanged, this, &BluecurveDecoration::updateButtonsGeometry);
	connect(window(), &KDecoration3::DecoratedWindow::widthChanged, this, &BluecurveDecoration::updateButtonsGeometry);
    connect(window(), &KDecoration3::DecoratedWindow::maximizedChanged, this, &BluecurveDecoration::updateButtonsGeometry);
    connect(window(), &KDecoration3::DecoratedWindow::adjacentScreenEdgesChanged, this, &BluecurveDecoration::updateButtonsGeometry);
    connect(window(), &KDecoration3::DecoratedWindow::shadedChanged, this, &BluecurveDecoration::updateButtonsGeometry);
	
	updateButtonsGeometry();

	update();
	return true;
}

void
BluecurveDecoration::reconfigure()
{
	/* This is called whenever the windows are reconfigured */

	updateBorders();
	createPixmaps();	
	updateButtonsGeometryDelayed();
}

void
BluecurveDecoration::updateBorders()
{
	QFontMetricsF metrics(settings()->font());	
	m_titleHeight = m_buttonSize = std::max(14, qRound(metrics.height()) + 3); // Height to match Metacity theme
	int borderWidth = window()->isMaximized() ? 0 : BORDER_WIDTH;	
	setBorders(QMargins(borderWidth, m_titleHeight + 3, borderWidth, borderWidth));
}

void
BluecurveDecoration::createPixmaps()
{
	QPainter p;
	QPalette palette = window()->palette();
	
	// Titlebar stipple
	QPainter stipplePainter;
	int x, y;
	titlePix = QPixmap(125, m_titleHeight-3);
	titlePix.fill(Qt::transparent);
	stipplePainter.begin(&titlePix);

	for(y = 0; y < (m_titleHeight-3); y++) {
		for(x = (3 - y) % 5; x < 125; x += 5) {			
			stipplePainter.setPen(QColor(2,2,2,116));
			stipplePainter.drawPoint(x,y);
			stipplePainter.setPen(QColor(242,242,242,94));
			stipplePainter.drawPoint(x+1,y);
		}
	}

	stipplePainter.end();

	// Titlebar colors
	QColor activeTitleColor(window()->color(KDecoration3::ColorGroup::Active,
											KDecoration3::ColorRole::TitleBar));
	QColor inactiveTitleColor(window()->color(KDecoration3::ColorGroup::Inactive,
											  KDecoration3::ColorRole::TitleBar));

	// Titlebar gradient images
	iTitleGradient = QPixmap(8, m_titleHeight + 1);
	drawGradient(iTitleGradient, inactiveTitleColor, shade(inactiveTitleColor, 0.8));

	// Title blocker bottom
	titleBlockerBottom = QPixmap(8, m_titleHeight + 1);
	titleBlockerBottom.fill(Qt::transparent);
	expAlphaGradient(titleBlockerBottom, activeTitleColor,
					 0, 1, 0, 0);

	titleGradientBottom = QPixmap(8, m_titleHeight + 1);
	titleGradientBottom.fill(Qt::transparent);

	QColor titleGradientColor = shade(activeTitleColor,2);
	titleGradientColor.setAlpha(225);
	expAlphaGradient(titleGradientBottom, titleGradientColor,
					 0, 1, 0, 0, 0.8);

	// Active pins
	pinUpPix = QPixmap(14, 14);
	pinUpPix.fill(Qt::transparent);
	p.begin( &pinUpPix );
	colorBitmaps( &p, palette, 0, 0, 14, 14, true, pinup_white_bits,
	  pinup_gray_bits, NULL, NULL, pinup_dgray_bits, NULL );
	p.end();
	pinUpPix.setMask(QBitmap::fromData(
						 QSize(14, 14),
						 pinup_mask_bits, QImage::Format_MonoLSB));

	pinDownPix = QPixmap(14, 14);
	pinDownPix.fill(Qt::transparent);
	p.begin( &pinDownPix );
	colorBitmaps( &p, palette, 0, 0, 14, 14, true, pindown_white_bits,
				  pindown_gray_bits, NULL, NULL, pindown_dgray_bits, NULL );
	p.end();
	pinDownPix.setMask(QBitmap::fromData(
						  QSize(14, 14),
						  pindown_mask_bits, QImage::Format_MonoLSB));

	// Inactive pins
	ipinUpPix = QPixmap(14, 14);
	ipinUpPix.fill(Qt::transparent);
	p.begin( &ipinUpPix );
	colorBitmaps( &p, palette, 0, 0, 14, 14, true, pinup_white_bits,
				  pinup_gray_bits, NULL, NULL, pinup_dgray_bits, NULL );
	p.end();
	ipinUpPix.setMask(QBitmap::fromData(
						   QSize(14, 14),
						   pinup_mask_bits, QImage::Format_MonoLSB));

	ipinDownPix = QPixmap(14, 14);
	ipinDownPix.fill(Qt::transparent);
	p.begin( &ipinDownPix );
	colorBitmaps( &p, palette, 0, 0, 14, 14, true, pindown_white_bits,
				  pindown_gray_bits, NULL, NULL, pindown_dgray_bits, NULL );
	p.end();
	ipinDownPix.setMask(QBitmap::fromData(
							 QSize(14, 14),
							 pindown_mask_bits, QImage::Format_MonoLSB));

	// Cache all possible button states
	btnPix = QPixmap(m_buttonSize + 3, m_buttonSize);
	btnPix.fill(Qt::transparent);
	ibtnPix = QPixmap(m_buttonSize + 3, m_buttonSize);
	ibtnPix.fill(Qt::transparent);

	auto drawButtonBackground = [&](QPixmap &pixmap, bool active) {
		
		if (active) {
			QColor c = window()->color(QPalette::Active, QPalette::Button);

			QPixmap topEdge = QPixmap(pixmap.width(), 1);
			drawGradient(topEdge, shade(c, 0.7), shade(c, 1.3),
						 0, 0, 1, 0);
			
			QPixmap sideEdge = QPixmap(1, pixmap.height()+1);
			drawGradient(sideEdge, shade(c, 0.7), shade(c, 1.3));

			QPixmap bg1 = QPixmap(pixmap.width(), pixmap.height()+1);
			drawGradient(bg1, shade(c, 1.3), shade(c, 0.9),
						 0, 0, 1, 1);

			QPixmap bg2 = QPixmap(pixmap.width() - 1, pixmap.height());
			drawGradient(bg2, shade(c, 0.85), shade(c, 1.3),
						 0, 0, 1, 1);

			QPainter bgPainter(&pixmap);
			bgPainter.drawPixmap(0, 0, bg1);
			bgPainter.drawPixmap(1, 0, bg2);
			bgPainter.drawPixmap(1, 0, sideEdge);
			bgPainter.drawPixmap(1, 0, topEdge);
			bgPainter.end();
			
		} else {
			drawGradient(pixmap, inactiveTitleColor, shade(inactiveTitleColor, 0.8));
		}
	};

	drawButtonBackground( btnPix, true );
	drawButtonBackground( ibtnPix, false );

	QImage bottomleft(bottom_left_xpm);
	QImage bottomright(bottom_right_xpm);
	QImage abottomleft(bottom_left_xpm);
	QImage abottomright(bottom_right_xpm);

	colorize(bottomleft, shade(inactiveTitleColor, 0.8));
	colorize(bottomright, shade(inactiveTitleColor, 0.8));
    colorize(abottomleft, activeTitleColor);
	colorize(abottomright, activeTitleColor);

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
		x = m_leftButtons->geometry().right() + 1;
		width = m_rightButtons->geometry().left() - x;
	} else {
		x = 0;
		width = window()->width();
	}
	
	setTitleBar(QRect(x,TOP_GRABBAR_WIDTH,width,m_titleHeight));
}

void
BluecurveDecoration::updateButtonsGeometryDelayed()
{
	QTimer::singleShot(0, this, &BluecurveDecoration::updateButtonsGeometry);
}

void
BluecurveDecoration::updateButtonsGeometry()
{
	const auto buttons = m_leftButtons->buttons() + m_rightButtons->buttons();
	for (auto *button : buttons) {
		button->setGeometry(QRectF(0, 0, m_buttonSize + 3, m_buttonSize));
	}

	m_leftButtons->setSpacing(1);
	m_rightButtons->setSpacing(1);

	if (window()->isMaximized()) {
		m_leftButtons->setPos(QPointF(0, TOP_GRABBAR_WIDTH));
		m_rightButtons->setPos(QPointF(size().width() - m_rightButtons->geometry().width() - 1, TOP_GRABBAR_WIDTH));
	} else {
		m_leftButtons->setPos(QPointF(1, TOP_GRABBAR_WIDTH));
		m_rightButtons->setPos(QPointF(size().width() - m_rightButtons->geometry().width() - 2, TOP_GRABBAR_WIDTH));
	}
	
	updateTitleBar();
	update();
}

void
BluecurveDecoration::paint(QPainter *p, const QRectF &repaintRegion)
{
	// Disable antialiasing
	p->setRenderHint(QPainter::Antialiasing, false);
	
	// Store colours that we'll need
    QColor activeTitleColor(window()->color(KDecoration3::ColorGroup::Active,
											KDecoration3::ColorRole::TitleBar));
	QColor inactiveTitleColor(window()->color(KDecoration3::ColorGroup::Inactive,
											  KDecoration3::ColorRole::TitleBar));
	QColor activeWindowColor(window()->color(QPalette::Active, QPalette::Window));
	QColor activeLightColor = shade(activeWindowColor, 1.3);
	QColor activeDarkColor = shade(activeWindowColor, 0.7);
	QColor activeButtonColor(window()->color(QPalette::Active, QPalette::Button));
	QColor activeButtonDark = shade(activeButtonColor, 0.7);
	
	bool drawLeftDivider = true; 
	bool drawRightDivider = true;

	// Obtain widget bounds for buffer painting
	int w  = rect().width();
	int h  = rect().height();
	
	// Rectangle over which we paint the titlebar decoration
	QRectF r(titleBar().adjusted(1,-1,-1,0));

	// Buffer for the decoration (allows us to avoid weird coordinate issues)
	QPixmap decoBuffer = QPixmap(w, h);
	decoBuffer.fill(Qt::transparent);

	// Create a disposable pixmap buffer for the titlebar
	// very early before drawing begins so there is no lag
	// during painting pixels.
	QPixmap titleBuffer = QPixmap(w, m_titleHeight + TOP_GRABBAR_WIDTH + 1);
	titleBuffer.fill(Qt::red);

	QPainter p2(&titleBuffer);

	// Draw base titlebar gradient
	if (window()->isActive())
		p2.fillRect(r, activeTitleColor);
	else		
		p2.drawTiledPixmap(0, TOP_GRABBAR_WIDTH, w, m_titleHeight+TOP_GRABBAR_WIDTH,
						   iTitleGradient);

	// Draw active titlebar graphics
	if (window()->isActive()) {
		p2.setPen(shade(activeButtonColor, 1.2));
		p2.drawLine(0, 1, w - 1, 1);
		
		p2.fillRect(r, activeTitleColor);
		if (!titlePix.isNull())
			p2.drawTiledPixmap(r.x()+1, r.y()+2, r.width()-2, r.height()-4, titlePix);
		if (!titleBlockerBottom.isNull())
			p2.drawTiledPixmap(r, titleBlockerBottom);

		QPixmap titleBlockerRight = QPixmap(r.width(), r.height());
		titleBlockerRight.fill(Qt::transparent);
		expAlphaGradient(titleBlockerRight, activeTitleColor,
						 0.1, 0, 1, 0);
		p2.drawPixmap(r, titleBlockerRight, titleBlockerRight.rect());

		QPixmap shine = QPixmap(r.width(), 1);
		shine.fill(Qt::transparent);
		drawGradient(shine, shade(activeTitleColor, 2), shade(activeTitleColor, 1.7),
					 0, 0, 1, 0, 0.2);		
		drawGradient(shine, shade(activeTitleColor, 2), shade(activeTitleColor, 2),
					 0, 0, 0, 1, 0.4);
		
		p2.drawPixmap(r.x(), r.y(), shine);

		if (!titleGradientBottom.isNull())
			p2.drawTiledPixmap(r, titleGradientBottom);
	} else {
		p2.setPen(shade(inactiveTitleColor, 1.2));
		p2.drawLine(0, 1, w - 1, 1);
	}

	if (! window()->isMaximized()) {
		// Shading around the left button edges
		if (window()->isActive()) {
			p2.setPen(shade(activeButtonColor, 1.2));
			p2.drawLine(3, 2, 4, 2);
			p2.drawLine(2, 3, 2, 4);
		}
   
		// Shading around the right button edge
		p2.setPen(shade(window()->isActive() ? activeButtonColor : inactiveTitleColor, 1.1));
		p2.drawLine(w-5, 2, w-4, 2);
		p2.setPen(shade(window()->isActive() ? activeButtonColor : inactiveTitleColor, 0.9));
		p2.drawLine(w-3, 3, w-3, 4);
		p2.setPen(shade(window()->isActive() ? activeButtonColor : inactiveTitleColor, 0.85));
		p2.drawLine(w-2, 5, w-2, TOP_GRABBAR_WIDTH + m_titleHeight);
	}

	// Draw text
	QFont fnt = settings()->font();
	p2.setFont( fnt );
	if (window()->isActive()) {
		p2.setPen(shade(activeTitleColor, 0.4));
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

	// Draw edge between titlebar and window contents
	p2.setPen(window()->isActive() ? shade(activeButtonDark, 0.9) : shade(inactiveTitleColor, 0.6));
    p2.drawLine(0, m_titleHeight + TOP_GRABBAR_WIDTH,
				w-1, m_titleHeight + TOP_GRABBAR_WIDTH);
	if (window()->isActive()) {
		p2.setPen(shade(activeTitleColor, 0.1));
		p2.drawLine(r.x(), m_titleHeight + TOP_GRABBAR_WIDTH,
					r.x() + r.width(), m_titleHeight + TOP_GRABBAR_WIDTH);
	}

	// Draw button separators
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
						
			if ((! window()->isMaximized()) && isButtonRight)
				continue;

			QRectF buttonRect = button->geometry();
			p2.setPen(activeButtonDark);
			p2.drawLine (buttonRect.x() + buttonRect.width(), TOP_GRABBAR_WIDTH - 1,
						 buttonRect.x() + buttonRect.width(), TOP_GRABBAR_WIDTH + m_titleHeight);
		}
	}

	// Draw dividers
	p2.setPen(window()->isActive() ? blend(activeTitleColor, Qt::black, 0.4) :
			  blend(inactiveTitleColor, window()->color(KDecoration3::ColorGroup::Inactive,
														KDecoration3::ColorRole::Foreground), 0.3));
	if (drawLeftDivider)
	{
		p2.drawLine (titleBar().x() , 1, titleBar().x() , m_titleHeight + TOP_GRABBAR_WIDTH);
	}
	if (drawRightDivider)
	{
		p2.drawLine (titleBar().x() + titleBar().width() - 1, 1,
					 titleBar().x() + titleBar().width() - 1 , m_titleHeight + TOP_GRABBAR_WIDTH);
	}

	// Paint the buttons on to the title buffer
	m_leftButtons->paint(&p2, repaintRegion);
	m_rightButtons->paint(&p2, repaintRegion);

	p2.end();

	if (! window()->isMaximized()) {
		// Create a bitmap mask for the title buffer (to round off the corners)
		// This is only applied if the window isn't maximized
		QBitmap titleMask(titleBuffer.width(), titleBuffer.height());
		titleMask.fill(Qt::color1);
		QPainter maskPainter(&titleMask);
		maskPainter.setPen(Qt::color0);

		// Top left corner mask
		maskPainter.drawLine(0, 0, 4, 0);
		maskPainter.drawLine(0, 1, 2, 1);
		maskPainter.drawLine(0, 2, 1, 2);
		maskPainter.drawLine(0, 3, 0, 4);

		// Top right corner mask
		maskPainter.drawLine(w-5, 0, w-1, 0);
		maskPainter.drawLine(w-3, 1, w-1, 1);
		maskPainter.drawLine(w-2, 2, w-1, 2);
		maskPainter.drawLine(w-1, 3, w-1, 4);

		titleBuffer.setMask(titleMask);
	}

	QPainter p1(&decoBuffer); // Painter for the main decoration buffer
	
	p1.drawPixmap(0, 0, titleBuffer); // Paint the title buffer

	if (! window()->isMaximized()) {
		// Draw the border bevel
		int sideStart = m_titleHeight + TOP_GRABBAR_WIDTH + 1;

		// Left border bevel
		p1.fillRect(1, sideStart, BORDER_WIDTH, h - CORNER_HEIGHT - sideStart, activeWindowColor);
		p1.setPen(blend(activeLightColor, Qt::white, 0.7));
		p1.drawLine(1, sideStart, 1, h-CORNER_HEIGHT);
		p1.setPen(activeDarkColor);
		p1.drawLine(BORDER_WIDTH-1, sideStart, BORDER_WIDTH-1, h - CORNER_HEIGHT);
	
		// Bottom border bevel
		p1.fillRect(CORNER_HEIGHT, h - BORDER_WIDTH, w - 2*CORNER_HEIGHT, BORDER_WIDTH, activeWindowColor);
		p1.setPen(blend(activeWindowColor, Qt::black, 0.2));
		p1.drawLine(CORNER_HEIGHT, h-2, w - CORNER_HEIGHT, h-2);
		p1.setPen(blend(activeLightColor, Qt::white, 0.7));
		p1.drawLine(CORNER_HEIGHT, h - BORDER_WIDTH + 1, w - CORNER_HEIGHT, h - BORDER_WIDTH + 1);
		p1.setPen(activeDarkColor);
		p1.drawLine(CORNER_HEIGHT, h - BORDER_WIDTH, w - CORNER_HEIGHT, h - BORDER_WIDTH);

		// Right border bevel
		p1.fillRect(w - BORDER_WIDTH, sideStart, BORDER_WIDTH, h - CORNER_HEIGHT - sideStart, activeWindowColor);
		p1.setPen(activeDarkColor);
		p1.drawLine(w - BORDER_WIDTH, sideStart, w - BORDER_WIDTH, h - CORNER_HEIGHT);
		p1.setPen(blend(activeLightColor, Qt::white, 0.7));
		p1.drawLine(w - BORDER_WIDTH + 1, sideStart, w - BORDER_WIDTH + 1, h - CORNER_HEIGHT);
		p1.setPen(blend(activeWindowColor, Qt::black, 0.2));
		p1.drawLine(w-2, sideStart, w-2, h - CORNER_HEIGHT);
	
		// Draw the black border edges
		p1.setPen(Qt::black);

		// Sides
		p1.drawLine(w-1, 5, w-1, h-6);
		p1.drawLine(0, 5, 0, h-6);

		// Top/bottom
		p1.drawLine(5, 0, w-6, 0);
		p1.drawLine(5, h-1, w-6, h-1);

		// Top left corner
		p1.drawLine(4, 1, 3, 1);
		p1.drawPoint(2, 2);
		p1.drawLine(1, 3, 1, 4);

		// Top right corner
		p1.drawLine(w-5, 1, w-4, 1);
		p1.drawPoint(w-3, 2);
		p1.drawLine(w-2, 3, w-2, 4);

		// Bottom left corner
		p1.drawLine(4, h-2, 3, h-2);
		p1.drawPoint(2, h-3);
		p1.drawLine(1, h-4, 1, h-5);

		// Bottom right corner
		p1.drawLine(w-5, h-2, w-4, h-2);
		p1.drawPoint(w-3, h-3);
		p1.drawLine(w-2, h-4, w-2, h-5);

		// Put on the bottom corners
		p1.drawPixmap(0, h - bottomLeftPix.height(),
					  window()->isActive() ? abottomLeftPix : bottomLeftPix);
		p1.drawPixmap(w - bottomRightPix.width(), h - bottomRightPix.height(), 
					  window()->isActive() ? abottomRightPix : bottomRightPix);
	} else {
		// If the window is maximized, just draw a black border along the top
		p1.setPen(Qt::black);
		p1.drawLine(0, 0, w, 0);
	}

	p1.end();
	
	// Draw the decoration buffer
	p->drawPixmap(rect().x(),rect().y(), decoBuffer);

}

BluecurveButton::BluecurveButton(KDecoration3::DecorationButtonType type,
								 KDecoration3::Decoration *decoration,
								 QObject *parent)
	: KDecoration3::DecorationButton(type, decoration, parent)
{	
	setGeometry(QRectF(0,0,14,14)); // default to 14x14 as a backup
	
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
		iconBits = decoration->window()->isMaximized() ? QBitmap::fromData(QSize(14,14), minmax_bits)
			: QBitmap::fromData(QSize(14,14), maximize_bits);
		connect(decoration->window(), &KDecoration3::DecoratedWindow::maximizedChanged, this, &BluecurveButton::onMaximizedChanged);
		break;
	case KDecoration3::DecorationButtonType::Close:
		iconBits = QBitmap::fromData(QSize(14,14), close_bits);
		break;
	case KDecoration3::DecorationButtonType::ContextHelp:
		iconBits = QBitmap::fromData(QSize(14,14), question_bits);
		break;		
	default:
		iconBits = QBitmap();
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
BluecurveButton::onMaximizedChanged()
{
	if (type() == KDecoration3::DecorationButtonType::Maximize)
		iconBits = decoration()->window()->isMaximized() ? QBitmap::fromData(QSize(14,14), minmax_bits)
			: QBitmap::fromData(QSize(14,14), maximize_bits);

	update();
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

	// Button background
	QPixmap btnbg;

	btnbg = decoration()->window()->isActive() ? btnPix : ibtnPix;
	p1.drawPixmap(0,0,btnbg);

	// Apply prelight / dark tint as necessary
    if (isPressed()) {
		QColor tint = shade(decoration()->window()->color(QPalette::ColorGroup::Active,
														  QPalette::ColorRole::Button), 0.75);
		p1.setOpacity(0.5);
		p1.fillRect(buttonBuffer.rect(), tint);
		p1.setOpacity(1.0);
	} else if (isHovered()) {
		QColor tint = decoration()->window()->palette().midlight().color();
		p1.setOpacity(0.4);
		p1.fillRect(buttonBuffer.rect(), tint);
		p1.setOpacity(1.0);
	}

	if (!iconBits.isNull()) {
		// Button icon
		int xOff = (w-14)/2 + 1;
		int yOff = (h-14)/2;
		
		QPixmap icon(iconBits.size());
		icon.fill(decoration()->window()->isActive() ?
				  decoration()->window()->color(
					  QPalette::ColorGroup::Active,
					  QPalette::ColorRole::ButtonText) :
				  decoration()->window()->color(
					  KDecoration3::ColorGroup::Inactive,
					  KDecoration3::ColorRole::Foreground));
				  
		icon.setMask(iconBits);
		p1.setOpacity(0.7);
		p1.drawPixmap(xOff, yOff, icon);
		if (isHovered() && !isPressed()) // If button is being hovered, draw the pixmap twice
			p1.drawPixmap(xOff, yOff, icon);
		p1.setOpacity(1.0);		
	} else {
		QPixmap icon;
		int xOff, yOff;
		if (type() == KDecoration3::DecorationButtonType::OnAllDesktops) {
			if (decoration()->window()->isActive())
				icon = isChecked() ? pinDownPix : pinUpPix;
			else
				icon = isChecked() ? ipinDownPix : ipinUpPix;
			xOff = (w-14)/2;
			yOff = (h-14)/2 - 1;
		} else {
			int iconSize = std::min(w-2, h-2);
			xOff = (w-iconSize)/2 + 1;
			yOff = (h-iconSize)/2;
			icon = decoration()->window()->icon().pixmap(iconSize,iconSize);
		}
		
		p1.drawPixmap(xOff,yOff,icon);
	}
	
	p1.end();

	// Apply the mask (for rounded edges) if the window isn't minimized
	if (! decoration()->window()->isMaximized())		
		buttonBuffer.setMask(buttonMask());
	
	p->drawPixmap(x,y,buttonBuffer);
}

QBitmap
BluecurveButton::buttonMask()
{
	// Obtain button bounds
	int w  = geometry().width();
	int h  = geometry().height();

	QBitmap mask(w, h);
	mask.fill(Qt::color1);

	QPainter p(&mask);
	p.setPen(Qt::color0);

	if (geometry().x()==1) {
		p.drawLine(0, 0, 3, 0);
		p.drawLine(0, 1, 0, 2);
		p.drawLine(1, 1, 1, 2);
	} else if (geometry().x()==decoration()->size().width() - w - 2) {
		p.drawLine(w-3, 0, w-1, 0);
		p.drawLine(w-1, 1, w-1, 2);
	}

	p.end();
	return mask;
}

#include "bluecurvedecoration.moc"
