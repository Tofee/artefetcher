#include "rtmpthread.h"

#include <librtmp/rtmp.h>
#include <QtGui>

//#define DEBUG_FAKE_DOWNLOAD

RTMPThread::RTMPThread(const QMap<int, FilmDetails> details, QObject *parent) :
    QThread(parent), m_details(details)
{
}

void RTMPThread::run()
{
    QMap<int, FilmDetails>::const_iterator it;
    for (it  = m_details.constBegin(); it != m_details.constEnd(); ++it)

    {
        const FilmDetails& detail = it.value();
        int filmId = it.key();

        for (QMap<StreamType, Stream>::const_iterator it = detail.m_streamsByType.constBegin();
             it != detail.m_streamsByType.constEnd(); ++it)
        {
            const Stream filmStream = it.value();

            if (! filmStream.use)
                continue;

            emit(updateProgress(filmId, it.key(), 0, 0));

#ifdef DEBUG_FAKE_DOWNLOAD
            sleep(10);
#else

            RTMP* session = RTMP_Alloc();
            RTMP_Init(session);
            std::string stdStringUrl = QString("%1 swfUrl=%2 swfVfy=1")
                    .arg(filmStream.m_rtmpStreamUrl, detail.m_flashPlayerUrl).toStdString();
            char *charUrl = new char[stdStringUrl.size()+1] ;
            strcpy(charUrl, stdStringUrl.c_str());


            int result = RTMP_SetupURL(session, charUrl);

            if (! result)
            {
                continue;
            }

            result = RTMP_Connect(session, NULL);

            if (! result)
            {
                continue;
            }
            result = RTMP_ConnectStream(session, 0);


            if (! result)
            {
                // TODO faire mieux que des continue!!
                continue;
            }

            static const int buffersize = 1024;
            char buffer[buffersize];
            QFile device(filmStream.m_targetFileName);

            device.open(QIODevice::WriteOnly);

            int size, totalTransferredSize = 0;

            // Duration is not available as long as the header of the flash is not read.
            uint32_t duration = 0;

            int lastPercentageEmit(0);

            QTime lastShownSpeedTime(QTime::currentTime());
            int lastTransferredSize(0);

            while ((size = RTMP_Read(session, &buffer[0],  buffersize)))
            {
                QByteArray array = QByteArray::fromRawData(buffer, size);
                device.write(array);

                totalTransferredSize+=size;

                if (duration == 0)
                {
                    duration = RTMP_GetDuration(session);
                }

                if (duration != 0)
                {
                    int currentPercentage = ((double) session->m_read.timestamp) / (duration * 1000.0) * 100.0;
                    currentPercentage = ((double) (int) (currentPercentage * 10.0)) / 10.0;

                    if (currentPercentage > lastPercentageEmit)
                    {
                        int speed = 1000 * (totalTransferredSize - lastTransferredSize)
                                    / (lastShownSpeedTime.msecsTo(QTime::currentTime()) * 1024);
                        emit(updateProgress(filmId, it.key(), currentPercentage, speed));
                        lastPercentageEmit = currentPercentage;
                        lastShownSpeedTime = QTime::currentTime();
                        lastTransferredSize = totalTransferredSize;
                    }
                }
            }

            RTMP_Close(session);
            device.close();


            RTMP_Free(session);
            delete charUrl;/**/
#endif // DEBUG_FAKE_DOWNLOAD

            emit(filmDownloaded(filmId, it.key()));
        }

    }
}

