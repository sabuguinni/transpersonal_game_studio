// TranspersonalGameMode.h
// Engine: Unreal Engine 5.5
// Studio: Transpersonal Game Studio
// Agent: #03 — Core Systems Programmer
// Purpose: Game mode for prehistoric survival — sets default pawn, controller, and HUD classes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TranspersonalGameMode.generated.h"

/**
 * ATranspersonalGameMode
 *
 * Primary game mode for the prehistoric survival game.
 * Responsibilities:
 *   - Sets DefaultPawnClass = ATranspersonalCharacter
 *   - Manages player spawning at PlayerStart actors
 *   - Provides hooks for game state transitions (future: day/night, wave events)
 *
 * Usage: Set this as the GameMode in MinPlayableMap World Settings.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ATranspersonalGameMode();

    // ~AGameModeBase interface
    virtual void BeginPlay() override;
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
    virtual void RestartPlayer(AController* NewPlayer) override;
    // ~End AGameModeBase interface

    /**
     * Returns true if the game is currently in survival mode (player alive, world active).
     * Used by UI and quest systems to gate certain interactions.
     */
    UFUNCTION(BlueprintCallable, Category = "Game|Survival")
    bool IsSurvivalActive() const { return bSurvivalActive; }

    /**
     * Called by survival system when player dies — triggers respawn or game over.
     */
    UFUNCTION(BlueprintCallable, Category = "Game|Survival")
    void OnPlayerDied(AController* DeadPlayer);

protected:
    /** Whether the survival simulation is running (hunger/thirst/stamina drains active). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Survival")
    bool bSurvivalActive = true;

    /** Respawn delay in seconds after player death. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Survival", meta = (ClampMin = "0.0", ClampMax = "30.0"))
    float RespawnDelay = 5.0f;
};
