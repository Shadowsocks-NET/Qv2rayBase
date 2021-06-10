#pragma once

#include "Qv2rayBaseFeatures.hpp"
#include "qv2raybase_export.h"

#include <QtCore>

// Base folder suffix.
#ifdef QT_DEBUG
#define QV2RAY_CONFIG_DIR_SUFFIX "_debug/"
#define _BOMB_ (static_cast<QObject *>(nullptr)->event(nullptr))
#else
#define QV2RAY_CONFIG_DIR_SUFFIX "/"
#define _BOMB_
#endif

#define NEWLINE "\n"

// constexpr auto OUTBOUND_TAG_BLACKHOLE = "BLACKHOLE";
// constexpr auto OUTBOUND_TAG_DIRECT = "DIRECT";
// constexpr auto OUTBOUND_TAG_PROXY = "PROXY";
// constexpr auto OUTBOUND_TAG_FORWARD_PROXY = "QV2RAY_FORWARD_PROXY";

// constexpr auto API_TAG_DEFAULT = "QV2RAY_API";
// constexpr auto API_TAG_INBOUND = "QV2RAY_API_INBOUND";

// constexpr auto QV2RAY_USE_FPROXY_KEY = "_QV2RAY_USE_GLOBAL_FORWARD_PROXY_";

#define DEBUG(...)
#define LOG(...)

namespace Qv2rayBase
{
    Q_NAMESPACE

    // clang-format off
    class IStorageProvider;
    namespace Models { struct Qv2rayBaseConfigObject; }
    namespace Plugins { class PluginManagerCore; }
    namespace Plugins { class PluginAPIHost; }
    namespace Profile { class ProfileManager; }
    namespace Profile { class KernelManager; }
    namespace Profile { class IConfigurationGenerator; }
    // clang-format on

    enum QV2RAYBASE_INITIALIZATION_FAILED_REASON
    {
        NORMAL = 0,

        // Anything larger than this value is considered to be an error.
        ERROR = 100,
        INITIALIZATION_FAILED,
        PRECONDITION_FAILED,
    };
    Q_ENUM_NS(QV2RAYBASE_INITIALIZATION_FAILED_REASON)

    enum Qv2rayStartFlags
    {
        NO_API,
        NO_AUTOMATIC_CONNECTION,
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
        enum MessageOpt
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

        QV2RAYBASE_INITIALIZATION_FAILED_REASON Initialize(QFlags<Qv2rayStartFlags> flags, IStorageProvider *storageProvider);

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
        /// \brief Get the path to the configuration file.
        ///
        static QString GetConfigPath();

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
        static MessageOpt Ask(const QString &title, const QString &text, const QList<MessageOpt> &options = { Yes, No });

        ///
        /// \brief OpenURL Opens a URL in external app
        /// \param url the URL to be opened
        ///
        static void OpenURL(const QUrl &url);

        ///
        /// \brief PluginAPIHost returns Plugin API host
        /// \return The pointer to the Plugin API Host
        ///
        static Qv2rayBase::Plugins::PluginAPIHost *PluginAPIHost();

        ///
        /// \brief PluginManagerCore Get the core plugin manager
        /// \return The pointer to the core plugin manager
        ///
        static Qv2rayBase::Plugins::PluginManagerCore *PluginManagerCore();

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
        static Qv2rayBase::Profile::IConfigurationGenerator *ConfigurationGenerator();

        ///
        /// \brief StorageProvider Get the storage provider, implemented by downstream to provide Qv2rayBase a stable storage to place connections.
        /// \return The pointer to the storage provider
        ///
        static Qv2rayBase::IStorageProvider *StorageProvider();

      protected:
        virtual void p_MessageBoxWarn(const QString &title, const QString &text) = 0;
        virtual void p_MessageBoxInfo(const QString &title, const QString &text) = 0;
        virtual MessageOpt p_MessageBoxAsk(const QString &title, const QString &text, const QList<MessageOpt> &options) = 0;
        virtual void p_OpenURL(const QUrl &url) = 0;
        virtual QStringList p_ExtraAssetsPaths(const QString &dirname) = 0;

      private:
        Q_DECLARE_PRIVATE_D(d_ptr, Qv2rayBaseLibrary)
        QScopedPointer<Qv2rayBaseLibraryPrivate> d_ptr;
    };
} // namespace Qv2rayBase
