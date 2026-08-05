#pragma once

#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/Decoration>
#include <KDecoration3/DecorationButton>
#include <KDecoration3/DecorationButtonGroup>

#include <QMargins>
#include <QVariant>
#include <QBitmap>
#include <QPixmap>

#define TOP_GRABBAR_WIDTH     2
#define TITLEBAR_BORDER_WIDTH 1

class BluecurveDecoration : public KDecoration3::Decoration
{
	Q_OBJECT

public:
	explicit BluecurveDecoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());
	~BluecurveDecoration() override;

	bool init() override;
	void paint(QPainter *p, const QRectF &repaintRegion) override;

private:
	int m_titleHeight = 14;
	int m_buttonSize = 14;
	
	QPixmap titlePix;
	QPixmap iTitleGradient;

	QPixmap titleBlockerBottom;
	QPixmap titleGradientBottom;

	QPixmap bottomLeftPix;
	QPixmap bottomRightPix;
	QPixmap abottomLeftPix;
	QPixmap abottomRightPix;
	
	KDecoration3::DecorationButtonGroup *m_leftButtons = nullptr;
	KDecoration3::DecorationButtonGroup *m_rightButtons = nullptr;

	void reconfigure();
	void updateBorders();
	void createPixmaps();
	void updateTitleBar();
	void updateButtonsGeometryDelayed();
	void updateButtonsGeometry();

	inline int calcTopBorder() const {
		return m_titleHeight + TOP_GRABBAR_WIDTH + TITLEBAR_BORDER_WIDTH;
	}
};

class BluecurveButton : public KDecoration3::DecorationButton
{
	Q_OBJECT

public:
	explicit BluecurveButton(KDecoration3::DecorationButtonType type,
							 KDecoration3::Decoration *decoration,
							 QObject *parent = nullptr);
	~BluecurveButton() override;

	static BluecurveButton *create(KDecoration3::DecorationButtonType type,
								   KDecoration3::Decoration *decoration,
								   QObject *parent);

	void paint(QPainter *p, const QRectF &repaintRegion) override;
private:	
	QBitmap iconBits;

	void onMaximizedChanged();
	QBitmap buttonMask();
};
