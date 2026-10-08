#include "ProfileManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

QJsonObject MouseProfile::toJson() const {
    QJsonObject obj;
    obj["name"] = name;
    obj["dpi"] = dpi;
    obj["pollingRate"] = pollingRate;
    obj["rgbEffect"] = rgbEffect;

    // Bindings
    QJsonObject bindObj;
    for (auto it = buttonBindings.begin(); it != buttonBindings.end(); ++it) {
        bindObj[QString::number(it.key())] = it.value();
    }
    obj["bindings"] = bindObj;

    // Zone colors
    QJsonObject zonesObj;
    for (auto it = zoneColors.begin(); it != zoneColors.end(); ++it) {
        zonesObj[QString::number(it.key())] = it.value().name();
    }
    obj["zoneColors"] = zonesObj;

    // Records
    QJsonObject scoreObj;
    for (auto it = highScores.begin(); it != highScores.end(); ++it) scoreObj[it.key()] = it.value();
    obj["highScores"] = scoreObj;

    QJsonObject accObj;
    for (auto it = bestAccuracy.begin(); it != bestAccuracy.end(); ++it) accObj[it.key()] = it.value();
    obj["bestAccuracy"] = accObj;

    QJsonObject streakObj;
    for (auto it = bestStreak.begin(); it != bestStreak.end(); ++it) streakObj[it.key()] = it.value();
    obj["bestStreak"] = streakObj;

    QJsonObject reactObj;
    for (auto it = bestReactionMs.begin(); it != bestReactionMs.end(); ++it) reactObj[it.key()] = it.value();
    obj["bestReactionMs"] = reactObj;

    return obj;
}

MouseProfile MouseProfile::fromJson(const QJsonObject &obj) {
    MouseProfile p;
    p.name = obj.value("name").toString("Default");
    p.dpi = obj.value("dpi").toInt(1600);
    p.pollingRate = obj.value("pollingRate").toInt(1000);
    p.rgbEffect = obj.value("rgbEffect").toInt(0);

    QJsonObject bindObj = obj.value("bindings").toObject();
    for (auto it = bindObj.begin(); it != bindObj.end(); ++it) {
        p.buttonBindings[it.key().toInt()] = it.value().toString();
    }

    QJsonObject zonesObj = obj.value("zoneColors").toObject();
    for (auto it = zonesObj.begin(); it != zonesObj.end(); ++it) {
        p.zoneColors[it.key().toInt()] = QColor(it.value().toString());
    }

    QJsonObject scoreObj = obj.value("highScores").toObject();
    for (auto it = scoreObj.begin(); it != scoreObj.end(); ++it) p.highScores[it.key()] = it.value().toInt();

    QJsonObject accObj = obj.value("bestAccuracy").toObject();
    for (auto it = accObj.begin(); it != accObj.end(); ++it) p.bestAccuracy[it.key()] = it.value().toDouble();

    QJsonObject streakObj = obj.value("bestStreak").toObject();
    for (auto it = streakObj.begin(); it != streakObj.end(); ++it) p.bestStreak[it.key()] = it.value().toInt();

    QJsonObject reactObj = obj.value("bestReactionMs").toObject();
    for (auto it = reactObj.begin(); it != reactObj.end(); ++it) p.bestReactionMs[it.key()] = it.value().toInt();

    return p;
}

ProfileManager& ProfileManager::instance() {
    static ProfileManager mgr;
    return mgr;
}

ProfileManager::ProfileManager() {
    ensureDefaultProfiles();
    load();
}

QString ProfileManager::configFilePath() const {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (configDir.isEmpty()) {
        configDir = QDir::homePath() + "/.config/mousy";
    }
    QDir().mkpath(configDir);
    return configDir + "/profiles.json";
}

void ProfileManager::ensureDefaultProfiles() {
    if (profiles.contains("Default")) return;

    MouseProfile def;
    def.name = "Default";
    def.dpi = 1600;
    def.pollingRate = 1000;
    def.rgbEffect = 0;
    profiles["Default"] = def;

    MouseProfile fps;
    fps.name = "FPS Gaming";
    fps.dpi = 800;
    fps.pollingRate = 1000;
    fps.rgbEffect = 4; // Crimson Fire
    fps.buttonBindings[4] = "Melee (V)";
    fps.buttonBindings[5] = "Grenade (G)";
    profiles["FPS Gaming"] = fps;

    MouseProfile prod;
    prod.name = "Productivity";
    prod.dpi = 1200;
    prod.pollingRate = 500;
    prod.rgbEffect = 3; // Matrix Green
    prod.buttonBindings[4] = "Ctrl+C";
    prod.buttonBindings[5] = "Ctrl+V";
    profiles["Productivity"] = prod;
}

void ProfileManager::load() {
    QFile file(configFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;

    QJsonObject root = doc.object();
    currentProfileName = root.value("activeProfile").toString("Default");
    soundEnabled = root.value("soundEnabled").toBool(true);
    heatmapTrail = root.value("heatmapTrail").toBool(true);
    heatmapDisplayMode = root.value("heatmapDisplayMode").toInt(2);

    QJsonArray profArr = root.value("profiles").toArray();
    for (const auto &val : profArr) {
        if (val.isObject()) {
            MouseProfile p = MouseProfile::fromJson(val.toObject());
            profiles[p.name] = p;
        }
    }

    if (!profiles.contains(currentProfileName)) {
        currentProfileName = profiles.isEmpty() ? "Default" : profiles.firstKey();
    }
}

void ProfileManager::save() {
    QJsonObject root;
    root["activeProfile"] = currentProfileName;
    root["soundEnabled"] = soundEnabled;
    root["heatmapTrail"] = heatmapTrail;
    root["heatmapDisplayMode"] = heatmapDisplayMode;

    QJsonArray profArr;
    for (const auto &p : profiles) {
        profArr.append(p.toJson());
    }
    root["profiles"] = profArr;

    QJsonDocument doc(root);
    QFile file(configFilePath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

QStringList ProfileManager::profileNames() const {
    return profiles.keys();
}

MouseProfile& ProfileManager::activeProfile() {
    if (!profiles.contains(currentProfileName)) {
        ensureDefaultProfiles();
        currentProfileName = "Default";
    }
    return profiles[currentProfileName];
}

const MouseProfile& ProfileManager::activeProfile() const {
    auto it = profiles.find(currentProfileName);
    if (it != profiles.end()) {
        return it.value();
    }
    auto defIt = profiles.find("Default");
    if (defIt != profiles.end()) {
        return defIt.value();
    }
    static const MouseProfile fallback;
    return fallback;
}

bool ProfileManager::setActiveProfile(const QString &name) {
    if (!profiles.contains(name)) return false;
    currentProfileName = name;
    save();
    return true;
}

bool ProfileManager::createProfile(const QString &name, const MouseProfile &copyFrom) {
    if (name.trimmed().isEmpty() || profiles.contains(name)) return false;
    MouseProfile p = copyFrom;
    p.name = name;
    profiles[name] = p;
    currentProfileName = name;
    save();
    return true;
}

bool ProfileManager::deleteProfile(const QString &name) {
    if (profiles.size() <= 1 || !profiles.contains(name)) return false;
    profiles.remove(name);
    if (currentProfileName == name) {
        currentProfileName = profiles.firstKey();
    }
    save();
    return true;
}

bool ProfileManager::renameProfile(const QString &oldName, const QString &newName) {
    if (!profiles.contains(oldName) || profiles.contains(newName) || newName.trimmed().isEmpty()) return false;
    MouseProfile p = profiles.take(oldName);
    p.name = newName;
    profiles[newName] = p;
    if (currentProfileName == oldName) currentProfileName = newName;
    save();
    return true;
}
