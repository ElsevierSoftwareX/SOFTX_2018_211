/**
 * Copyright (C) 2017 - Marcos Cardinot
 * @author Marcos Cardinot <mcardinot@gmail.com>
 */

#include <QFileDialog>
#include <QMessageBox>

#include "ui_pluginswidget.h"
#include "pluginswidget.h"

namespace evoplex {

PluginsWidget::PluginsWidget(MainApp* mainApp, QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui_PluginsWidget)
    , m_mainApp(mainApp)
{
    m_ui->setupUi(this);

    m_ui->table->horizontalHeader()->setSectionResizeMode(UID, QHeaderView::ResizeToContents);
    m_ui->table->horizontalHeader()->setSectionResizeMode(TYPE, QHeaderView::ResizeToContents);

    connect(m_ui->bImport, SIGNAL(pressed()), SLOT(importPlugin()));
    connect(m_ui->table, SIGNAL(itemSelectionChanged()), SLOT(rowSelectionChanged()));

    for (GraphPlugin* g : m_mainApp->getGraphs()) {
        insertRow(g);
    }
    for (ModelPlugin* m : m_mainApp->getModels()) {
        insertRow(m);
    }
}

PluginsWidget::~PluginsWidget()
{
    delete m_ui;
}

void PluginsWidget::rowSelectionChanged()
{
    int row = m_ui->table->currentRow();
    int type = m_ui->table->item(row, TYPE)->data(Qt::UserRole).toInt();
    if (type == AbstractPlugin::GraphPlugin) {
        loadHtml(m_mainApp->getGraph(m_ui->table->item(row, UID)->text()));
    } else if (type == AbstractPlugin::ModelPlugin) {
        loadHtml(m_mainApp->getModel(m_ui->table->item(row, UID)->text()));
    } else {
        qFatal("[PluginsWidget]: invalid plugin type! It should never happen.");
    }
}

void PluginsWidget::loadHtml(const GraphPlugin* plugin)
{
    if (!plugin) {
        qFatal("[PluginsWidget]: invalid plugin object! It should never happen.");
    }

    QString html = "<h2>" + plugin->name() + "</h2><br>"
                 + "<b>Author:</b> " + plugin->author() + "<br>"
                 + "<b>Plugin Type:</b> Graph<br>"
                 + "<b>Description:</b> " + plugin->description() + "<br>";

    m_ui->browser->setHtml(html);
}

void PluginsWidget::loadHtml(const ModelPlugin* plugin)
{
    if (!plugin) {
        qFatal("[PluginsWidget]: invalid plugin object! It should never happen.");
    }

    QString html = "<h2>" + plugin->name() + "</h2><br>"
                 + "<b>Author:</b> " + plugin->author() + "<br>"
                 + "<b>Plugin Type:</b> Model<br>"
                 + "<b>Description:</b> " + plugin->description() + "<br>";

    m_ui->browser->setHtml(html);
}

void PluginsWidget::importPlugin()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Plugin"),
                                                    QDir::homePath(),
                                                    tr("Plugin (*.so)"));
    if (fileName.isEmpty()) {
        return;
    }

    QString error;
    QString uid = m_mainApp->loadPlugin(fileName, error);

    if (!error.isEmpty()) {
        QMessageBox::warning(this, "Error", error);
        return;
    }

    const AbstractPlugin* plugin = m_mainApp->getModel(uid);
    if (plugin) {
        plugin = m_mainApp->getGraph(uid);
    }
    insertRow(plugin);
}

void PluginsWidget::insertRow(const AbstractPlugin* plugin)
{
    QTableWidgetItem* typeItem;
    if (plugin->type() == AbstractPlugin::ModelPlugin) {
        typeItem = new QTableWidgetItem("model");
    } else if (plugin->type() == AbstractPlugin::GraphPlugin) {
        typeItem = new QTableWidgetItem("graph");
    } else {
        qFatal("[PluginsWidget]: invalid plugin type! It should never happen.");
    }
    typeItem->setData(Qt::UserRole, plugin->type());

    int row = m_ui->table->rowCount();
    m_ui->table->insertRow(row);
    m_ui->table->setItem(row, UID, new QTableWidgetItem(plugin->id()));
    m_ui->table->setItem(row, NAME, new QTableWidgetItem(plugin->name()));
    m_ui->table->setItem(row, TYPE, typeItem);
}

}
