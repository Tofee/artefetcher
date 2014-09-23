#include <catalogs/icatalog.h>

void ICatalog::addMetadataIfNotEmpty(FilmDetails *film, QVariantMap inputMap, QString fieldName, MetaType internalFieldName, bool isDate) {
    if (!inputMap.value(fieldName).isValid())
        return;
    QString value = inputMap.value(fieldName).toString();
    if (!isDate) {
        film->m_metadata.insert(internalFieldName, value);
    }
    else {
        // convert "25/10/2013 08:30:09 +0200" into "25/10/2013 08:30:09"
        film->m_metadata.insert(internalFieldName, value.left(19));
    }
}
