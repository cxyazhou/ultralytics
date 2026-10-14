/****************************************************************************
** Meta object code from reading C++ file 'GuiMain.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.13.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../inc/GuiMain.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GuiMain.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.13.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_GuiMainThread_t {
    QByteArrayData data[1];
    char stringdata0[14];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GuiMainThread_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GuiMainThread_t qt_meta_stringdata_GuiMainThread = {
    {
QT_MOC_LITERAL(0, 0, 13) // "GuiMainThread"

    },
    "GuiMainThread"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GuiMainThread[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void GuiMainThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject GuiMainThread::staticMetaObject = { {
    &QThread::staticMetaObject,
    qt_meta_stringdata_GuiMainThread.data,
    qt_meta_data_GuiMainThread,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *GuiMainThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GuiMainThread::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GuiMainThread.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int GuiMainThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    return _id;
}
struct qt_meta_stringdata_GuiMain_t {
    QByteArrayData data[8];
    char stringdata0[99];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GuiMain_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GuiMain_t qt_meta_stringdata_GuiMain = {
    {
QT_MOC_LITERAL(0, 0, 7), // "GuiMain"
QT_MOC_LITERAL(1, 8, 15), // "sigLabelShowImg"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 7), // "QImage*"
QT_MOC_LITERAL(4, 33, 25), // "on_m_register_lav_clicked"
QT_MOC_LITERAL(5, 59, 22), // "on_m_start_det_clicked"
QT_MOC_LITERAL(6, 82, 12), // "LabelShowImg"
QT_MOC_LITERAL(7, 95, 3) // "img"

    },
    "GuiMain\0sigLabelShowImg\0\0QImage*\0"
    "on_m_register_lav_clicked\0"
    "on_m_start_det_clicked\0LabelShowImg\0"
    "img"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GuiMain[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   37,    2, 0x08 /* Private */,
       5,    0,   38,    2, 0x08 /* Private */,
       6,    1,   39,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 3,    7,

       0        // eod
};

void GuiMain::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GuiMain *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sigLabelShowImg((*reinterpret_cast< QImage*(*)>(_a[1]))); break;
        case 1: _t->on_m_register_lav_clicked(); break;
        case 2: _t->on_m_start_det_clicked(); break;
        case 3: _t->LabelShowImg((*reinterpret_cast< QImage*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (GuiMain::*)(QImage * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GuiMain::sigLabelShowImg)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject GuiMain::staticMetaObject = { {
    &QMainWindow::staticMetaObject,
    qt_meta_stringdata_GuiMain.data,
    qt_meta_data_GuiMain,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *GuiMain::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GuiMain::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GuiMain.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "CCPPObject"))
        return static_cast< CCPPObject*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int GuiMain::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void GuiMain::sigLabelShowImg(QImage * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
