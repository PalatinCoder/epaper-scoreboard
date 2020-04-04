#ifndef _SCOREBOARD_MAIN_H
#define _SCOREBOARD_MAIN_H

#include "GattServer.h"
#include "GattSpecial.h"
#include "epaper.h"

class View {
private:
    EPaper::Config displayConfig;
public:
    static View& Instance();
    ~View() {};
    /**
     * Redraw the view
     */
    void Update(std::string scoreHome, std::string scoreAway, std::string teamHome, std::string teamAway);
    /**
     * Initialize the scoreboard view
     */
    void Init();
    /**
     * Set the view to standby, i.e. display a "screensaver"
     */
    void Standby();
protected:
    View() {};
    View(const View& other) {};
    View& operator = (const View&);
};

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
        INotifyModelChanged notifyModelChanged;
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
        INotifyModelChanged notifyModelChanged;
    public:
        Team(BLE::UUID uuid, std::string desc, std::string name, INotifyModelChanged cb);
        void setName(std::string name);
        std::string getName();
        operator std::string();
    };

} // namespace Model

#endif // _SCOREBOARD_MAIN_H