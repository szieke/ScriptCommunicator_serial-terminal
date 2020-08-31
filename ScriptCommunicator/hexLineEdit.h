#ifndef CUSTOM_WIDGETS_H
#define CUSTOM_WIDGETS_H

#include <QMainWindow>
#include <QShowEvent>
#include <QTreeWidgetItem>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QLineEdit>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QFocusEvent>
#include <QCloseEvent>
#include <QMessageBox>
#include <QComboBox>
#include <QTimer>





///Line edit for entering hexadecimal values.
class HexLineEdit : public QLineEdit
{

public:

    /**
     * Conctructor.
     *
     * @param parent Parent pointer.
     */
    HexLineEdit(QWidget *parent = nullptr)  : QLineEdit(parent),
         m_max(std::numeric_limits<quint32>::max())
    {
        connect(this, &QLineEdit::textChanged, this, &HexLineEdit::textChangedSlot);
    }


    /**
      * Configures the hex line edit.
      *
      * @param max Max. value.
      */
     void configure(quint32 max){m_max = max;textChangedSlot(text());}

     ///Returns the current value.
     quint32 getValue(void);

     ///Is called if the current text/value has changed.
     void textChangedSlot(QString newText);

protected:

    ///Max. value.
    quint32 m_max;

};



#endif // CUSTOM_WIDGETS_H
