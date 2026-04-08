#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "StudioDirectorSubsystem.h"
#include "TranspersonalGameMode.generated.h"

/**
 * Main Game Mode for Transpersonal Game
 * Coordinates all systems and manages game flow
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ATranspersonalGameMode();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

public:
    /** Initialize all game systems */
    UFUNCTION(BlueprintCallable, Category = "Game Mode")
    void InitializeGameSystems();

    /** Get the Studio Director subsystem */
    UFUNCTION(BlueprintPure, Category = "Game Mode")
    UStudioDirectorSubsystem* GetStudioDirector() const;

    /** Handle player death */
    UFUNCTION(BlueprintCallable, Category = "Game Mode")
    void HandlePlayerDeath(class APawn* DeadPlayer, const FVector& DeathLocation);

    /** Handle creature death */
    UFUNCTION(BlueprintCallable, Category = "Game Mode")
    void HandleCreatureDeath(class APawn* DeadCreature, const FVector& DeathLocation);

    /** Spawn player at safe location */
    UFUNCTION(BlueprintCallable, Category = "Game Mode")
    void RespawnPlayer(class AController* PlayerController);

    /** Check if game is in survival mode */
    UFUNCTION(BlueprintPure, Category = "Game Mode")
    bool IsSurvivalMode() const { return bSurvivalMode; }

    /** Set survival mode */
    UFUNCTION(BlueprintCallable, Category = "Game Mode")
    void SetSurvivalMode(bool bEnabled) { bSurvivalMode = bEnabled; }

protected:
    /** Is the game in survival mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    bool bSurvivalMode = true;

    /** Time of day progression speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    float TimeProgressionSpeed = 1.0f;

    /** Maximum number of creatures in world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    int32 MaxCreatureCount = 100;

    /** Player respawn delay in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    float RespawnDelay = 5.0f;

private:
    /** Initialize physics systems */
    void InitializePhysicsSystems();

    /** Initialize AI systems */
    void InitializeAISystems();

    /** Initialize world systems */
    void InitializeWorldSystems();

    /** Setup game rules */
    void SetupGameRules();

    /** Handle respawn timer */
    void ExecuteRespawn(class AController* PlayerController);

    /** Timer handles for delayed operations */
    TMap<class AController*, FTimerHandle> RespawnTimers;

    /** Studio Director reference */
    UPROPERTY()
    TObjectPtr<UStudioDirectorSubsystem> StudioDirector;
};