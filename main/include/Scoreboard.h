#ifndef _SCOREBOARD_MAIN_H
#define _SCOREBOARD_MAIN_H

#include "GattServer.h"
#include "GattSpecial.h"

namespace Model {
    typedef void(*INotifyModelChanged)(void);

    class Score : public BLE::Characteristic {
    private:
        uint8_t m_score;
        BLE::Special::UserDescriptionDescriptor* m_desc;
        void* GetValue() override;
        size_t GetValueSize() override;
        void SetValueFromBuffer(void* buf, uint16_t len) override;
        uint16_t value_maxlen() override;
        INotifyModelChanged onChange;
    public:
        Score(BLE::UUID uuid, std::string description, INotifyModelChanged cb);
        void setScore(uint8_t s);
        uint8_t getScore();
        operator std::string();
    };

    class Team : public BLE::Characteristic {
    private:
        std::string m_name;
        BLE::Special::UserDescriptionDescriptor* m_desc;
        void* GetValue() override;
        size_t GetValueSize() override;
        void SetValueFromBuffer(void* buf, uint16_t len) override;
        uint16_t value_maxlen() override;
        INotifyModelChanged onChange;
    public:
        Team(BLE::UUID uuid, std::string desc, std::string name, INotifyModelChanged cb);
        void setName(std::string name);
        std::string getName();
        operator std::string();
    };

} // namespace Model

#endif // _SCOREBOARD_MAIN_H