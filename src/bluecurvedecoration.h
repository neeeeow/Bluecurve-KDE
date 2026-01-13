#pragma once

#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/Decoration>

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
	QBitmap doShape();
};
