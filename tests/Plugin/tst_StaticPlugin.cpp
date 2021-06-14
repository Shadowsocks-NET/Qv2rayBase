#include <QObject>
#include <QtTest>

class StaticPluginLoaderTest : public QObject
{
    Q_OBJECT
  public:
    StaticPluginLoaderTest(QObject *parent = nullptr) : QObject(parent){};
};

QTEST_MAIN(StaticPluginLoaderTest)
#include "tst_StaticPlugin.moc"
