#pragma once

#include <stdint.h>

enum class RunState { INITIALIZING, LOADING_SAVES, PLAYING, RESETTING, QUITTING, QUITTING_WAITING_FOR_SAVE, QUIT };

class GameState
{
public:
    GameState();
    ~GameState() = default;

    RunState GetRunState() const { return CurrentRunState; }
    void SetRunState(RunState state) { CurrentRunState = state; }

    int64_t GetGlobalHighScore() const { return GlobalHighScore; }
    void SetGlobalHighScore(int64_t score);

    int64_t GetLocalHighScore() const { return LocalHighScore; }
    void SetLocalHighScore(int64_t score);

    bool DoesGlobalNeedSaving() const { return GlobalNeedsSaving; }
    bool DoesLocalNeedSaving() const { return LocalNeedsSaving; }
    void ClearNeedsSaving() { GlobalNeedsSaving = LocalNeedsSaving = false; }

private:
    RunState CurrentRunState;
    int64_t GlobalHighScore;
    int64_t LocalHighScore;
    bool GlobalNeedsSaving;
    bool LocalNeedsSaving;
};
