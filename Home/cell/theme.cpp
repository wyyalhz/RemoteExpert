#include "theme.h"
#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QLabel>
#include <QAbstractButton>
#include <QAction>
#include <QWidget>
#include <QIcon>
#include <QPixmap>

/* 放到 theme.cpp 顶部其它静态函数旁*/
static QString iconKeyFor(QObject* obj) {
    /* 1) 有动态属性就优先用*/
    const QVariant v = obj->property("icon_key");
    if (v.isValid() && !v.toString().isEmpty())
        return v.toString();

    /* 2) 明确映射（根据你的对象名来改/补）*/
    static const QHash<QString, QString> map = {
        {"btnDevices",  "geer"},
        {"btnOrders",   "ticket"},
        {"btnThanks",   "flower"},
        {"btnSettings", "setGeer"},
        {"avatarLabel", "man"},
        /* 如果侧栏是 QAction，也可以加：*/
        {"actionDevices",  "geer"},
        {"actionOrders",   "ticket"},
        {"actionThanks",   "flower"},
        {"actionSettings", "setGeer"},
    };
    const QString name = obj->objectName();
    if (map.contains(name)) return map.value(name);

    /* 3) 通用猜测：去掉常见前缀，转小写*/
    QString n = name;
    if (n.startsWith("btn"))    n = n.mid(3);
    if (n.startsWith("action")) n = n.mid(6);
    if (!n.isEmpty()) {
        QString k = n;
        k[0] = k[0].toLower();
        return k.toLower();
    }
    return {};
}


static Theme g_theme = Theme::Light;

static QString themeFileName(Theme t) {
    return t == Theme::Dark ? "dark.qss" : "light.qss";
}
QString currentThemeName() {
    return (g_theme == Theme::Dark) ? "dark" : "light";
}

static bool loadQssFrom(const QString& path) {
    QFile f(path);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
        return true;
    }
    return false;
}

static bool tryLoadQss(const QString& fname) {
    /* 1) 用户 AppData*/
    const QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!appDataDir.isEmpty()) {
        if (loadQssFrom(QDir(appDataDir).filePath("themes/" + fname))) return true;
    }
    /* 2) 程序目录*/
    const QString appDir = QCoreApplication::applicationDirPath();
#ifdef Q_OS_MAC
    /* 如果你把 qss 放到 .app 的 Resources 里，也可以在这儿加一条*/
#endif
    if (loadQssFrom(QDir(appDir).filePath("themes/" + fname))) return true;

    /* 3) 资源兜底（兼容两种前缀）*/
    if (loadQssFrom(":/themes/" + fname)) return true; /* 我建议的前缀*/
    if (loadQssFrom(":/" + fname)) return true;        /* 你项目里 themes.qrc 的前缀（/）*/

    return false;
}

bool applyTheme(Theme t) {
    g_theme = t;
    const QString fname = themeFileName(t);
    if (!tryLoadQss(fname)) {
        return false;
    }

    /* 记住用户选择*/
    QSettings st("YourOrg", "YourApp");
    st.setValue("theme", currentThemeName());

    refreshThemedAssets();
    return true;
}

static QStringList iconCandidates(const QString& key, Theme t) {
    QStringList c;
    /* 我之前的建议路径*/
    c << (t == Theme::Dark ? ":/icons/dark/" + key + ".svg"
                           : ":/icons/light/" + key + ".svg");

    /* 兼容你现在的 qrc（prefix="/" 且文件名形如 light/<key>-light.svg）*/
    c << (t == Theme::Dark ? ":/dark/"  + key + "-dark.svg"
                           : ":/light/" + key + "-light.svg");

    /* 兼容旧的 static 资源（例如 :/home/geer-black.svg 等）*/
    if (t == Theme::Dark) {
        c << ":/home/" + key + "-white.svg";
        c << ":/home/" + key + "-color.svg"; /* 退而求其次*/
    } else {
        c << ":/home/" + key + "-black.svg";
        c << ":/home/" + key + "-color.svg";
    }
    return c;
}

static QIcon firstExistingIcon(const QStringList& candidates) {
    for (const QString& p : candidates) {
        if (QFile::exists(p)) return QIcon(p);
    }
    return QIcon();
}

static QPixmap firstExistingPixmap(const QStringList& candidates) {
    for (const QString& p : candidates) {
        if (QFile::exists(p)) return QPixmap(p);
    }
    return QPixmap();
}

//void refreshThemedAssets() {
//    const QWidgetList widgets = QApplication::allWidgets();
//    for (QWidget* w : widgets) {
//        /* 1) 按钮：根据 icon_key 切*/
//        if (auto* btn = qobject_cast<QAbstractButton*>(w)) {
//            const QVariant key = btn->property("icon_key");
//            if (key.isValid()) {
//                const auto c = iconCandidates(key.toString(), g_theme);
//                btn->setIcon(firstExistingIcon(c));
//            }
//        }
//        /* 2) 标签：根据 pixmap_key 切*/
//        if (auto* lab = qobject_cast<QLabel*>(w)) {
//            const QVariant key = lab->property("pixmap_key");
//            if (key.isValid()) {
//                const auto c = iconCandidates(key.toString(), g_theme);
//                lab->setPixmap(firstExistingPixmap(c));
//            }
//        }
//        /* 3) QAction：根据 icon_key 切*/
//        const auto actions = w->findChildren<QAction*>();
//        for (QAction* act : actions) {
//            const QVariant key = act->property("icon_key");
//            if (key.isValid()) {
//                const auto c = iconCandidates(key.toString(), g_theme);
//                act->setIcon(firstExistingIcon(c));
//            }
//        }
//    }
//}

void refreshThemedAssets() {
    const QWidgetList widgets = QApplication::allWidgets();
    for (QWidget* w : widgets) {
        // 1) QAbstractButton（QToolButton/QPushButton）
        if (auto* btn = qobject_cast<QAbstractButton*>(w)) {
            const QString key = iconKeyFor(btn);
            if (!key.isEmpty()) {
                const auto c = iconCandidates(key, g_theme);
                btn->setIcon(firstExistingIcon(c));
                btn->setIconSize(QSize(20,20));  // 可按需
            }
        }
//        // 2) QLabel（头像）
//        if (auto* lab = qobject_cast<QLabel*>(w)) {
//            const QString key = iconKeyFor(lab);
//            void refreshThemedAssets() {
//                const QWidgetList widgets = QApplication::allWidgets();
//                for (QWidget* w : widgets) {
//                    // 1) QAbstractButton（QToolButton/QPushButton）
//                    if (auto* btn = qobject_cast<QAbstractButton*>(w)) {
//                        const QString key = iconKeyFor(btn);
//                        if (!key.isEmpty()) {
//                            const auto c = iconCandidates(key, g_theme);
//                            btn->setIcon(firstExistingIcon(c));
//                            btn->setIconSize(QSize(20,20));  // 可按需
//                        }
//                    }
//                    // 2) QLabel（头像）
//                    if (auto* lab = qobject_cast<QLabel*>(w)) {
//                        const QString key = iconKeyFor(lab);
//                        if (!key.isEmpty()) {
//                            const auto c = iconCandidates(key, g_theme);
//                            lab->setPixmap(firstExistingPixmap(c));
//                        }
//                    }
//                    // 3) QAction（如果你的侧栏用的是 QAction 触发）
//                    const auto actions = w->findChildren<QAction*>();
//                    for (QAction* act : actions) {
//                        const QString key = iconKeyFor(act);
//                        if (!key.isEmpty()) {
//                            const auto c = iconCandidates(key, g_theme);
//                            act->setIcon(firstExistingIcon(c));
//                        }
//                    }
//                }
//            }
//            if (!key.isEmpty()) {
//                const auto c = iconCandidates(key, g_theme);
//                lab->setPixmap(firstExistingPixmap(c));
//            }
//        }

        if (auto* lab = qobject_cast<QLabel*>(w)) {
            const QVariant key = lab->property("pixmap_key");
            if (key.isValid()) {
                const auto c = iconCandidates(key.toString(), g_theme);
                lab->setPixmap(firstExistingPixmap(c));
            }
        }

        // 3) QAction（如果你的侧栏用的是 QAction 触发）
        const auto actions = w->findChildren<QAction*>();
        for (QAction* act : actions) {
            const QString key = iconKeyFor(act);
            if (!key.isEmpty()) {
                const auto c = iconCandidates(key, g_theme);
                act->setIcon(firstExistingIcon(c));
            }
        }
    }
}

