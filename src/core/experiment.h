/**
 *  This file is part of Evoplex.
 *
 *  Evoplex is a multi-agent system for networks.
 *  Copyright (C) 2016 - Marcos Cardinot <marcos@cardinot.net>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <QMutex>

#include "expinputs.h"
#include "experimentsmgr.h"
#include "mainapp.h"
#include "constants.h"
#include "output.h"
#include "trial.h"
#include "graphplugin.h"
#include "modelplugin.h"

namespace evoplex {

/**
 * Evoplex assumes that any experiment belongs to a valid project.
 */
class Experiment : public QObject
{
    Q_OBJECT

    friend class ExperimentsMgr;
    friend class Trial;

public:
    enum Status {
        INVALID,  // something went wrong
        UNSET,    // has not yet been initialized
        READY,    // ready for another step
        QUEUED,   // queued to run
        RUNNING,  // running in a work thread
        FINISHED, // all is done
    };
    Q_ENUM(Status) // important to make the signals work

    explicit Experiment(MainApp* mainApp, ExpInputs* inputs, ProjectPtr project);

    ~Experiment();

    bool init(ExpInputs* inputs, QString& error);

    void reset();

    // set experiment status
    // this IS thread-safe
    void setExpStatus(Status s);

    // play if it's paused and pause it's running
    void toggle();

    // run all trials
    void play();
    void playNext();

    // create a set of nodes for the current inputs
    bool createNodes(Nodes& nodes) const;

    bool removeOutput(OutputPtr output);
    OutputPtr searchOutput(const OutputPtr find);
    inline bool hasOutputs() const;
    inline void addOutput(OutputPtr output);

    // pause all trials asap
    inline void pause();
    // pause all trials at a specific step
    inline int pauseAt() const;
    inline void setPauseAt(int step);

    // stop all trials asap
    inline void stop();
    // stop all trials at a specific step
    inline int stopAt() const;
    inline void setStopAt(int step);

    inline quint16 delay() const;
    inline void setDelay(quint16 delay);

    inline bool autoDeleteTrials() const;
    inline void setAutoDeleteTrials(bool b);

    const Trial* trial(quint16 trialId) const;
    inline const Trials& trials();

    inline int id() const;
    inline ProjectPtr project() const;
    inline int numTrials() const;
    inline Status expStatus() const;
    inline quint16 progress() const;
    inline const ExpInputs* inputs() const;
    inline const QString& modelId() const;
    inline const QString& graphId() const;
    inline const ModelPlugin* modelPlugin() const;
    inline const GraphPlugin* graphPlugin() const;

signals:
    void trialCreated(int trialId);
    void restarted();
    void progressUpdated();
    void statusChanged(Experiment::Status);

private slots:
    // Updates the progress value.
    // This method might be expensive!
    void updateProgressValue();

private:
    QMutex m_mutex;
    MainApp* m_mainApp;
    const int m_id;
    ProjectPtr m_project;

    const ExpInputs* m_inputs;
    const GraphPlugin* m_graphPlugin;
    const ModelPlugin* m_modelPlugin;
    int m_numTrials;
    bool m_autoDeleteTrials;
    int m_stopAt;

    QString m_fileHeader;   // file header is the same for all trials; let's save it then
    QString m_filePathPrefix;
    std::unordered_set<OutputPtr> m_outputs;

    int m_pauseAt;
    Status m_expStatus;
    quint16 m_progress; // current progress value [0, 360]
    quint16 m_delay;

    Trials m_trials;

    // The trials are meant to have the same initial population.
    // So, considering that it might be a very expensive operation (eg, I/O),
    // we try to do the heavy stuff only once, storing the initial population
    // in the 'm_clonableNodes' container. Except when the experiment has only
    // one trial.
    Nodes m_clonableNodes;

    // Return a clone of 'm_clonableNodes'. It also clear the 'm_clonableNodes'
    // if 'trialId' is the last trial being created for this experiment.
    // This method is NOT thread-safe.
    Nodes cloneCachedNodes(const int trialId);

    void deleteTrials();
};

/************************************************************************
   Experiment: Inline member functions
 ************************************************************************/

inline bool Experiment::Experiment::hasOutputs() const
{ return !m_outputs.empty(); }

inline void Experiment::addOutput(OutputPtr output)
{ m_outputs.insert(output); }

inline void Experiment::pause()
{ m_pauseAt = 0; }

inline int Experiment::pauseAt() const
{ return m_pauseAt; }

inline void Experiment::setPauseAt(int step)
{ m_pauseAt = step > m_stopAt ? m_stopAt : step; }

inline void Experiment::stop()
{ pause(); m_stopAt = 0; play(); }

inline int Experiment::stopAt() const
{ return m_stopAt; }

inline void Experiment::setStopAt(int step)
{ m_stopAt = step > EVOPLEX_MAX_STEPS ? EVOPLEX_MAX_STEPS : step; }

inline const Trials& Experiment::trials()
{ return m_trials; }

inline quint16 Experiment::delay() const
{ return m_delay; }

inline void Experiment::setDelay(quint16 delay)
{ m_delay = delay; }

inline bool Experiment::autoDeleteTrials() const
{ return m_autoDeleteTrials; }

inline void Experiment::setAutoDeleteTrials(bool b)
{ m_autoDeleteTrials = b; }

inline int Experiment::id() const
{ return m_id; }

inline ProjectPtr Experiment::project() const
{ return m_project; }

inline int Experiment::numTrials() const
{ return m_numTrials; }

inline Experiment::Status Experiment::expStatus() const
{ return m_expStatus; }

inline quint16 Experiment::progress() const
{ return m_progress; }

inline const ExpInputs* Experiment::inputs() const
{ return m_inputs; }

inline const QString& Experiment::modelId() const
{ return m_modelPlugin->id(); }

inline const QString& Experiment::graphId() const
{ return m_graphPlugin->id(); }

inline const ModelPlugin* Experiment::modelPlugin() const
{ return m_modelPlugin; }

inline const GraphPlugin* Experiment::graphPlugin() const
{ return m_graphPlugin; }

} // evoplex

// Lets make the Experiment pointer known to QMetaType
// It enable us to convert an Experiment* to a QVariant for example
Q_DECLARE_METATYPE(evoplex::Experiment*)
Q_DECLARE_METATYPE(const evoplex::Experiment*)

#endif // EXPERIMENT_H
