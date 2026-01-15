#pragma once

#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/Decoration>
#include <KDecoration3/DecorationButton>
#include <KDecoration3/DecorationButtonGroup>

#include <QMargins>
#include <QVariant>
#include <QBitmap>

class BluecurveDecoration : public KDecoration3::Decoration
{
	Q_OBJECT

public:
	explicit BluecurveDecoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());
	~BluecurveDecoration() override;

	bool init() override;
	void paint(QPainter *p, const QRectF &repaintRegion) override;
private:
	KDecoration3::DecorationButtonGroup *m_leftButtons = nullptr;
	KDecoration3::DecorationButtonGroup *m_rightButtons = nullptr;
	
	void createPixmaps();
	void updateBorders();
	void updateTitleBar();
	void updateButtonsGeometryDelayed();
	void updateButtonsGeometry();
	QBitmap decorationMask();
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

	QBitmap buttonMask();
};
