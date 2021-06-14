#pragma once

#include "Qv2rayBaseFeatures.hpp"
#include "Qv2rayBase_export.h"

#include <QtCore>
#include <QtDebug>

// Base folder suffix.
#ifdef QT_DEBUG
#define _BOMB_ (static_cast<QObject *>(nullptr)->event(nullptr))
#else
#define _BOMB_
#endif

#define NEWLINE "\n"

#define QvLog() qInfo() << QV_MODULE_NAME << ":"
#define QvDebug() qDebug() << QV_MODULE_NAME << ":"

namespace Qv2rayBase
{
    Q_NAMESPACE

    // clang-format off
    namespace Models  { struct Qv2rayBaseConfigObject; }
    namespace Plugin  { class LatencyTestHost; }
    namespace Plugin  { class PluginManagerCore; }
    namespace Plugin  { class PluginAPIHost; }
    namespace Profile { class ProfileManager; }
    namespace Profile { class KernelManager; }
    namespace Interfaces { class IStorageProvider; };
    namespace Interfaces { class IConfigurationGenerator; }
    // clang-format on

    enum QV2RAYBASE_FAILED_REASON
    {
        NORMAL = 0,

        // Anything larger than this value is considered to be an error.
        ERROR = 100,
        INITIALIZATION_FAILED,
        PRECONDITION_FAILED,
    };
    Q_ENUM_NS(QV2RAYBASE_FAILED_REASON)

    enum Qv2rayStartFlags
    {
        NO_PLUGINS,
    };
    Q_ENUM_NS(Qv2rayStartFlags)

    class Qv2rayBaseLibraryPrivate;
    ///
    /// \brief The Qv2rayBaseLibrary class
    /// This is the class used by Qv2ray baselib, and must be initialized before using the library,
    ///
    class QV2RAYBASE_EXPORT Qv2rayBaseLibrary
    {
      public:
        enum class MessageOpt
        {
            // clang-format off
            OK,     Cancel,
            Yes,    No,
            Ignore
            // clang-format on
        };

      public:
        ///
        /// \brief Qv2rayBaseLibrary
        /// construct a Qv2rayBaseLibrary instance, the constructor should only be called once during the
        /// whole application lifetime.
        explicit Qv2rayBaseLibrary();
        ~Qv2rayBaseLibrary();

        QV2RAYBASE_FAILED_REASON Initialize(QFlags<Qv2rayStartFlags> flags, Interfaces::IConfigurationGenerator *g = nullptr, Interfaces::IStorageProvider *s = nullptr);

      public:
        ///
        /// \brief Get current instance of Qv2ray base library, returns nullptr when the library is not initialized.
        /// \return returns the pointer to the current instance of Qv2ray base library
        ///
        static Qv2rayBaseLibrary *instance();

        ///
        /// \brief Get configurations for Qv2ray base library.
        ///
        static Qv2rayBase::Models::Qv2rayBaseConfigObject *GetConfig();

        ///
        /// \brief Get search paths for assets, (e.g. when searching for plugins)
        /// \param dirName The directory suffix name used to search, (e.g. "plugins")
        ///
        static QStringList GetAssetsPaths(const QString &dirName);

        ///
        /// \brief Warn Show a warning message to user
        /// \param title The title of message
        /// \param text The content of message
        ///
        static void Warn(const QString &title, const QString &text);

        ///
        /// \brief Warn Show an information message to user
        /// \param title The title of message
        /// \param text The content of message
        ///
        static void Info(const QString &title, const QString &text);

        ///
        /// \brief Warn Let user to make a choice
        /// \param title The title of message
        /// \param text The content of message
        /// \param options The possible options to the user
        ///
        static MessageOpt Ask(const QString &title, const QString &text, const QList<MessageOpt> &options = { MessageOpt::Yes, MessageOpt::No });

        ///
        /// \brief OpenURL Opens a URL in external app
        /// \param url the URL to be opened
        ///
        static void OpenURL(const QUrl &url);

        ///
        /// \brief PluginAPIHost returns Plugin API host
        /// \return The pointer to the Plugin API Host
        ///
        static Qv2rayBase::Plugin::PluginAPIHost *PluginAPIHost();

        ///
        /// \brief PluginManagerCore Get the core plugin manager
        /// \return The pointer to the core plugin manager
        ///
        static Qv2rayBase::Plugin::PluginManagerCore *PluginManagerCore();

        ///
        /// \brief LatencyTestHost Get the latency tester host
        /// \return The pointer to the latency test host.
        static Qv2rayBase::Plugin::LatencyTestHost *LatencyTestHost();

        ///
        /// \brief ProfileManager Get the profile manager.
        /// \return The pointer to the profile manager.
        ///
        static Qv2rayBase::Profile::ProfileManager *ProfileManager();

        ///
        /// \brief KernelManager Get the kernel manager.
        /// \return The pointer to the kernel manager.
        ///
        static Qv2rayBase::Profile::KernelManager *KernelManager();

        ///
        /// \brief ConfigurationGenerator Get the configuration generator, mostly for applying custom configuration filters or chain expansions
        /// \return The pointer to the configuration generator
        ///
        static Qv2rayBase::Interfaces::IConfigurationGenerator *ConfigurationGenerator();

        ///
        /// \brief StorageProvider Get the storage provider, implemented by downstream to provide Qv2rayBase a stable storage to place connections.
        /// \return The pointer to the storage provider
        ///
        static Qv2rayBase::Interfaces::IStorageProvider *StorageProvider();

      protected:
        virtual void p_MessageBoxWarn(const QString &title, const QString &text) = 0;
        virtual void p_MessageBoxInfo(const QString &title, const QString &text) = 0;
        virtual MessageOpt p_MessageBoxAsk(const QString &title, const QString &text, const QList<MessageOpt> &options) = 0;
        virtual void p_OpenURL(const QUrl &url) = 0;

      private:
        Q_DECLARE_PRIVATE_D(d_ptr, Qv2rayBaseLibrary)
        QScopedPointer<Qv2rayBaseLibraryPrivate> d_ptr;
    };

} // namespace Qv2rayBase

// clang-format off
#define QvBaselib                           Qv2rayBase::Qv2rayBaseLibrary::instance()
#if 0
    #define QvKernelManager                 Qv2rayBase::Qv2rayBaseLibrary::KernelManager()
    #define QvProfileManager                Qv2rayBase::Qv2rayBaseLibrary::ProfileManager()
    #define QvPluginManagerCore             Qv2rayBase::Qv2rayBaseLibrary::PluginManagerCore()
    #define QvPluginHost                    Qv2rayBase::Qv2rayBaseLibrary::PluginAPIHost()
    #define QvConfigurationGenerator        Qv2rayBase::Qv2rayBaseLibrary::ConfigurationGenerator()
    #define QvStorageProvider               Qv2rayBase::Qv2rayBaseLibrary::StorageProvider()
#endif
// clang-format on
