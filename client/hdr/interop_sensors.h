#pragma once
#include "client_global.h"

#include <cstdint>
#include <memory>

namespace emserverproxy {

class ISensorState
{
public:
    using Ptr = std::shared_ptr<ISensorState>;
    ISensorState() = default;
    virtual ~ISensorState() = default;
    virtual int32_t sensorId() const = 0;
};

/*!
 * \brief Класс SensorBase является базовым виртуальным классом для всех реализаций фильтров
 */
class CLIENT_EXPORT SensorBase
{
public:
    using sensorid = int32_t;
    using Ptr = std::shared_ptr<SensorBase>;   ///< определение типа "умный указатель" для данныго класса
    /*!
     * \brief Перечисление SensorType определяет тип фильтра
     */
    enum SensorType
    {
        TypeUnknown = 0,
        TypeDryContact = 1,
        TypeTemperature = 2
    };
    SensorBase(const sensorid &id) : m_id(id) { };
    virtual ~SensorBase() = default;
    virtual SensorType type() const = 0;                 ///< тип датчика
    virtual sensorid sensorId() const { return m_id; };  ///< идентификатор датчика
    virtual void updateState(ISensorState::Ptr &state) = 0;
    virtual ISensorState::Ptr state() const = 0;

protected:
    int32_t m_id;
};


/*!
 * \brief Класс SensorDryContact сожержит параметры работы датчика типа "сухой котакт"
 */
class CLIENT_EXPORT SensorDryContact : public SensorBase
{
public:
    using Ptr = std::shared_ptr<SensorDryContact>;                                           ///< определение типа "умный указатель" для данного класса
    class State : public ISensorState
    {
    public:
        using Ptr = std::shared_ptr<State>;
        State(const sensorid &id, bool currentState) : m_id{id}, m_state{currentState} {  };

        sensorid sensorId() const override { return m_id; }
        bool state() const { return m_state; }

    private:
        const sensorid m_id;
        bool m_state;
    };

    SensorDryContact(const sensorid id, const bool triggeredState);                              ///< Конструктор принимает @param[in] checkLevel - уровень по которому будет генерироваться событие
    virtual SensorBase::SensorType type() const final { return SensorBase::TypeDryContact; } ///< @return тип датчика
    virtual void updateState(ISensorState::Ptr &state) override;
    bool triggeredState() const;                                                             ///< @return уровень по которому датчик будет активироваться
    ISensorState::Ptr state() const override { return m_currentState; }

private:
    State::Ptr m_currentState;
    bool m_triggeredState;
};

/*!
 * \brief Класс SensorTemperature содержит параметры работы датчика температуры
 */
class CLIENT_EXPORT SensorTemperature : public SensorBase
{
public:
    using Ptr = std::shared_ptr<SensorTemperature>;                                           ///< определение типа "умный указатель" для данного класса
    class State : public ISensorState
    {
    public:
        using Ptr = std::shared_ptr<State>;
        State(const sensorid &id, int32_t temperatureValue) : m_id{id}, m_temperatureValue{temperatureValue}
        { }

        sensorid sensorId() const override { return m_id; }
        int32_t value() const { return m_temperatureValue; }
    private:
        const sensorid m_id;
        int32_t m_temperatureValue;
    };

    SensorTemperature(const sensorid id, int32_t tresholdLevel);                              ///< Конструктор. Принимает @param[in] tresholdLevel - порог срабатывания события, в градусах Кельвина
    virtual SensorBase::SensorType type() const final { return SensorBase::TypeTemperature; } ///< @return тип датчика
    virtual void updateState(ISensorState::Ptr &state) override;
    int32_t tresholdLevel() const;                                                            ///< @return порог срабатывания датчика
    ISensorState::Ptr state() const override { return m_currentState; }

private:
    State::Ptr m_currentState;
    int32_t m_tresholdLevel;
};


} // emserverproxy
