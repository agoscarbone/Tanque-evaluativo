#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QtGlobal>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer.start(1000);

    connect(ui->dial_1,SIGNAL(valueChanged(int)),this,SLOT(Qin(int)));
    connect(ui->dial_2,SIGNAL(valueChanged(int)),this,SLOT(Qout(int)));
    connect(ui->dial_3,SIGNAL(valueChanged(int)),this,SLOT(QoutA(int)));
    connect(ui->dial_4,SIGNAL(valueChanged(int)),this,SLOT(QoutB(int)));

    connect(ui->spinBox_1,SIGNAL(valueChanged(int)),this,SLOT(dial_max()));
    connect(ui->spinBox_3,SIGNAL(valueChanged(int)),this,SLOT(dial_max()));
    connect(ui->spinBox_6,SIGNAL(valueChanged(int)),this,SLOT(dial_max()));
    connect(ui->spinBox_7,SIGNAL(valueChanged(int)),this,SLOT(dial_max()));

    connect(ui->spinBox_2,SIGNAL(valueChanged(int)),this,SLOT(t_max()));
    connect(ui->spinBox_5,SIGNAL(valueChanged(int)),this,SLOT(t_max()));
    connect(ui->spinBox_4,SIGNAL(valueChanged(int)),this,SLOT(t_max()));
    connect(&timer,SIGNAL(timeout()),this,SLOT(tanklevel()));


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::Qin(int value)
{
    ui->lcdNumber_1->display(value);
}

void MainWindow::Qout(int value)
{
    ui->lcdNumber_2->display(value);
}

void MainWindow::QoutA(int value)
{
    ui->lcdNumber_3->display(value);
}

void MainWindow::QoutB(int value)
{
    ui->lcdNumber_4->display(value);
}

void MainWindow::dial_max()
{
    ui->dial_1->setMaximum(ui->spinBox_1->value());
    ui->dial_2->setMaximum(ui->spinBox_3->value());
    ui->dial_3->setMaximum(ui->spinBox_6->value());
    ui->dial_4->setMaximum(ui->spinBox_7->value());
}

void MainWindow::t_max()
{
    ui->t1->setMaximum(ui->spinBox_2->value());
    ui->t2->setMaximum(ui->spinBox_5->value());
    ui->t3->setMaximum(ui->spinBox_4->value());

    if (ui->t1->value() > ui->spinBox_2->value())
        ui->t1->setValue(ui->spinBox_2->value());
    if (ui->t2->value() > ui->spinBox_5->value())
        ui->t2->setValue(ui->spinBox_5->value());
    if (ui->t3->value() > ui->spinBox_4->value())
        ui->t3->setValue(ui->spinBox_4->value());
}


void MainWindow::tanklevel()
{
    // Capacidades
    double cap_max    = ui->spinBox_2->value();
    double cap_max_A  = ui->spinBox_5->value();
    double cap_max_B  = ui->spinBox_4->value();

    // Paso de tiempo y factor de aceleración
    double dt = 1.0;            // [s] por tick
    double simulacion = 100.0;  // factor para acelerar
    double dt_eff = dt * simulacion;

    // Caudales: diales en L/h -> L/s
    double caud_in    = ui->dial_1->value() / 3600.0;
    double caud_out   = ui->dial_2->value() / 3600.0;
    double caud_out_A = ui->dial_3->value() / 3600.0;
    double caud_out_B = ui->dial_4->value() / 3600.0;

    // Mostrar
    ui->lcdNumber_1->display(ui->dial_1->value());
    ui->lcdNumber_2->display(ui->dial_2->value());
    ui->lcdNumber_3->display(ui->dial_3->value());
    ui->lcdNumber_4->display(ui->dial_4->value());

    // Niveles actuales
    double nivel_t  = ui->t1->value();
    double nivel_t2 = ui->t2->value();
    double nivel_t3 = ui->t3->value();

    // Habilitar/inhabilitar salidas por nivel bajo (10% de capacidad)
    bool nivel_t_bajo  = (nivel_t  <= 0.1 * cap_max);
    bool nivel_t2_bajo = (nivel_t2 <= 0.1 * cap_max_A);
    bool nivel_t3_bajo = (nivel_t3 <= 0.1 * cap_max_B);

    if (nivel_t_bajo) {

        ui->dial_2->setEnabled(true);

        double margen = 0.10 * caud_in;          // 10% de la entrada
        if (margen < 1e-9) margen = 1e-9;        // evita que quede exactamente 0

        double max_salida = caud_in - margen;    // salida máxima
        if (max_salida < 0.0) max_salida = 0.0;  // por si la entrada es muy chica o 0

        if (caud_out > max_salida) {
            caud_out = max_salida;
        }

    } else {
        ui->dial_2->setEnabled(true);
    }

    if (nivel_t2_bajo) {
        ui->dial_3->setValue(0);
        ui->lcdNumber_3->display(0);
        caud_out_A = 0.0;
    }

    if (nivel_t3_bajo) {
        ui->dial_4->setValue(0);
        ui->lcdNumber_4->display(0);
        caud_out_B = 0.0;
    }


    // ------------------ Reparto desde T1 hacia A y B ------------------
    double flujo_A = 0.0; // L/s hacia T2
    double flujo_B = 0.0; // L/s hacia T3

    // Si hay entrada o salida activa, procesamos
    if ((caud_in > 0.0) || (caud_out > 0.0))
    {
        bool tanque_lleno_A = (nivel_t2 >= cap_max_A);
        bool tanque_lleno_B = (nivel_t3 >= cap_max_B);
        bool Aprohibido     = ui->checkBox_A->isChecked();
        bool Bprohibido     = ui->checkBox_B->isChecked();

        bool A_puede_recibir = (!tanque_lleno_A && !Aprohibido);
        bool B_puede_recibir = (!tanque_lleno_B && !Bprohibido);

        // Salida bloqueada: ningún tanque puede recibir
        bool salida_bloqueada =
            (!A_puede_recibir && !B_puede_recibir) || (Aprohibido && Bprohibido);

        if (salida_bloqueada)
        {
            // Bloqueamos siempre si no hay destino posible
            ui->dial_2->setEnabled(false);
            ui->dial_2->setValue(0);
            ui->lcdNumber_2->display(0);
            caud_out = 0.0;

        }
        else
        {
            // Si al menos uno puede recibir, habilitamos la salida
            ui->dial_2->setEnabled(true);

            // Reparto de flujo
            if (A_puede_recibir && B_puede_recibir)
            {
                flujo_A = 0.5 * caud_out; // mitad a cada uno
                flujo_B = 0.5 * caud_out;
            }
            else if (A_puede_recibir)
            {
                flujo_A = caud_out; // todo a A
                flujo_B = 0.0;
            }
            else if (B_puede_recibir)
            {
                flujo_B = caud_out; // todo a B
                flujo_A = 0.0;
            }

            // ------------------ CONTROL POR NIVEL MÍNIMO DE T1 ------------------

            double min_t1 = 0.1 * cap_max;
            bool t1_en_minimo = (nivel_t <= min_t1);

            // Si T1 está al 10 % o menos → bloqueo de salida
            if (t1_en_minimo)
            {
                ui->dial_2->setEnabled(false);
                ui->dial_2->setValue(0);
                ui->lcdNumber_2->display(0);
                caud_out = 0.0;
            }
            else
            {
                // Si sube del 10 %, reactivo la salida
                ui->dial_2->setEnabled(true);
            }

        }
    }


    // ------------------ Balance + integración ------------------
    // T1: entra caud_in, sale (flujo_A + flujo_B)
    double delta_t1 = (caud_in - (flujo_A + flujo_B)) * dt_eff;
    // T2: entra flujo_A, sale caud_out_A
    double delta_t2 = (flujo_A - caud_out_A) * dt_eff;
    // T3: entra flujo_B, sale caud_out_B
    double delta_t3 = (flujo_B - caud_out_B) * dt_eff;

    nivel_t  += delta_t1;
    nivel_t2 += delta_t2;
    nivel_t3 += delta_t3;

    if (nivel_t >= cap_max) {
        ui->dial_1->setValue(0);
        ui->dial_1->setEnabled(false);
    } else {
        ui->dial_1->setEnabled(true);
    }

    // niveles de los tanques
    ui->t1->setValue(nivel_t);
    ui->t2->setValue(nivel_t2);
    ui->t3->setValue(nivel_t3);

    // ------------------ HABILITAR/DESHABILITAR SALIDAS DE A Y B ------------------

    // Tanque A
    if (nivel_t2 <= 0.1 * cap_max_A) {
        ui->dial_3->setEnabled(false);
        ui->dial_3->setValue(0);
        ui->lcdNumber_3->display(0);
    } else {
        ui->dial_3->setEnabled(true);
    }

    // Tanque B
    if (nivel_t3 <= 0.1 * cap_max_B) {
        ui->dial_4->setEnabled(false);
        ui->dial_4->setValue(0);
        ui->lcdNumber_4->display(0);
    } else {
        ui->dial_4->setEnabled(true);
    }
}
