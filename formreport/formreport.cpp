#include "formreport.h"
#include "ui_formreport.h"

FormReport::FormReport(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormReport)
{
    ui->setupUi(this);

    ui->dateEditBeg->setDate(QDate::currentDate().addDays(-QDate::currentDate().day()+1));
    ui->dateEditEnd->setDate(QDate::currentDate());

    //executor = new ProgressExecutor(this);

    //model = new ModelReport(this);

    //ui->tableView->setModel(model);

    //connect(executor,SIGNAL(finished()), this, SLOT(updFinished()));
    connect(ui->pushButtonUpd,SIGNAL(clicked(bool)),this,SLOT(upd()));
    connect(ui->pushButtonSave,SIGNAL(clicked(bool)),this,SLOT(save()));
}

FormReport::~FormReport()
{
    delete ui;
}

void FormReport::upd()
{
    /*QString query;
    if (ui->radioButtonPart->isChecked()){
        query=QString("select e.marka, p.diam, p.n_s||'-'||date_part('year',p.dat_part) as part, ev.nam, ep.pack_ed, "
                      "c.ostbeg, c.pack, c.thermo, c.perepack-c.decperepack as perepack, c.perepackbreak, "
                      "c.arch - c.archout as arch, c.isp, c.selfn, c.oth, c.war, c.warout, c.ostend "
                      "from calc_prod_report('%1','%2') as c "
                      "inner join parti p on p.id = c.id_part "
                      "inner join elrtr e on e.id = p.id_el "
                      "inner join elrtr_vars ev on ev.id = p.id_var "
                      "inner join el_pack ep on ep.id = p.id_pack "
                      "order by e.marka, p.diam, part, ev.nam, ep.pack_ed").arg(ui->dateEditBeg->date().addDays(-1).toString("yyyy-MM-dd")).arg(ui->dateEditEnd->date().toString("yyyy-MM-dd"));
    } else {
        query=QString("select e.marka, p.diam, NULL, ev.nam, ep.pack_ed, "
                      "sum(c.ostbeg), sum(c.pack), sum(c.thermo), sum(c.perepack-c.decperepack) as perepack, sum(c.perepackbreak), "
                      "sum(c.arch - c.archout) as arch, sum(c.isp), sum(c.selfn), sum(c.oth), sum(c.war), sum(c.warout), sum(c.ostend) "
                      "from calc_prod_report('%1','%2') as c "
                      "inner join parti p on p.id = c.id_part "
                      "inner join elrtr e on e.id = p.id_el "
                      "inner join elrtr_vars ev on ev.id = p.id_var "
                      "inner join el_pack ep on ep.id = p.id_pack "
                      "group by e.marka, p.diam, ev.nam, ep.pack_ed "
                      "order by e.marka, p.diam, ev.nam, ep.pack_ed").arg(ui->dateEditBeg->date().addDays(-1).toString("yyyy-MM-dd")).arg(ui->dateEditEnd->date().toString("yyyy-MM-dd"));
    }

    executor->setQuery(query);
    executor->start();*/
}

void FormReport::updFinished()
{
    /*QVector<QVector<QVariant>> data=executor->getData();
    QStringList titles=executor->getTitles();
    QVector<QVariant> sums;
    int colCount= titles.size();
    sums.resize(colCount);
    if (colCount>15){
        sums[0]=tr("ИТОГО");
    }
    for (QVector<QVariant> dt : data){
        for (int i=colCount-12; i<colCount; i++){
            sums[i]=sums[i].toDouble()+dt[i].toDouble();
        }
    }
    if (data.size()>1){
        data.push_back(sums);
    }

    model->setModelData(data);
    ui->tableView->resizeToContents();*/
}

void FormReport::save()
{
    //QString tit=tr("Отчет цеха электродов с ")+ui->dateEditBeg->date().toString("dd.MM.yyyy")+tr(" по ")+ui->dateEditEnd->date().toString("dd.MM.yyyy")+".xlsx";
    //ui->tableView->save(tit,2,true,Qt::LandscapeOrientation);
}

/*ModelReport::ModelReport(QWidget *parent) : TableModel(parent)
{
    QStringList titles;
    titles<<tr("Марка")<<tr("Диаметр")<<tr("Партия")<<tr("Вариант")<<tr("Упаковка")<<tr("Налич.на нач.")<<tr("Упаковка")<<tr("Термопак")<<tr("Переуп.(+/-)")<<tr("Брак при переуп.");
    titles<<tr("Архив(+/-)")<<tr("Испытан.")<<tr("Соб. потр.")<<tr("Другие")<<tr("На склад")<<tr("Со склада")<<tr("Ост.на конец");
    setHeader(titles);
}

QVariant ModelReport::data(const QModelIndex &index, int role) const
{
    if((role == Qt::BackgroundRole)) {
        double beg = this->data(this->index(index.row(),5),Qt::EditRole).toDouble();
        double end = this->data(this->index(index.row(),16),Qt::EditRole).toDouble();
        if (beg<0 || end<0) {
            return QVariant(QColor(255,170,170));
        } else {
            return TableModel::data(index,role);
        }
    } else return TableModel::data(index,role);
}*/
