#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorTypes.h"
#include "NPC_BehaviorManager.generated.h"

class ANPCBase;
class ANPC_TribalHuman;
class ADinosaurBase;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_BehaviorManagerSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float NPCUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float PlayerDetectionRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float NPCInteractionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    int32 MaxActiveNPCs = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    bool bEnableAdvancedBehaviors = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_GlobalBehaviorState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global State")
    float TimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global State")
    ENPC_WeatherType CurrentWeather = ENPC_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global State")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global State")
    FVector PlayerLastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global State")
    bool bPlayerNearby = false;
};

/**
 * Central manager for all NPC behaviors in the world
 * Coordinates between different NPC types and manages global behavior states
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_BehaviorManager : public AActor
{
    GENERATED_BODY()

public:
    ANPC_BehaviorManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Management Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Management")
    void RegisterNPC(AActor* NPCActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Management")
    void UnregisterNPC(AActor* NPCActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Management")
    void UpdateAllNPCBehaviors();

    UFUNCTION(BlueprintCallable, Category = "NPC Management")
    TArray<AActor*> GetNPCsInRadius(FVector Location, float Radius);

    // Global State Management
    UFUNCTION(BlueprintCallable, Category = "Global State")
    void UpdateGlobalBehaviorState();

    UFUNCTION(BlueprintCallable, Category = "Global State")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Global State")
    void SetWeatherType(ENPC_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Global State")
    void SetThreatLevel(float NewThreatLevel);

    // Player Interaction
    UFUNCTION(BlueprintCallable, Category = "Player Interaction")
    void NotifyPlayerLocation(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Player Interaction")
    void TriggerNPCReaction(AActor* NPCActor, ENPC_ReactionType ReactionType);

    // Behavior Coordination
    UFUNCTION(BlueprintCallable, Category = "Behavior Coordination")
    void CoordinateTribalBehaviors();

    UFUNCTION(BlueprintCallable, Category = "Behavior Coordination")
    void CoordinateDinosaurBehaviors();

    UFUNCTION(BlueprintCallable, Category = "Behavior Coordination")
    void HandleNPCConflict(AActor* NPC1, AActor* NPC2);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Global State")
    FNPC_GlobalBehaviorState GetGlobalBehaviorState() const { return GlobalBehaviorState; }

    UFUNCTION(BlueprintPure, Category = "Settings")
    FNPC_BehaviorManagerSettings GetBehaviorSettings() const { return BehaviorSettings; }

protected:
    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FNPC_BehaviorManagerSettings BehaviorSettings;

    // Global State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Global State")
    FNPC_GlobalBehaviorState GlobalBehaviorState;

    // Registered NPCs
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Management")
    TArray<TWeakObjectPtr<AActor>> RegisteredNPCs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Management")
    TArray<TWeakObjectPtr<ANPC_TribalHuman>> TribalHumans;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Management")
    TArray<TWeakObjectPtr<ADinosaurBase>> Dinosaurs;

    // Timers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Internal")
    float LastUpdateTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Internal")
    float LastPlayerCheckTime = 0.0f;

private:
    // Internal helper functions
    void CleanupInvalidNPCs();
    void UpdatePlayerProximity();
    void ProcessNPCInteractions();
    AActor* GetPlayerActor();
};