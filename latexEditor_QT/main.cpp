#ifdef QT_WIDGETS_LIB
#include <QApplication>
#else
#include <QGuiApplication>
#endif
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QFontDatabase>
#include <QQmlContext>
#include <QQmlFileSelector>
#include <QQuickStyle>

#include "texsetting.h"
#include "datatypes.h"
#include "latexhandler.h"


int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    qmlRegisterType<LatexHandler>("com.latexeditor", 0, 1, "LatexHandler");

#ifdef QT_WIDGETS_LIB
    QApplication app(argc, argv);
#else
    QGuiApplication app(argc, argv);
#endif
    QFontDatabase fontDatabase;
    if (fontDatabase.addApplicationFont(":/fonts/materialdesignicons.ttf") == -1)
        qWarning() << "Failed to load materialdesignicons.ttf";

    QCoreApplication::setOrganizationName("com");
    QCoreApplication::setApplicationName("Latex xwysiwyg editor");
    texSetting::setPath(texSetting::IniFormat,texSetting::UserScope
                        ,".");
    //texSetting setting("com","Latex xwysiwyg editor");
    texSetting setting("settings.ini", QSettings::IniFormat);
    //qDebug()<<setting.value("docId");
    QJsonObject session;
    setting.setValue("serverUrl","localhost");
    setting.setValue("servelPort",3435);
    setting.setValue("mathJaxUrl","http://127.0.0.1:3434/mathjax");
    setting.setValue("session",session.toVariantMap());//reference?
    setting.setValue("defalut_mode",DISPLAY_VIEW);
//qDebug()<<setting.value("docId");
    latexClient client(setting);
//TODO onchanged change client setting

    //LatexHandler handler(&client,&setting);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("setting", &setting);
    engine.rootContext()->setContextProperty("client", &client);
    engine.rootContext()->setContextProperty("session", session.toVariantMap());//reference?

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    //, (Qt::ConnectionType)(Qt::AutoConnection | Qt::UniqueConnection)
    engine.load(url);

    return app.exec();
}
