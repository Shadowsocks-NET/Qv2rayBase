//  Qv2rayBase, the modular feature-rich infrastructure library for Qv2ray.
//  Copyright (C) 2021 Moody and relavent Qv2ray contributors.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "Qv2rayBase/Interfaces/IStorageProvider.hpp"
#include "Qv2rayBase/Plugin/PluginManagerCore.hpp"
#include "Qv2rayBase/Qv2rayBaseLibrary.hpp"
#include "QvPlugin/PluginInterface.hpp"
#include "TestCommon.hpp"

#include <QtTest>

class TestStaticPlugin
    : public QObject
    , public Qv2rayPlugin::Qv2rayInterface

{
    Q_OBJECT
    Q_INTERFACES(Qv2rayPlugin::Qv2rayInterface)
    Q_PLUGIN_METADATA(IID Qv2rayInterface_IID)
  public:
    TestStaticPlugin(QObject *parent = nullptr) : QObject(parent){};
    virtual const Qv2rayPlugin::QvPluginMetadata GetMetadata() const override
    {
        return Qv2rayPlugin::QvPluginMetadata{ QStringLiteral("Static Plugin Loader Test Plugin"), //
                                               QStringLiteral("Moody"),                            //
                                               PluginId(QStringLiteral("static_plugin_test")),
                                               QStringLiteral(""),
                                               QStringLiteral(""),
                                               {} };
    }
    virtual bool InitializePlugin() override
    {
        return true;
    }
    virtual void SettingsUpdated() override
    {
    }
  signals:
    void PluginLog(QString) override;
    void PluginErrorMessageBox(QString, QString) override;
};

class PluginLoaderTest : public QObject
{
    Q_OBJECT
  public:
    PluginLoaderTest(QObject *parent = nullptr) : QObject(parent){};

  private slots:
    void initTestCase()
    {
    }

    void testWithoutPlugins()
    {
        baselib = new Qv2rayBase::Qv2rayBaseLibrary;
        baselib->Initialize({ Qv2rayBase::START_NO_PLUGINS }, {}, new Qv2rayBase::Tests::UIInterface);
        QCOMPARE(baselib->PluginManagerCore()->GetPlugin(PluginId(QStringLiteral("static_plugin_test"))), nullptr);
        delete baselib;
    }

    void testWithPlugins()
    {
        baselib = new Qv2rayBase::Qv2rayBaseLibrary;
        baselib->Initialize({}, {}, new Qv2rayBase::Tests::UIInterface);
        const auto pluginInfo = baselib->PluginManagerCore()->GetPlugin(PluginId(QStringLiteral("static_plugin_test")));
        QVERIFY2(pluginInfo != nullptr, "PluginInfo should not be nullptr, that is, PluginManagerCore should find the static plugin.");
        QCOMPARE(pluginInfo->libraryPath, "[STATIC]");
        delete baselib;
    }

  private:
    Qv2rayBase::Qv2rayBaseLibrary *baselib;
};

QTEST_MAIN(PluginLoaderTest)
Q_IMPORT_PLUGIN(TestStaticPlugin)

#include "tst_PluginLoader.moc"
