#pragma once

#include <QWidget>
#include <QStaticText>
#include <QPropertyAnimation>

class QLabel;

class Notification : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit Notification(const QString& text, QWidget* parent = nullptr);
    explicit Notification(const QString& text, const QFont& font, int milliseconds, QWidget* parent = nullptr);
    virtual ~Notification();

public:
    void setTextColor(QColor& color) { m_textColor = color; }
    void showImmediatly();
    void run();

    void fadeOut();

    void setOpacity(qreal opacity);
    qreal opacity() const;

public:
    static void showMessage(const QString& message, QWidget* parent);
    static void showMessage(const QString& message, QString& url, QRect rect, int milliseconds, QWidget* parent);
    static void showMessage(const QString& message, const QFont& font, QWidget* parent);
    static void showMessage(const QString& message, const QFont& font, int milliseconds, QWidget* parent);
    static void showMessage(const QString& message, QColor color, QRect rect, int milliseconds, QWidget* parent);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent *event);

private:
    QStaticText m_label;
    qreal m_opacity;
    int m_milliseconds;
    QString m_url;
    QPropertyAnimation* m_animation;
    QColor m_textColor;
};
