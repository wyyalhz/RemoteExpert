#pragma once
#include <QString>

enum class Theme { Light, Dark };

bool applyTheme(Theme t);
void refreshThemedAssets();
QString currentThemeName();
