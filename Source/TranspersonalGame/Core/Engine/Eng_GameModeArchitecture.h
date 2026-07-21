#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "Eng_GameModeArchitecture.generated.h"

/**
 * Engine Architect's Game Mode Architecture
 * Defines the core game mode structure and rules for the prehistoric survival game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_GameModeArchitecture : public AGameModeBase
{
    GENERATED_BODY()

public:
    AEng_GameModeArchitecture();

protected:
    virtual void BeginPlay() override;
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

public:
    // Game Mode Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Mode")
    bool bSurvivalMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Mode")
    float DayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Mode")
    float NightDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Mode")
    int32 MaxPlayers;

    // Dinosaur Spawn Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs")
    int32 MaxDinosaursInWorld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs")
    float DinosaurSpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs")
    float DinosaurRespawnTime;

    // Survival Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HungerDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float ThirstDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float TemperatureEffectStrength;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Game Mode")
    void SetDayNightCycle(float NewDayDuration, float NewNightDuration);

    UFUNCTION(BlueprintCallable, Category = "Game Mode")
    void SpawnDinosaur(FVector Location, TSubclassOf<class APawn> DinosaurClass);

    UFUNCTION(BlueprintCallable, Category = "Game Mode")
    int32 GetActiveDinosaurCount() const;

    UFUNCTION(BlueprintCallable, Category = "Game Mode")
    void EnableSurvivalMode(bool bEnable);

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Mode")
    void OnDayNightTransition(bool bIsDay);

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Mode")
    void OnPlayerDeath(class APlayerController* DeadPlayer);

protected:
    UFUNCTION()
    void UpdateDayNightCycle();

    UFUNCTION()
    void ManageDinosaurPopulation();

    UFUNCTION()
    void UpdateSurvivalEffects();

private:
    float CurrentTimeOfDay;
    bool bIsDay;
    int32 CurrentDinosaurCount;
};