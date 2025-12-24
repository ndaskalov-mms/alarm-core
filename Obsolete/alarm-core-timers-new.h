#pragma once
//#include <Arduino.h>
#include <stdio.h>

// Maximum timer name length (adjust as needed)
#define MAX_TIMER_NAME 16
// Maximum number of timers (adjust based on your needs)
#define MAX_TIMERS 10

enum timeoutOper {
    SET = 1,
    GET = 2,
    FORCE = 3,
};

#ifndef ARDUINO
#include <windows.h>
unsigned long millis() {
    SYSTEMTIME st;
    GetSystemTime(&st);
    printf("The system time is: %02d:%02d:%02d\n", st.wHour, st.wMinute, st.wSecond);
    unsigned long res = (st.wHour * 60 * 60 * 1000 + st.wMinute * 60 * 1000 + st.wSecond * 1000 + st.wMilliseconds);
    printf("Time in mS: %lu\n", res);
    return res;
}
#endif

// Timer interface
class ITimer {
public:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void reset() = 0;
    virtual bool isActive() const = 0;
    virtual bool hasElapsed() const = 0;
    virtual unsigned long getRemainingTime() const = 0;
    virtual unsigned long getElapsedTime() const = 0;
    virtual void update() = 0;
    virtual ~ITimer() {}
};

// Countdown timer implementation
class CountdownTimer : public ITimer {
private:
    unsigned long duration_ms;
    unsigned long startTime_ms;
    bool active;
    bool elapsed;
    char name[MAX_TIMER_NAME];
    void (*callback)(const char*);  // Simple C-style callback

public:
    CountdownTimer(const char* timerName, unsigned long duration_ms, void (*elapsedCallback)(const char*) = nullptr)
        : duration_ms(duration_ms), startTime_ms(0), active(false), elapsed(false), callback(elapsedCallback) {
        // Safely copy name with bounds checking
        //strncpy(name, timerName, MAX_TIMER_NAME - 1);
        //name[MAX_TIMER_NAME - 1] = '\0';  // Ensure null termination
        snprintf(name,  MAX_TIMER_NAME - 1, "%s", timerName);
      }

    const char* getName() const {
        return name;
    }

    void start() override {
        startTime_ms = millis();
        active = true;
        elapsed = false;
    }

    void stop() override {
        active = false;
    }

    void reset() override {
        startTime_ms = millis();
        elapsed = false;
    }

    bool isActive() const override {
        return active;
    }

    bool hasElapsed() const override {
        if (!active) return false;
        return getElapsedTime() >= duration_ms;
    }

    unsigned long getRemainingTime() const override {
        if (!active) return duration_ms;

        unsigned long elapsed = getElapsedTime();
        if (elapsed >= duration_ms) return 0;
        return duration_ms - elapsed;
    }

    unsigned long getElapsedTime() const override {
        if (!active) return 0;
        return millis() - startTime_ms;
    }

    void update() override {
        // Check if timer just elapsed
        if (active && !elapsed && hasElapsed()) {
            elapsed = true;
            active = false;  // Auto-stop

            // Execute callback if provided
            if (callback) {
                callback(name);
            }
        }
    }

    void setDuration(unsigned long newDuration) {
        duration_ms = newDuration;
    }

    bool checkAndClearElapsed() {
        if (elapsed) {
            elapsed = false;
            return true;
        }
        return false;
    }
};

// Periodic timer with auto-reset
class PeriodicTimer : public CountdownTimer {
private:
    bool repeating;
    bool newCycle;

public:
    PeriodicTimer(const char* name, unsigned long period_ms, void (*callback)(const char*) = nullptr)
        : CountdownTimer(name, period_ms, callback), repeating(true), newCycle(false) {
    }

    void update() override {
        bool wasActive = isActive();
        bool wasElapsed = hasElapsed();

        CountdownTimer::update();

        // Auto-restart when elapsed if repeating
        if (repeating && wasActive && !wasElapsed && hasElapsed()) {
            reset();
            start();
            newCycle = true;
        }
    }

    void setRepeating(bool repeat) {
        repeating = repeat;
    }

    bool checkNewCycle() {
        if (newCycle) {
            newCycle = false;
            return true;
        }
        return false;
    }
};

// Simple timer manager
class TimerManager {
private:
    ITimer* timers[MAX_TIMERS];
    char timerNames[MAX_TIMERS][MAX_TIMER_NAME];
    int timerCount;

public:
    TimerManager() : timerCount(0) {
        // Initialize pointers
        for (int i = 0; i < MAX_TIMERS; i++) {
            timers[i] = nullptr;
        }
    }

    ~TimerManager() {
        // Clean up timers
        for (int i = 0; i < timerCount; i++) {
            if (timers[i]) {
                delete timers[i];
                timers[i] = nullptr;
            }
        }
    }

    bool addTimer(ITimer* timer, const char* name) {
        if (timerCount >= MAX_TIMERS) return false;

        // Copy name with bounds checking
        //strncpy(timerNames[timerCount], name, MAX_TIMER_NAME - 1);
        //timerNames[timerCount][MAX_TIMER_NAME - 1] = '\0';  // Ensure null termination
        snprintf(timerNames[timerCount], MAX_TIMER_NAME - 1, "%s", name);
        timers[timerCount] = timer;
        timerCount++;
        return true;
    }

    ITimer* getTimer(const char* name) {
        for (int i = 0; i < timerCount; i++) {
            if (strcmp(timerNames[i], name) == 0) {
                return timers[i];
            }
        }
        return nullptr;
    }

    void updateAllTimers() {
        for (int i = 0; i < timerCount; i++) {
            if (timers[i]) {
                timers[i]->update();
            }
        }
    }

    bool hasElapsed(const char* name) {
        ITimer* timer = getTimer(name);
        return timer && timer->hasElapsed();
    }

    void startTimer(const char* name) {
        ITimer* timer = getTimer(name);
        if (timer) timer->start();
    }

    void stopTimer(const char* name) {
        ITimer* timer = getTimer(name);
        if (timer) timer->stop();
    }

    void resetTimer(const char* name) {
        ITimer* timer = getTimer(name);
        if (timer) timer->reset();
    }

    CountdownTimer* getTimerAs(const char* name) {
        ITimer* timer = getTimer(name);
        return static_cast<CountdownTimer*>(timer); // No dynamic_cast in Arduino
    }
};

// Timer callback function
void timerElapsedCallback(const char* timerName) {
	printf("Timer elapsed: %s\n", timerName);
}

// Global timer manager
TimerManager timerManager;

void setup() {
#ifdef ARDUINO
    Serial.begin(115200);
    while (!Serial) { ; } // Wait for serial to connect
    Serial.println("Timer System Example");
#endif
    // Create a periodic timer for sensor readings
    PeriodicTimer* sensorTimer = new PeriodicTimer("SENSOR", 2000, timerElapsedCallback);
    timerManager.addTimer(sensorTimer, "SENSOR");
    sensorTimer->start();

    // Create a one-shot timer for display updates
    CountdownTimer* displayTimer = new CountdownTimer("DISPLAY", 5000, timerElapsedCallback);
    timerManager.addTimer(displayTimer, "DISPLAY");
    displayTimer->start();

    // Create an entry delay timer
    CountdownTimer* entryTimer = new CountdownTimer("ENTRY_1", 30000, timerElapsedCallback);
    timerManager.addTimer(entryTimer, "ENTRY_1");
}

void loop() {
    // Update all timers
    timerManager.updateAllTimers();

    // Check specific timer directly
    CountdownTimer* displayTimer = timerManager.getTimerAs("DISPLAY");
    if (displayTimer && displayTimer->checkAndClearElapsed()) {
        printf("Display update required\n");
    }

    // Check sensor timer for new cycles
    PeriodicTimer* sensorTimer = static_cast<PeriodicTimer*>(timerManager.getTimer("SENSOR"));
    if (sensorTimer && sensorTimer->checkNewCycle()) {
        printf("Reading sensors\n");
    }

    // Check if we should start the entry delay
    static bool entryDelayStarted = false;
    if (!entryDelayStarted && millis() > 10000) {
        printf("Starting entry delay!\n");
        timerManager.startTimer("ENTRY_1");
        entryDelayStarted = true;
    }

    // Print remaining time for entry delay
    static unsigned long lastUpdate = 0;
    if (entryDelayStarted && millis() - lastUpdate > 1000) {
        ITimer* entryTimer = timerManager.getTimer("ENTRY_1");
        if (entryTimer && entryTimer->isActive()) {
            unsigned long remaining = entryTimer->getRemainingTime() / 1000;
			printf("Entry delay remaining: %lu seconds\n", remaining);
        }
        lastUpdate = millis();
    }

    // Small delay to prevent flooding serial
#ifdef ARDUINO
    delay(100);
#endif
}

extern TimerManager alarmTimerManager;

bool timeoutOps(int oper, int whichOne) {
    // Convert old timer IDs to name strings
    char timerName[MAX_TIMER_NAME];
    snprintf(timerName, MAX_TIMER_NAME - 1, "TIMER_%d", whichOne);

    switch (oper) {
    case SET:
        alarmTimerManager.resetTimer(timerName);
        alarmTimerManager.startTimer(timerName);
        return true;
    case GET:
        return alarmTimerManager.hasElapsed(timerName);
    case FORCE:
        // Make timer elapsed immediately
        CountdownTimer* timer = alarmTimerManager.getTimerAs(timerName);
        if (timer) {
            timer->stop();
            return true;
        }
        return false;
    }
    return false;
}