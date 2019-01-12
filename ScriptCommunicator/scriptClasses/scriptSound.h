#ifndef SCRIPTSOUND_H
#define SCRIPTSOUND_H

#include <QSound>
#include "scriptObject.h"

///This wrapper class is used to access a QSound object from a script.
class ScriptSound : public QObject, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)
public:
    explicit ScriptSound(QObject *parent, QString filename) : QObject(parent), m_sound(filename)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)

    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptSound.api");
    }

    ///Returns the filename associated with this script sound object.
    Q_INVOKABLE QString fileName(void){return m_sound.fileName();}

    ///Returns true if the sound has finished playing; otherwise returns false.
    Q_INVOKABLE bool isFinished(void){return m_sound.isFinished();}

    ///Returns the number of times the sound will play. Return value of QSound::Infinite (-1) indicates infinite number of loops.
    Q_INVOKABLE int loops(void){return m_sound.loops();}

    ///Returns the remaining number of times the sound will loop (for all positive values this value decreases each time the sound is played).
    ///Return value of QSound::Infinite (-1) indicates infinite number of loops.
    Q_INVOKABLE int loopsRemaining(void){return m_sound.loopsRemaining();}

    ///Starts playing the sound specified by this QSound object.
    Q_INVOKABLE void play(void){m_sound.play();}

    ///Sets the sound to repeat the given number of times when it is played.
    Q_INVOKABLE void setLoops(int number){m_sound.setLoops(number);}

    ///Stops the sound playing.
    Q_INVOKABLE void stop(void){m_sound.stop();}


signals:


private:

    ///The wrapped QSound object.
    QSound m_sound;

};
#endif // SCRIPTSOUND_H

