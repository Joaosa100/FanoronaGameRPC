#include "boardwire.h"

#include <QDataStream>
#include <QIODevice>

namespace {

void writeSteps(QDataStream &out, const std::vector<MoveStepDTO> &steps)
{
    out << quint32(steps.size());
    for (const auto &step : steps) {
        out << qint32(step.from) << qint32(step.to) << quint8(static_cast<quint8>(step.captureType));
        out << quint32(step.captured.size());
        for (int c : step.captured)
            out << qint32(c);
    }
}

std::vector<MoveStepDTO> readSteps(QDataStream &in)
{
    std::vector<MoveStepDTO> steps;
    quint32 count = 0;
    in >> count;
    steps.reserve(count);
    for (quint32 i = 0; i < count; ++i) {
        MoveStepDTO step;
        qint32 from = 0;
        qint32 to = 0;
        quint8 captureType = 0;
        in >> from >> to >> captureType;
        step.from = from;
        step.to = to;
        step.captureType = static_cast<CaptureType>(captureType);

        quint32 capturedCount = 0;
        in >> capturedCount;
        step.captured.reserve(capturedCount);
        for (quint32 j = 0; j < capturedCount; ++j) {
            qint32 c = 0;
            in >> c;
            step.captured.push_back(c);
        }
        steps.push_back(step);
    }
    return steps;
}

} // namespace

QByteArray encodeHello(const QString &playerName)
{
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out << playerName;
    return payload;
}

QByteArray encodeStartGame(bool hostPlaysFirst)
{
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out << hostPlaysFirst;
    return payload;
}

QByteArray encodeMove(const std::vector<MoveStepDTO> &steps)
{
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    writeSteps(out, steps);
    return payload;
}

QByteArray encodeChat(const QString &text)
{
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out << text;
    return payload;
}

QString decodeHello(const QByteArray &payload)
{
    QDataStream in(payload);
    QString name;
    in >> name;
    return name;
}

bool decodeStartGame(const QByteArray &payload)
{
    QDataStream in(payload);
    bool hostFirst = false;
    in >> hostFirst;
    return hostFirst;
}

std::vector<MoveStepDTO> decodeMove(const QByteArray &payload)
{
    QDataStream in(payload);
    return readSteps(in);
}

QString decodeChat(const QByteArray &payload)
{
    QDataStream in(payload);
    QString text;
    in >> text;
    return text;
}
