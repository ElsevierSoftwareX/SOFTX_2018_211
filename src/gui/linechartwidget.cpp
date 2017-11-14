/**
 * Copyright (C) 2016 - Marcos Cardinot
 * @author Marcos Cardinot <mcardinot@gmail.com>
 */

#include <QMessageBox>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>

#include "linechartwidget.h"
#include "titlebar.h"
#include "core/utils.h"

namespace evoplex {

LineChartWidget::LineChartWidget(Experiment* exp, QWidget* parent)
    : QDockWidget(parent)
    , m_settingsDlg(new Ui_LineChartSettings)
    , m_exp(exp)
    , m_chart(new QtCharts::QChart())
    , m_maxY(0)
    , m_finished(false)
{
    setWindowTitle("Line Chart");
    setAttribute(Qt::WA_DeleteOnClose, true);

    TitleBar* titleBar = new TitleBar(exp, this);
    setTitleBarWidget(titleBar);
    connect(titleBar, SIGNAL(trialSelected(int)), this, SLOT(setSelectedTrial(int)));
    setSelectedTrial(0);

    QPalette pal = palette();
    pal.setColor(QPalette::Background, QColor(239,235,231));
    setAutoFillBackground(true);
    setPalette(pal);

    QDialog* dlg = new QDialog(this);
    m_settingsDlg->setupUi(dlg);
    connect(titleBar, SIGNAL(openSettingsDlg()), dlg, SLOT(show()));
    dlg->hide();

    for (QString funcName : DefaultOutput::availableFunctions()) {
        m_settingsDlg->func->addItem(funcName, 0); // default function
    }
    for (QString funcName : m_exp->modelPlugin()->customOutputs) {
        m_settingsDlg->func->addItem(funcName, 1); // custom function
    }
    connect(m_settingsDlg->add, SIGNAL(clicked(bool)), SLOT(slotAddSeries()));
    connect(m_settingsDlg->func, SIGNAL(currentIndexChanged(int)), SLOT(slotFuncChanged(int)));
    connect(m_settingsDlg->entityAgent, SIGNAL(toggled(bool)), SLOT(slotEntityChanged(bool)));
    m_settingsDlg->entityAgent->setChecked(true);

    m_chart->legend()->hide();
    QtCharts::QChartView *chartView = new QtCharts::QChartView(m_chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    setWidget(chartView);
}

LineChartWidget::~LineChartWidget()
{
}

void LineChartWidget::setSelectedTrial(int trialId)
{
}

void LineChartWidget::slotFuncChanged(int idx)
{
    m_settingsDlg->input->clear();
    m_settingsDlg->attr->setEnabled(m_settingsDlg->func->itemData(idx) == 0);
    m_settingsDlg->input->setEnabled(m_settingsDlg->func->itemData(idx) == 0);
}

void LineChartWidget::slotEntityChanged(bool isAgent)
{
    m_settingsDlg->attr->clear();
    m_settingsDlg->input->clear();
    if (isAgent) {
        for (QString n : m_exp->modelPlugin()->agentAttrMin.names()) {
            m_settingsDlg->attr->addItem(n);
        }
    } else {
        for (QString n : m_exp->modelPlugin()->edgeAttrMin.names()) {
            m_settingsDlg->attr->addItem(n);
        }
    }
    m_settingsDlg->input->setEnabled(m_settingsDlg->attr->count() > 0);
}

void LineChartWidget::slotAddSeries()
{
    int row = m_settingsDlg->table->rowCount();
    QString attrName = m_settingsDlg->attr->currentText();

    Output* output;
    Value input;
    if (m_settingsDlg->func->currentData().toInt() == 0) { // default output
        DefaultOutput::Entity entity;
        if (m_settingsDlg->entityAgent->isChecked()) {
            entity = DefaultOutput::E_Agents;
            input = Utils::validateParameter(m_exp->modelPlugin()->agentAttrSpace.value(attrName).second,
                                             m_settingsDlg->input->text());
        } else {
            entity = DefaultOutput::E_Edges;
            input = Utils::validateParameter(m_exp->modelPlugin()->edgeAttrSpace.value(attrName).second,
                                             m_settingsDlg->input->text());
        }

        if (!input.isValid()) {
            QMessageBox::warning(this, "Wrong Input", "The 'input' is not valid for the current 'attribute'.");
            return;
        }

        DefaultOutput::Function func = DefaultOutput::funcFromString(m_settingsDlg->func->currentText());
        Q_ASSERT(func != DefaultOutput::F_Invalid);

        output = new DefaultOutput(func, entity, attrName, m_settingsDlg->attr->currentIndex(), {input});

        m_settingsDlg->table->insertRow(row);
        m_settingsDlg->table->setItem(row, 0, new QTableWidgetItem("Default"));
        m_settingsDlg->table->setItem(row, 1, new QTableWidgetItem(m_settingsDlg->func->currentText()));
        m_settingsDlg->table->setItem(row, 2, new QTableWidgetItem(entity == DefaultOutput::E_Agents ? "Agent" : "Edge"));
        m_settingsDlg->table->setItem(row, 3, new QTableWidgetItem(m_settingsDlg->attr->currentText()));
        m_settingsDlg->table->setItem(row, 4, new QTableWidgetItem(m_settingsDlg->input->text()));

    } else { // custom output
        input = Value(m_settingsDlg->func->currentText());
        output = new CustomOutput({input});

        m_settingsDlg->table->insertRow(row);
        m_settingsDlg->table->setItem(row, 0, new QTableWidgetItem("Custom"));
        m_settingsDlg->table->setItem(row, 1, new QTableWidgetItem(m_settingsDlg->func->currentText()));
    }

    Series s;
    s.series = new QtCharts::QLineSeries();
    s.series->setUseOpenGL(true);
    Output* existingOutput = m_exp->searchOutput(output);
    if (existingOutput) {
        delete output;
        s.cacheIdx = existingOutput->addCache({input});
        s.output = existingOutput;
    } else {
        s.cacheIdx = 0;
        s.output = output;
        m_exp->addOutput(output);
    }
    m_series.emplace_back(s);

    m_chart->addSeries(m_series.back().series);
}

void LineChartWidget::updateSeries()
{
    if (m_series.empty() || m_finished || !this->isVisible()) {
        return;
    }

    float maxY = m_maxY;
    for (Series& s : m_series) {
        QVector<QPointF> points = s.series->pointsVector();
        int x = points.size() + s.rowsSkipped;
        float y = 0.f;

        // read only the top 10k (max) lines to avoid blocking the UI
        bool lastWasDuplicated = false;
        for (int i = 0; i < 10000 && !s.output->isEmpty(s.cacheIdx); ++i) {
            Values row = s.output->readFrontRow(s.cacheIdx);
            Q_ASSERT(row.size() == 1);
            s.output->flushFrontRow(s.cacheIdx);

            if (row.at(0).type == Value::INT) {
                y = row.at(0).toInt;
            } else if (row.at(0).type == Value::DOUBLE) {
                y = row.at(0).toDouble;
            } else {
                qFatal("[LineChart] : the type is invalid!");
            }

            // we skip the duplicated rows to reduce the amount of unnecessary points
            if (!points.isEmpty()) {
                bool currIsDuplicated = qFuzzyCompare(y, (float) points.last().y());
                if (lastWasDuplicated && currIsDuplicated) {
                    points.last().setX(x++);
                    ++s.rowsSkipped;
                    lastWasDuplicated = currIsDuplicated;
                    continue;
                }
                lastWasDuplicated = currIsDuplicated;
            }

            points.push_back(QPointF(x++, y));
            if (y > maxY) maxY = y;
        }

        if (lastWasDuplicated) {
            points.push_back(QPointF(x, y));
        }

        s.series->replace(points);
    }

    if (maxY != m_maxY) {
        m_maxY = maxY;
        m_chart->createDefaultAxes();
        m_chart->axisX()->setRange(0, m_exp->stopAt());
        m_chart->axisY()->setRange(0, m_maxY + 1);
    }

    m_finished = m_series.front().output->isEmpty(m_series.front().cacheIdx)
                    && m_exp->expStatus() == Experiment::FINISHED;
}

}
