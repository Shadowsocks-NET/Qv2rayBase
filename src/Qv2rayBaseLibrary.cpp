#include "Qv2rayBaseLibrary.hpp"

#include "Plugin/PluginAPIHost.hpp"
#include "Plugin/PluginManagerCore.hpp"
#include "Profile/Generator.hpp"
#include "Profile/KernelManager.hpp"
#include "Profile/ProfileManager.hpp"
#include "StorageProvider.hpp"

// Private headers
#include "src/private/BaseConfigurationGenerator.hpp"
#include "src/private/BaseStorageProvider.hpp"

#include <QDir>
#include <QStandardPaths>

#define QV_MODULE_NAME "BaseApplication"

namespace Qv2rayBase
{
    using namespace Qv2rayBase::Profile;
    using namespace Qv2rayBase::Plugins;
    QJsonObject MigrateSettings(int fromVersion, int toVersion, const QJsonObject &original);
    Qv2rayBaseLibrary *m_instance = nullptr;
    class Qv2rayBaseLibraryPrivate
    {
      public:
        Qv2rayBase::Models::Qv2rayBaseConfigObject *configuration;

        Plugins::PluginAPIHost *pluginAPIHost;
        Plugins::PluginManagerCore *pluginCore;
        Profile::ProfileManager *profileManager;
        Profile::KernelManager *kernelManager;
        QFlags<Qv2rayStartFlags> startupFlags;

        Profile::IConfigurationGenerator *configGenerator;
        IStorageProvider *storageProvider;
    };

    QV2RAYBASE_INITIALIZATION_FAILED_REASON Qv2rayBaseLibrary::Initialize(QFlags<Qv2rayStartFlags> flags, IConfigurationGenerator *gen, IStorageProvider *stor)
    {
        Q_ASSERT_X(m_instance == nullptr, "Qv2rayBaseLibrary", "m_instance is not null! Cannot construct another Qv2rayBaseLibrary when there's one existed");
        m_instance = this;
        Q_D(Qv2rayBaseLibrary);
        d->startupFlags = flags;
        if (stor)
            d->storageProvider = stor;
        else
            d->storageProvider = new _private::Qv2rayBasePrivateStorageProvider;

        if (gen)
            d->configGenerator = gen;
        else
            d->configGenerator = new _private::Qv2rayBasePrivateConfigurationGenerator;

        // TODO load configurations

        d->pluginCore = new Plugins::PluginManagerCore;
        if (!flags.testFlag(NO_PLUGINS))
            d->pluginCore->LoadPlugins();

        d->profileManager = new Profile::ProfileManager;
        d->kernelManager = new Profile::KernelManager;

        return NORMAL;
    }

    Qv2rayBaseLibrary *Qv2rayBaseLibrary::instance()
    {
        Q_ASSERT_X(m_instance, "Qv2rayBaseLibrary", "m_instance is null! Did you forget to construct Qv2rayBaseLibrary?");
        return m_instance;
    }

    Qv2rayBaseLibrary::Qv2rayBaseLibrary()
    {
        qInfo() << "Qv2ray Base Library" << QV2RAY_BASELIB_VERSION << "on" << QSysInfo::prettyProductName() << QSysInfo::currentCpuArchitecture();
        qDebug() << "Qv2ray Start Time:" << QTime::currentTime().msecsSinceStartOfDay();
        d_ptr.reset(new Qv2rayBaseLibraryPrivate);
    }

    Qv2rayBaseLibrary::~Qv2rayBaseLibrary()
    {
        m_instance = nullptr;
    }

    QStringList Qv2rayBaseLibrary::GetAssetsPaths(const QString &dirName)
    {
        static const auto makeAbs = [](const QDir &p) { return p.absolutePath(); };

        // Configuration Path
        QStringList list;

        if (qEnvironmentVariableIsSet("QV2RAY_RESOURCES_PATH"))
            list << makeAbs(qEnvironmentVariable("QV2RAY_RESOURCES_PATH") + "/" + dirName);

        // Default behavior on Windows
        list << makeAbs(QCoreApplication::applicationDirPath() + "/" + dirName);

        // WARNING BREAKING CHANGE
        // list << makeAbs(instance()->d_ptr->configurationPaths + dirName);
        list << ":/" + dirName;

        list << QStandardPaths::locateAll(QStandardPaths::AppDataLocation, dirName, QStandardPaths::LocateDirectory);
        list << QStandardPaths::locateAll(QStandardPaths::AppConfigLocation, dirName, QStandardPaths::LocateDirectory);

#ifdef Q_OS_UNIX
        if (qEnvironmentVariableIsSet("APPIMAGE"))
            list << makeAbs(QCoreApplication::applicationDirPath() + "/../share/qv2ray/" + dirName);

        if (qEnvironmentVariableIsSet("SNAP"))
            list << makeAbs(qEnvironmentVariable("SNAP") + "/usr/share/qv2ray/" + dirName);

        if (qEnvironmentVariableIsSet("XDG_DATA_DIRS"))
            list << makeAbs(qEnvironmentVariable("XDG_DATA_DIRS") + "/" + dirName);

        list << makeAbs("/usr/local/share/qv2ray/" + dirName);
        list << makeAbs("/usr/local/lib/qv2ray/" + dirName);
        list << makeAbs("/usr/share/qv2ray/" + dirName);
        list << makeAbs("/usr/lib/qv2ray/" + dirName);
        list << makeAbs("/lib/qv2ray/" + dirName);
#endif

#ifdef Q_OS_MAC
        // macOS platform directories.
        list << QDir(QCoreApplication::applicationDirPath() + "/../Resources/" + dirName).absolutePath();
#endif

        list << instance()->StorageProvider()->GetAssetsPath(dirName);

        list.removeDuplicates();
        return list;
    }

    void Qv2rayBaseLibrary::Warn(const QString &title, const QString &text)
    {
        return instance()->p_MessageBoxWarn(title, text);
    }

    void Qv2rayBaseLibrary::Info(const QString &title, const QString &text)
    {
        return instance()->p_MessageBoxInfo(title, text);
    }

    Qv2rayBaseLibrary::MessageOpt Qv2rayBaseLibrary::Ask(const QString &title, const QString &text, const QList<MessageOpt> &options)
    {
        return instance()->p_MessageBoxAsk(title, text, options);
    }

    Plugins::PluginAPIHost *Qv2rayBaseLibrary::PluginAPIHost()
    {
        return instance()->d_ptr->pluginAPIHost;
    }

    Qv2rayBase::Models::Qv2rayBaseConfigObject *Qv2rayBaseLibrary::GetConfig()
    {
        return instance()->d_ptr->configuration;
    }

    PluginManagerCore *Qv2rayBaseLibrary::PluginManagerCore()
    {
        return instance()->d_ptr->pluginCore;
    }

    IConfigurationGenerator *Qv2rayBaseLibrary::ConfigurationGenerator()
    {
        return instance()->d_ptr->configGenerator;
    }

    IStorageProvider *Qv2rayBaseLibrary::StorageProvider()
    {
        return instance()->d_ptr->storageProvider;
    }

    ProfileManager *Qv2rayBaseLibrary::ProfileManager()
    {
        return instance()->d_ptr->profileManager;
    }

    KernelManager *Qv2rayBaseLibrary::KernelManager()
    {
        return instance()->d_ptr->kernelManager;
    }
} // namespace Qv2rayBase
