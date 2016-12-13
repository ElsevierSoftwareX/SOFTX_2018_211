/**
 * Copyright (C) 2016 - Marcos Cardinot
 * @author Marcos Cardinot <mcardinot@gmail.com>
 */

#ifndef WIDGETPROJECT_H
#define WIDGETPROJECT_H

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHash>
#include <QLineEdit>
#include <QSpinBox>
#include <QStringList>
#include <QWidget>

#include "core/project.h"
#include "gui/contextmenutable.h"
#include "gui/experimentview.h"

namespace Ui {
class WidgetProject;
}

class WidgetProject : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetProject(MainApp* mainApp, Project* project, QWidget *parent = 0);
    ~WidgetProject();

private slots:
    int slotAdd();
    void slotAddRun();
    void slotImport();
    void slotRunAll();
    void slotRunSelected();
    void slotStatusChanged(int experimentId, int processId, int newStatus);
    void slotOpenView(int experimentId);
    void slotContextMenu(QPoint point);
    void slotAgentFile();

private:
    Ui::WidgetProject* m_ui;
    MainApp* m_mainApp;
    Project* m_project;
    ContextMenuTable* m_contextMenu;

    QHash<int, ExperimentView*> m_views;
    QHash<QString, int> m_tableHeader; // map column name to column index

    // general properties
    QComboBox* m_treeGraphType;
    QLineEdit* m_treeAgents;
    QSpinBox* m_treeSeed;
    QSpinBox* m_treeStopAt;
    // particular properties
    QHash<QString, QDoubleSpinBox*> m_treeModelParams;

    // get all parameters from the treeWidget itens
    QVariantHash getGeneralParams();
    QVariantHash getModelParams();

    // insert a new experiment to the table
    void insertRow(Simulation *sim, QVariantHash generalParams, QVariantHash modelParams);
};

#endif // WIDGETPROJECT_H
