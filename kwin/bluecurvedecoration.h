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

#pragma once

#include <KDecoration3/Decoration>
#include <KDecoration3/DecorationButton>
#include <KDecoration3/DecorationButtonGroup>

#include <QVariant>
#include <QBitmap>
#include <QPixmap>

struct BluecurvePixmaps
{
	// Titlebar decorations
    QPixmap titlePix;
    QPixmap iTitleGradient;

    QPixmap titleBlockerBottom;
    QPixmap titleGradientBottom;

	// Corners
    QPixmap bottomLeftPix;
    QPixmap bottomRightPix;
    QPixmap abottomLeftPix;
    QPixmap abottomRightPix;

	// Buttons
	QPixmap pinDownPix;
	QPixmap pinUpPix;
	QPixmap btnPix;
};

class BluecurveDecoration : public KDecoration3::Decoration
{
	Q_OBJECT

public:
	explicit BluecurveDecoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());
	~BluecurveDecoration() override = default;

	bool init() override;
	void paint(QPainter *p, const QRectF &repaintRegion) override;

	// Access pixmaps from outside of the decoration class (i.e. inside the button)
	const BluecurvePixmaps &pixmaps() const { return m_pixmaps; }

private:
	BluecurvePixmaps m_pixmaps;
	qreal m_titleHeight = 14;	
	
	KDecoration3::DecorationButtonGroup *m_leftButtons = nullptr;
	KDecoration3::DecorationButtonGroup *m_rightButtons = nullptr;

	void reconfigure();
	void updateBorders();
	void updateButtonsGeometry();
	void updateTitleBar();	
	void createPixmaps();
};

class BluecurveButton : public KDecoration3::DecorationButton
{
	Q_OBJECT

public:
	explicit BluecurveButton(KDecoration3::DecorationButtonType type,
							 KDecoration3::Decoration *decoration,
							 QObject *parent = nullptr);
	~BluecurveButton() override = default;

	static BluecurveButton *create(KDecoration3::DecorationButtonType type,
								   KDecoration3::Decoration *decoration,
								   QObject *parent);

	void paint(QPainter *p, const QRectF &repaintRegion) override;
private:	
	QBitmap iconBits;

	void onMaximizedChanged();
	QBitmap buttonMask();
};
