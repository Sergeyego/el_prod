#include "formreport.h"
#include "ui_formreport.h"

FormReport::FormReport(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormReport)
{
    ui->setupUi(this);

    ui->dateEditBeg->setDate(QDate::currentDate().addDays(-QDate::currentDate().day()+1));
    ui->dateEditEnd->setDate(QDate::currentDate());

    progressDialog = new ProgressReportDialog(this);

    model = new RestRoTableModel(this);

    ui->tableView->setModel(model);

    connect(ui->dateEditBeg,SIGNAL(dateChanged(QDate)),this,SLOT(updPath()));
    connect(ui->dateEditEnd,SIGNAL(dateChanged(QDate)),this,SLOT(updPath()));
    connect(ui->radioButtonMark,SIGNAL(clicked(bool)),this,SLOT(updPath()));
    connect(ui->radioButtonPart,SIGNAL(clicked(bool)),this,SLOT(updPath()));

    connect(ui->pushButtonUpd,SIGNAL(clicked(bool)),model,SLOT(select()));

    connect(ui->pushButtonSave,SIGNAL(clicked(bool)),this,SLOT(save()));

    connect(model,SIGNAL(sigStartRefresh()),progressDialog,SLOT(show()));
    connect(model,SIGNAL(sigRefresh()),progressDialog,SLOT(hide()));

    updPath();
}

FormReport::~FormReport()
{
    delete ui;
}

void FormReport::updPath()
{
    QUrlQuery query;
    query.addQueryItem("by_part",ui->radioButtonPart->isChecked() ? "true" : "false");
    model->setPath("api/elrtr/report/"+ui->dateEditBeg->date().toString("yyyy-MM-dd")+"/"+ui->dateEditEnd->date().toString("yyyy-MM-dd")+"?"+query.toString());
}

void FormReport::save()
{
    //QString tit=tr("Отчет цеха электродов с ")+ui->dateEditBeg->date().toString("dd.MM.yyyy")+tr(" по ")+ui->dateEditEnd->date().toString("dd.MM.yyyy")+".xlsx";
    //ui->tableView->save(tit,2,true,Qt::LandscapeOrientation);
}
