#pragma once
#include "client_global.h"
#include "interop.h"

#include <cstdint>
#include <string>
#include <memory>
#include <functional>

namespace emserverproxy {


class EmserverAdapter;
/*!
 * \brief Класс EmserverFacade обеспечивает взаимодействие с встраиваемым сервером по сети
 */
class CLIENT_EXPORT EmserverFacade
{
public:
    /*! обратный вызов для обновления состояния и параметров подключения к серверу */
    using emserverConnectedStateChanging = std::function<void(EmserverFacade*, const ConnectedState&)>;
    /*! обратный вызов для обновления информации о технических возможностях комплекса */
    using emserverResponseCapabilities = std::function<void(EmserverFacade*, const Sensors&, const requestid&)>;
    /*!*/
    using emserverResponseSensorActivation = std::function<void(EmserverFacade*, ISensorState::Ptr, const requestid&)>;
    /*! обратный вызов для индикации ошибки, возникающей на сервере */
    using emserverServerRuntimeNotification = std::function<void(EmserverFacade*, const ErrorDescriptor&)>;

    enum
    {
        WrongCallCode = -1,       /*< Невозможно сформировать управляющую команду */
        InvalidSessionError = -2  /*< Невозможно отправить управляющую команду, возможно соединение разорвано */
    };

    /*!
     * \brief EmserverFacade конструктор.
     */
    EmserverFacade();

    /*!
     * \brief инициирует подключение к удалённому узлу
     * \param remoteIP IPv4 адрес удалённого узла
     * \param port порт удалённого узла
     *
     * Подключение к заданному IPv4:порт. Обновление состояния будет через обратный вызов \ref emserverConnectedStateChanging
     */
    void connectionOpen(const std::string &remoteIP, const uint16_t port);

    /*!
     * \brief останавливает попытки подключения к удалённому узлу или разрывает установленное подключение. В ином случае - ничего не делает
     */
    void connectionClose();

    /*!
     * \brief посылает разовый запрос технических возможностей комплекса.
     * \return идентификатор операции вызова метода, либо код ошибки
     *
     * Информация о технических возможностях комплекса будет обновлена через обратный вызов \ref emserverResponseCapabilities
     */
    requestid sendRequestCapabilities();



    requestid sendSensorsControl(const Sensors& sensorsParameters);

    // callback handlers registration
    /*!
     * \brief registerCallbackConnectedStateChanged регистрация функции обратного вызова для обновления информации о состоянии подключения
     * \param ptrConnected указатель на функцию обратного вызова
     */
    void registerCallbackConnectedStateChanged(const emserverConnectedStateChanging &ptrConnected);

    /*!
     * \brief registerCallbackCapabilities регистрация функции обратного вызова для обновления информации о технических возможностях комплекса
     * \param ptrCaps указатель на функцию обратного вызова
     */
    void registerCallbackCapabilities(const emserverResponseCapabilities &ptrCaps);

    /*!
    * \brief registerCallbackSensorActivated регистрация функции обратного вызова для события "сработки" конкретного сенсора
    * \param ptrSensActive указатель на функцию обратного вызова
    */
    void registerCallbackSensorActivated(const emserverResponseSensorActivation &ptrSensActive);

    /*!
     * \brief registerCallbackServerRuntimeNotifications регистрация функции обратного вызова для обновления информации об ошибках при вызове методов сервера или
     * в процессе работы сервера
     * \param ptrErrorHandler указатель на функцию обратного вызова
     */
    void registerCallbackServerRuntimeNotifications(const emserverServerRuntimeNotification& ptrErrorHandler);


private:
    std::shared_ptr<EmserverAdapter> m_pAdapter;
};

} // namespace emserverproxy
