#pragma once

#include "interop_sensors.h"

#include <vector>
#include <cstdint>
#include <string>
#include <cassert>


namespace emserverproxy {

/*! идентификатор вызова метода-запроса для EmserverFacade*/
using requestid = int64_t;

/*!
 * \brief Структура NetworkEndpoint позволяет определять параметры сетевого соединения на локальной/удалённой машинне
 */
struct NetworkEndpoint
{
    std::string address;    ///< IPv4 адрес узла сети
    uint16_t port;          ///< порт
    NetworkEndpoint() : address(""), port(0) {} ///< конструктор без параметров
    NetworkEndpoint(const std::string& addr, const uint16_t p) : address{ addr }, port{ p } { } ///< параметризованный конструктор
};
/*!
 * \brief Структура ConnectedState содержит параметры текущего подключения
 *
 * Когда подключение к между клиентом и сервером ещё не установлено, соответсвующее поле структуры будет не заполнено.
 */
struct ConnectedState
{
    NetworkEndpoint localPoint;  ///< параметры подключения, используемые локальной машиной
    NetworkEndpoint remotePoint; ///< параметры подключения, используемые удалённой машиной
};

// TODO : добавить документацию
using Sensors = std::vector<SensorBase::Ptr>;






/*!
 * \brief Структура ErrorDescriptor описывает ошибки методов сервера, возникающие как при управляющем воздействии и в ходе нормальной работы
 */
struct ErrorDescriptor
{
    enum ErrorType { MethodParametersError = 0x00 };
    int64_t methodCode;         ///< идентификатор последовательности вызова метода. Если равен \ref ErrorType::MethodParametersError , то это ошибка "валидации параметров"
    int32_t errorCode;          ///< код ошибки. 0 - успешный вызов метода
    std::string description;    ///< описание ошибки, если есть.

    ErrorDescriptor() : methodCode(MethodParametersError), errorCode(0) { }
};


} // emserverproxy
