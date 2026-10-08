#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QString>
#include <QMap>
#include <QList>
#include <QColor>
#include <QJsonObject>

struct MouseProfile {
    QString name;
    QMap<int, QString> buttonBindings; // buttonId -> key/macro
    int dpi = 1600;
    int pollingRate = 1000;
    int rgbEffect = 0;
    QMap<int, QColor> zoneColors; // 0=body, 1=lmb, 2=rmb, 3=wheel, 4=underglow

    // Aim Trainer records per game mode
    QMap<QString, int> highScores;       // mode -> max hits
    QMap<QString, double> bestAccuracy;  // mode -> max %
    QMap<QString, int> bestStreak;       // mode -> max streak
    QMap<QString, int> bestReactionMs;   // mode -> lowest ms

    QJsonObject toJson() const;
    static MouseProfile fromJson(const QJsonObject &obj);
};

class ProfileManager {
public:
    static ProfileManager& instance();

    void load();
    void save();

    QStringList profileNames() const;
    QString activeProfileName() const { return currentProfileName; }
    MouseProfile& activeProfile();
    const MouseProfile& activeProfile() const;

    bool setActiveProfile(const QString &name);
    bool createProfile(const QString &name, const MouseProfile &copyFrom = MouseProfile());
    bool deleteProfile(const QString &name);
    bool renameProfile(const QString &oldName, const QString &newName);

    // Global settings
    bool soundEnabled = true;
    bool heatmapTrail = true;
    int heatmapDisplayMode = 2; // Combined

private:
    ProfileManager();
    QString configFilePath() const;
    void ensureDefaultProfiles();

    QMap<QString, MouseProfile> profiles;
    QString currentProfileName = "Default";
};

#endif // PROFILEMANAGER_H
