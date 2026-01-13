#pragma once

#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/Decoration>
#include <KDecoration3/DecorationButton>

#include <QMargins>
#include <QVariant>

class BluecurveDecoration : public KDecoration3::Decoration
{
	Q_OBJECT

public:
	explicit BluecurveDecoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());
	~BluecurveDecoration() override;

	bool init() override;
	void paint(QPainter *p, const QRectF &repaintRegion) override;
private:
	void createPixmaps();
	void updateBorders();
	void updateTitleBar();
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

	void paint(QPainter *p, const QRectF &repaintRegion) override;
};
