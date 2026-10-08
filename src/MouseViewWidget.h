#ifndef MOUSEVIEWWIDGET_H
#define MOUSEVIEWWIDGET_H

#include <QWidget>
#include <QPainterPath>

class MouseViewWidget : public QWidget {
    Q_OBJECT

public:
    explicit MouseViewWidget(QWidget *parent = nullptr);

signals:
    void mouseButtonClicked(int buttonId, const QString& buttonName);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    struct MouseRegion {
        int id;
        QPainterPath path;
        QColor color;
        QString name;
    };

    QList<MouseRegion> regions;
    int hoveredRegion = -1;

    void setupRegions();
};

#endif // MOUSEVIEWWIDGET_H
