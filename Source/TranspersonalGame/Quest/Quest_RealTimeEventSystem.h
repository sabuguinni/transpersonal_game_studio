#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Quest_RealTimeEventSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_RealTimeEventType : uint8
{
    None UMETA(DisplayName = "None"),
    DinosaurMigration UMETA(DisplayName = "Dinosaur Migration"),
    WeatherStorm UMETA(DisplayName = "Weather Storm"),
    ResourceDepletion UMETA(DisplayName = "Resource Depletion"),
    PredatorHunt UMETA(DisplayName = "Predator Hunt"),
    HerdGathering UMETA(DisplayName = "Herd Gathering"),
    TerritorialDispute UMETA(DisplayName = "Territorial Dispute"),
    NightDanger UMETA(DisplayName = "Night Danger"),
    WaterSourceDrying UMETA(DisplayName = "Water Source Drying")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_RealTimeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    EQuest_RealTimeEventType EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FVector EventLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    float EventRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    float EventDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    float EventIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    float TimeRemaining;

    FQuest_RealTimeEvent()
    {
        EventType = EQuest_RealTimeEventType::None;
        EventName = TEXT("Unknown Event");
        EventDescription = TEXT("An unknown event is occurring");
        EventLocation = FVector::ZeroVector;
        EventRadius = 5000.0f;
        EventDuration = 300.0f; // 5 minutes
        EventIntensity = 1.0f;
        bIsActive = false;
        TimeRemaining = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_EventSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    EQuest_RealTimeEventType EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float MinSpawnInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float MaxSpawnInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float EventProbability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    TArray<FVector> PreferredLocations;

    FQuest_EventSpawnData()
    {
        EventType = EQuest_RealTimeEventType::None;
        MinSpawnInterval = 600.0f; // 10 minutes
        MaxSpawnInterval = 1800.0f; // 30 minutes
        EventProbability = 0.3f;
        PreferredLocations.Empty();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_RealTimeEventSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_RealTimeEventSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
    TArray<FQuest_RealTimeEvent> ActiveEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
    TArray<FQuest_EventSpawnData> EventSpawnData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
    float EventUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
    int32 MaxConcurrentEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
    bool bEventsEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
    float GlobalEventMultiplier;

private:
    FTimerHandle EventUpdateTimer;
    float LastEventSpawnTime;
    TMap<EQuest_RealTimeEventType, float> EventCooldowns;

public:
    UFUNCTION(BlueprintCallable, Category = "Events")
    void StartRealTimeEvent(EQuest_RealTimeEventType EventType, FVector Location, float Duration = 300.0f);

    UFUNCTION(BlueprintCallable, Category = "Events")
    void StopRealTimeEvent(int32 EventIndex);

    UFUNCTION(BlueprintCallable, Category = "Events")
    void StopAllEvents();

    UFUNCTION(BlueprintCallable, Category = "Events")
    TArray<FQuest_RealTimeEvent> GetActiveEvents() const;

    UFUNCTION(BlueprintCallable, Category = "Events")
    bool IsEventActiveAtLocation(FVector Location, float Radius = 1000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Events")
    FQuest_RealTimeEvent GetNearestEvent(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Events")
    void SetEventEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Events")
    void SetGlobalEventMultiplier(float Multiplier);

protected:
    UFUNCTION()
    void UpdateEvents();

    UFUNCTION()
    void TrySpawnRandomEvent();

    UFUNCTION()
    void ProcessEventEffects(FQuest_RealTimeEvent& Event);

    UFUNCTION()
    FVector GetRandomEventLocation(EQuest_RealTimeEventType EventType) const;

    UFUNCTION()
    bool CanSpawnEvent(EQuest_RealTimeEventType EventType) const;

    UFUNCTION()
    void InitializeEventSpawnData();

    UFUNCTION()
    void CleanupExpiredEvents();

    UFUNCTION()
    void NotifyPlayersOfEvent(const FQuest_RealTimeEvent& Event);

public:
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnEventStarted(const FQuest_RealTimeEvent& Event);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnEventEnded(const FQuest_RealTimeEvent& Event);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnEventUpdated(const FQuest_RealTimeEvent& Event);
};