#ifndef TOOLS_H
#define TOOLS_H
#include <QString>
#include "film/streamtype.h"

class FilmDetails;

QString cleanFilenameForFileSystem(const QString filename);

/**
 * @brief getFileName
 * @param film to get the filename
 * @return the filename
 */
QString getFileName(const FilmDetails * const film);

#endif // TOOLS_H
