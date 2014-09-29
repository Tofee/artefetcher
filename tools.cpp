#include "tools.h"
#include <QString>
#include <QDir>
#include "preferences.h"
#include "film/filmdetails.h"

QString cleanFilenameForFileSystem(const QString filename) {
    QString cleanedFilename(filename);
#ifdef Q_OS_WIN32
// AJOUT FreddyP 26/10/2013 : Jeu de caractère latin1 pour windows, c'est la valeur QString par défaut pour les caractères > 127
//                            Donc, pas de transformation, conversion directe des caractères accentués sans fromLocal8Bit

    cleanedFilename.replace(QRegExp("[éèëê]"), "e");
    cleanedFilename.replace(QRegExp("[ÉÈËÊ]"), "E");
    cleanedFilename.replace(QRegExp("[ô]"), "o");
    cleanedFilename.replace(QRegExp("[Ô]"), "O");
    cleanedFilename.replace(QRegExp("[âáà]"), "a");
    cleanedFilename.replace(QRegExp("[ÂÁÀ]"), "A");
    cleanedFilename.replace(QRegExp("[îï]"), "i");
    cleanedFilename.replace(QRegExp("[ÎÏ]"), "I");
    cleanedFilename.replace(QRegExp("[û]"), "u");
    cleanedFilename.replace(QRegExp("[Û]"), "U");
    cleanedFilename.replace(QRegExp("[ç]"), "c");
    cleanedFilename.replace(QRegExp("[Ç]"), "C");
    cleanedFilename.replace(QRegExp("[ß]"), "ss");
    cleanedFilename.replace(QRegExp("[ä]"), "ae");
    cleanedFilename.replace(QRegExp("[Ä]"), "AE");
    cleanedFilename.replace(QRegExp("[ö]"), "oe");
    cleanedFilename.replace(QRegExp("[Ö]"), "OE");
    cleanedFilename.replace(QRegExp("[ü]"), "ue");
    cleanedFilename.replace(QRegExp("[Ü]"), "UE");
#endif

#ifdef Q_OS_LINUX
// On conserve le fromLocal8Bit (obligatoirement APRES la conversion normale) pour environnement Linux
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[éèëê]")), "e");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ÉÈËÊ]")), "E");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ô]")), "o");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[Ô]")), "O");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[âáà]")), "a");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ÂÁÀ]")), "A");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[îï]")), "i");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ÎÏ]")), "I");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[û]")), "u");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[Û]")), "U");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ç]")), "c");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[Ç]")), "C");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ß]")), "ss");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ä]")), "ae");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[Ä]")), "AE");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ö]")), "oe");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[Ö]")), "OE");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ü]")), "ue");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[Ü]")), "UE");
#endif

    // common for win and linux
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[/]")), "-");

// Gestion des caractères interdits dans un nom de fichier
#ifdef Q_OS_WIN32
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[*]")), "-");       // AJOUT FreddyP 26/10/2013 : Windows n'aime pas "*" dans un filename
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[|]")), "-");       // AJOUT FreddyP 26/10/2013 : Windows n'aime pas "|" dans un filename
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[\\]")), "-");      // AJOUT FreddyP 26/10/2013 : Windows n'aime pas "\" dans un filename
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[:]")), "-");       // AJOUT FreddyP 26/10/2013 : Windows n'aime pas ":" dans un filename
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[""]")), "-");      // AJOUT FreddyP 26/10/2013 : Windows n'aime pas """" dans un filename
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[<]")), "-");       // AJOUT FreddyP 26/10/2013 : Windows n'aime pas "<" dans un filename
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[>]")), "-");       // AJOUT FreddyP 26/10/2013 : Windows n'aime pas ">" dans un filename
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[?]")), "-");       // AJOUT FreddyP 26/10/2013 : Windows n'aime pas ":" dans un filename
#endif

    // Remplacement par espace de tous les autres caractères spéciaux
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[^a-zA-Z0-9 _-()]")), " ");

    return cleanedFilename.simplified();
}

QRegExp getRegexpForEpisodeNumber(){
    return QRegExp("\\([0-9 \\-/]+\\)");
}

QString getEpisodeNumberTextFromTitle(const QString& title){
    QRegExp episodeNumberRegExp = getRegexpForEpisodeNumber();
    /* Get the episode suffix, like "(12/15)" to put it as prefix of the film title */
    QString episodeNumberText;
    if (episodeNumberRegExp.indexIn(title) >= 0) {
        episodeNumberText = episodeNumberRegExp.cap();
    }
    return episodeNumberText;
}

QString removeEpisodeNumberFromTitle(const QString& title){
    QRegExp episodeNumberRegExp = getRegexpForEpisodeNumber();
    return QString(title).replace(episodeNumberRegExp, "").trimmed();
}

QString getFileName(const FilmDetails * const film)
{
    QString qualityUpperCase = Preferences::getInstance()->selectedQuality().toUpper();
    //title the title of the film
    const QString& title = film->title();
    //the file name in the remote server (will be used to check the extension)
    const QString extension = "mp4";
    //the name of the episode if the film belongs to a video serie
    const QString& episodeName = film->m_metadata.value(Episode_name);

    const QString& targetDirectory = Preferences::getInstance()->destinationDir();

    QString cleanedTitle;

    ////////////////////////////////////////////////////////////////////////
    /// CASE 1 : The film belongs to a serie and there is one name for the
    /// episode, one for the serie.
    /// Ex: serie "P'tit Quinquin" - episode 3/4 :"L'diable in Perchonne"
    ////////////////////////////////////////////////////////////////////////
    if (!episodeName.isEmpty()) {

        QString serieName = removeEpisodeNumberFromTitle(title);

        QString episodeNumberText = getEpisodeNumberTextFromTitle(title);
        if (episodeNumberText.isEmpty() && film->m_episodeNumber > 0){
            episodeNumberText = QString("(%1)").arg(film->m_episodeNumber);
        }

        QString filename = Preferences::getInstance()->filenamePattern();
        filename.replace("%title", QString("%1 %2").arg(episodeNumberText).arg(episodeName))
                .replace("%language", film->m_choosenStreamType)
                .replace("%quality", qualityUpperCase);
        QString serieNameSeparator;
        if (Preferences::getInstance()->useDedicatedDirectoryForSeries()){
            // Format : <serie_name>/<episode_number><episode_name>
            serieNameSeparator = QDir::separator();
        } else {
            // Format : <serie_name> - <episode_number><episode_name>
            serieNameSeparator = " - ";
        }

        cleanedTitle = cleanFilenameForFileSystem(serieName).append(serieNameSeparator).append(cleanFilenameForFileSystem(filename));
    } else if (film->m_episodeNumber){
        ////////////////////////////////////////////////////////////////////////
        /// CASE 2 : There is no serie name but there is an episode numbering
        /// Ex: 051912-015 28 minutes. Episode 15th of 28 minutes
        ////////////////////////////////////////////////////////////////////////
        QString filename = Preferences::getInstance()->filenamePattern();

        QString titleWithoutEpisodeNumber = removeEpisodeNumberFromTitle(title);
        QString episodeNumber = getEpisodeNumberTextFromTitle(title);
        if (episodeNumber.isEmpty()) {
            episodeNumber = QString("(%1)").arg(film->m_episodeNumber);
        }

        QString titleWithEpisodeNumber = QString(titleWithoutEpisodeNumber).append(" ").append(episodeNumber);

        filename.replace("%title", titleWithEpisodeNumber)
                .replace("%language", film->m_choosenStreamType)
                .replace("%quality", qualityUpperCase);

        if (Preferences::getInstance()->useDedicatedDirectoryForSeries()){
            // Format : <serie_name>/<serie_name><episode_number>
            cleanedTitle = cleanFilenameForFileSystem(titleWithoutEpisodeNumber).append(QDir::separator()).append(cleanFilenameForFileSystem(filename));
        } else {
            // Format : <serie_name><episode_number>
            cleanedTitle = cleanFilenameForFileSystem(filename);
        }
    }
    else
    {
        ////////////////////////////////////////////////////////////////////////
        /// CASE 3 : The film does not belong to an episode
        ////////////////////////////////////////////////////////////////////////
        cleanedTitle = Preferences::getInstance()->filenamePattern();
        cleanedTitle.replace("%title", title)
                .replace("%language",  film->m_choosenStreamType)
                .replace("%quality", qualityUpperCase);
        cleanedTitle = cleanFilenameForFileSystem(cleanedTitle);
    }

    QString filename("%1%2%3.%4");
    filename = filename.arg(targetDirectory,
                            QDir::separator(),
                            cleanedTitle,
                            extension);
    return filename;
}
