#ifndef API_SERVER_H
#define API_SERVER_H

#include "cserver_global.h"
#include "api_capabilities.h"
#include "api_sensors.h"


#ifndef CSERVER_BUILD_STATIC
# if defined(__cplusplus)
extern "C" {
# endif
#endif

enum ServerLoggingLevel : uint32_t
{
    SrvDebug = 0,
    SrvInfo = 1,
    SrvWarning = 2,
    SrvError = 3,
    SrvFatal = 4,
    MaxValue
};

#pragma pack(push, 1)
struct ErrorDescription
{
    char *pError;
    uint32_t size;
    ErrorDescription() : pError(nullptr), size(0) { }
};

struct CNIpAddress
{
    char *pAddress;
    uint32_t size;
    CNIpAddress() : pAddress(nullptr), size(0)
    { }
};

struct CNString
{
    char *pStringData;
    uint32_t size;
    CNString() : pStringData(nullptr), size(0) { }
    ~CNString() { delete[] pStringData; }
    void reset() { delete[] pStringData; }
};

#pragma pack(pop)

typedef int64_t requestid;
typedef void (*cbClientConnected)(CNIpAddress *ip, uint16_t port);
typedef void (*cbLog)(CNString *message);
typedef int32_t (*cbCapabilitiesRequest)(requestid id);
typedef int32_t (*cbSensorControlrequest)(requestid id, const Sensors* pSensors);


/*!
 * \brief server_create функция создающаяя сервер
 * \return всегда true
 */
CSERVER_EXPORT bool server_create();

/*!
 * \brief server_update_loglevel функция для определения степени подробности протокола работы сервера
 * \param lvl уровень логгирования от 0 - самый подробный лог до 4 - только фатальные сообщения
 * \return true если заданный уровень лога был применён
 */
CSERVER_EXPORT bool server_update_loglevel(uint32_t lvl);

/*!
 * \brief server_update_logpath функция для определения метста, куда будут писаться логи
 * \param path путь к папке, доступной на запись
 */
CSERVER_EXPORT void server_update_logpath(CNString *path);

/*!
 * \brief server_register_callback_Logging коллбек для логгирования через внешнее приложение
 * \param cb указатель на функцию логгирования
 */
CSERVER_EXPORT void server_register_callback_Logging(cbLog cb);

/*!
 * \brief server_register_callback_ClientConnected регистрация коллбека, который будет вызыван при подключении клиента
 * \param cb коллбек
 * \return 0 если коллбек зарегистрирован успешно или значение меньше 0, если произошла ошибка
 */
CSERVER_EXPORT int server_register_callback_ClientConnected(cbClientConnected cb);

/*!
 * \brief server_register_callback_CapabilitiesRequest регистрация коллбека, который будет вызыван при запросе текущей конфигурации системы
 * \param cb коллбек
 * \return 0 если коллбек зарегистрирован успешно или значение меньше 0, если произошла ошибка
 */
CSERVER_EXPORT int server_register_callback_CapabilitiesRequest(cbCapabilitiesRequest cb);

/*!
 * \brief server_register_callback_SensorsControlRequest регистрация коллбека, который будет вызыван при изменении конфигурации сенсора
 * \param cb коллбек
 * \return 0 если коллбек зарегистрирован успешно или значение меньше 0, если произошла ошибка
 */
CSERVER_EXPORT int server_register_callback_SensorsControlRequest(cbSensorControlrequest cb);

/*!
 * \brief server_listen открывает сетевой сокет по интерфейсу address:port в ожидании поделючения
 * \param address IPv4 адрес подключения
 * \param port порт
 * \return 0 если всё хорошо или значение меньше 0, если произошла ошибка
 */
CSERVER_EXPORT int server_listen(CNIpAddress *address, int port);

/*!
 * \brief send_capabilities отправка текущей конфигурации системы подключенному клиенту
 * \param id идентификатор запроса
 * \param caps конфигурация датчиков
 * \return 0 если всё хорошо или значение меньше 0, если произошла ошибка
 */
CSERVER_EXPORT int send_capabilities(const requestid id, const Capabilities caps);
/*!
 * \brief send_activesensor отпрака подключенному клиенту события "сработки" датчика
 * \param id идентификатор запроса
 * \param pSensor сработавший датчик
 * \return 0 если всё хорошо или значение меньше 0, если произошла ошибка
 */
CSERVER_EXPORT int send_activesensor(const requestid id, const SensorBase *pSensor);

/*!
 * \brief server_runtime_error функция для информирования клиента, о том, что в процессе обработки его запроса произошла ошибка
 * \param code код ошибки
 * \param errorDescription описание ошибки
 */
CSERVER_EXPORT void server_runtime_error(int32_t code, ErrorDescription errorDescription);

/*!
 * \brief server_close закрытие слушающего сокета, подключенный клиент будет отключен
 */
CSERVER_EXPORT void server_close();
/*!
 * \brief server_release полная остановка сервера, разрущение выделенных ресурсов
 */
CSERVER_EXPORT void server_release();

#ifndef CSERVER_BUILD_STATIC
# if defined(__cplusplus)
}
# endif
#endif

#endif
