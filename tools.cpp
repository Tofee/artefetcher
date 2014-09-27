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

    if (Preferences::getInstance()->useDedicatedDirectoryForSeries() && !episodeName.isEmpty())
    {
        QRegExp episodeNumberRegExp("\\([0-9 \\-/]+\\)");

        QString serieName = QString(title).replace(episodeNumberRegExp, "");

        /* Get the episode suffix, like "(12/15)" to put it as prefix of the film title */
        QString episodeNumberText;
        if (episodeNumberRegExp.indexIn(title) >= 0)
        {
            episodeNumberText = episodeNumberRegExp.cap().append(" ");
        }

        QString filename = Preferences::getInstance()->filenamePattern();;
        filename.replace("%title", episodeNumberText.append(episodeName))
                .replace("%language", film->m_choosenStreamType)
                .replace("%quality", qualityUpperCase);

        cleanedTitle = cleanFilenameForFileSystem(serieName).append(QDir::separator()).append(cleanFilenameForFileSystem(filename));
    } else if (film->m_episodeNumber){
        QString filename = Preferences::getInstance()->filenamePattern();;
        filename.replace("%title", QString("%0 (%1)").arg(title).arg(film->m_episodeNumber))
                .replace("%language", film->m_choosenStreamType)
                .replace("%quality", qualityUpperCase);

        cleanedTitle = cleanFilenameForFileSystem(filename);
    }
    else
    {
        cleanedTitle = Preferences::getInstance()->filenamePattern();
        cleanedTitle.replace("%title", title)
                .replace("%language",  film->m_choosenStreamType)
                .replace("%quality", qualityUpperCase);
        cleanedTitle = cleanFilenameForFileSystem(cleanedTitle);
    }

    QString filename("%1%2%3.%5");
    filename = filename.arg(targetDirectory,
                            QDir::separator(),
                            cleanedTitle,
                            extension);
    return filename;
}
