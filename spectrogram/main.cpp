#include <qapplication.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qslider.h>
#include <qlabel.h>
#include <qcheckbox.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "plot.h"
#include "qwt_color_map.h"

#include "math.h"

#include "variables.h"

class MainWindow: public QMainWindow
{
public:
    MainWindow( QWidget * = NULL );

private:
    Plot *d_plot;
};

MainWindow::MainWindow( QWidget *parent ):
    QMainWindow( parent )
{
    d_plot = new Plot( this );

    setCentralWidget( d_plot );

    QToolBar *toolBar = new QToolBar( this );

#ifndef QT_NO_PRINTER
    QToolButton *btnPrint = new QToolButton( toolBar );
    btnPrint->setText( "Print" );
    btnPrint->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    toolBar->addWidget( btnPrint );
    connect( btnPrint, SIGNAL( clicked() ),
        d_plot, SLOT( printPlot() ) );

    toolBar->addSeparator();
#endif

    toolBar->addWidget( new QLabel("Color Map " ) );
    QComboBox *mapBox = new QComboBox( toolBar );
    mapBox->addItem( "RGB" );
    mapBox->addItem( "Indexed Colors" );
    mapBox->addItem( "Hue" );
    mapBox->addItem( "Alpha" );
    mapBox->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    toolBar->addWidget( mapBox );
    connect( mapBox, SIGNAL( currentIndexChanged( int ) ),
             d_plot, SLOT( setColorMap( int ) ) );

    toolBar->addWidget( new QLabel( " Opacity " ) );
    QSlider *slider = new QSlider( Qt::Horizontal );
    slider->setRange( 0, 255 );
    slider->setValue( 255 );
    connect( slider, SIGNAL( valueChanged( int ) ),
        d_plot, SLOT( setAlpha( int ) ) );

    toolBar->addWidget( slider );
    toolBar->addWidget( new QLabel("   " ) );

    QCheckBox *btnSpectrogram = new QCheckBox( "Spectrogram", toolBar );
    toolBar->addWidget( btnSpectrogram );
    connect( btnSpectrogram, SIGNAL( toggled( bool ) ),
        d_plot, SLOT( showSpectrogram( bool ) ) );

    QCheckBox *btnContour = new QCheckBox( "Contour", toolBar );
    toolBar->addWidget( btnContour );
    connect( btnContour, SIGNAL( toggled( bool ) ),
        d_plot, SLOT( showContour( bool ) ) );

    addToolBar( toolBar );

    btnSpectrogram->setChecked( true );
    btnContour->setChecked( false );

}

uint8_t image_buff[1280 * 720];

int main( int argc, char **argv )
{

    int fd = open("//Users/jsvirzi/Downloads/600_1280x720.nv21", O_RDONLY, S_IREAD);
    size_t n_bytes = read(fd, image_buff, sizeof(image_buff));

    JsvHistogram jsvHistogram(0.0, 1.0, 0.0, 1.0, (unsigned int) 0);

    double logr = jsvHistogram.image_entropy(image_buff, 1280, 720);

    printf("image entropy = %lf\n", logr);

    return 0;

#if 0
    const double c = 0.5 * log(2.0 * M_PI);

    static double log_factorial_lut[256];

    int i = 0;
    log_factorial_lut[i++] = log(1.0);
    log_factorial_lut[i++] = log(2 * 1.0);
    log_factorial_lut[i++] = log(3 * 2 * 1.0);
    log_factorial_lut[i++] = log(4 * 3 * 2 * 1.0);
    log_factorial_lut[i++] = log(5 * 4 * 3 * 2 * 1.0);
    log_factorial_lut[i++] = log(6 * 5 * 4 * 3 * 2 * 1.0);
    log_factorial_lut[i++] = log(7 * 6 * 5 * 4 * 3 * 2 * 1.0);
    log_factorial_lut[i++] = log(8 * 7 * 6 * 5 * 4 * 3 * 2 * 1.0);
    log_factorial_lut[i++] = log(9 * 8 * 7 * 6 * 5 * 4 * 3 * 2 * 1.0);
    for (; i < 256; ++i) {
        log_factorial_lut[i] = c + 0.5 * log(i) + i * log((double) i);
    }

    printf("log facs = %lf vs %lf\n", log_factorial_lut[8], c + 0.5 * log(9.0) + 9.0 * log(9.0) - 9.0);

#endif

    return 0;

    QApplication a( argc, argv );
    a.setStyle( "Windows" );

    MainWindow mainWindow;
    mainWindow.resize( 600, 400 );
    mainWindow.show();

    return a.exec();
}
