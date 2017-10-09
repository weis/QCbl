#ifndef QMLGUI_H
#define QMLGUI_H

#include <QQuickWindow>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>

class AppData;

class QmlGui : public QObject {
    Q_OBJECT

public:
    explicit QmlGui(QObject* parent = 0);
    ~QmlGui();
    static void clean();

    bool createMainWnd();
    Q_INVOKABLE QQuickWindow* mainWnd() const
    {
        return m_wndMain;
    }

    QQmlContext* getQmlContext();


private:
    QQuickWindow* loadAppWindow(const QString& qurl);

    QQmlEngine* m_qmlengine;
    QQuickWindow* m_wndMain;

};

#endif // QMLGUI_H
