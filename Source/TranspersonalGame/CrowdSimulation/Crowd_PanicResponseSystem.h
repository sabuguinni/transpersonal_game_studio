#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_PanicResponseSystem.generated.h"

// Panic response state for crowd entities
UENUM(BlueprintType)
enum class ECrowd_PanicState : uint8
{
    Calm = 0,
    Alerted,
    Panicking,
    Fleeing,
    Evacuating,
    Hiding,
    Regrouping
};

// Panic trigger types
UENUM(BlueprintType)
enum class ECrowd_PanicTrigger : uint8
{
    PredatorSighting = 0,
    LoudNoise,
    CombatNearby,
    GroupPanic,
    ThreatDetection,
    EnvironmentalDanger,
    LeaderCommand
};

// Evacuation priority levels
UENUM(BlueprintType)
enum class ECrowd_EvacuationPriority : uint8
{
    Low = 0,
    Medium,
    High,
    Critical,
    Emergency
};

// Panic response data fragment
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PanicResponseFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    ECrowd_PanicState CurrentPanicState = ECrowd_PanicState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    ECrowd_PanicTrigger LastTrigger = ECrowd_PanicTrigger::PredatorSighting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    float PanicLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    float PanicDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    FVector EvacuationTarget = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    float TimeSincePanicStart = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    bool bIsEvacuating = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    ECrowd_EvacuationPriority EvacuationPriority = ECrowd_EvacuationPriority::Low;
};

// Evacuation zone data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EvacuationZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    FVector ZoneLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    float ZoneRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    int32 MaxCapacity = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    int32 CurrentOccupancy = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    ECrowd_EvacuationPriority Priority = ECrowd_EvacuationPriority::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    FString ZoneName = TEXT("EvacuationZone");
};

// Panic response processor
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_PanicResponseProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_PanicResponseProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Settings")
    float PanicSpreadRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Settings")
    float PanicSpreadChance = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Settings")
    float MaxPanicLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Settings")
    float PanicThreshold = 0.5f;

private:
    FMassEntityQuery EntityQuery;
    
    void ProcessPanicBehavior(FMassExecutionContext& Context);
    void UpdatePanicStates(FMassExecutionContext& Context);
    void HandleEvacuationLogic(FMassExecutionContext& Context);
    void SpreadPanic(FMassExecutionContext& Context, const FVector& PanicSource, float Intensity);
};

// Panic response manager component
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_PanicResponseComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_PanicResponseComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    TArray<FCrowd_EvacuationZone> EvacuationZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    float GlobalPanicLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    bool bPanicModeActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Response")
    float PanicCooldownTime = 30.0f;

    UFUNCTION(BlueprintCallable, Category = "Panic Response")
    void TriggerPanic(const FVector& TriggerLocation, ECrowd_PanicTrigger TriggerType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Panic Response")
    void AddEvacuationZone(const FCrowd_EvacuationZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Panic Response")
    FVector FindNearestEvacuationZone(const FVector& FromLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Panic Response")
    void CalmCrowds();

    UFUNCTION(BlueprintCallable, Category = "Panic Response")
    float GetGlobalPanicLevel() const { return GlobalPanicLevel; }

private:
    float TimeSinceLastPanic = 0.0f;
    
    void UpdateGlobalPanicLevel(float DeltaTime);
    void ProcessEvacuationZones();
    FCrowd_EvacuationZone* FindBestEvacuationZone(const FVector& FromLocation);
};

// Panic response subsystem
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowd_PanicResponseSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Panic Response")
    void RegisterPanicManager(UCrowd_PanicResponseComponent* Manager);

    UFUNCTION(BlueprintCallable, Category = "Panic Response")
    void UnregisterPanicManager(UCrowd_PanicResponseComponent* Manager);

    UFUNCTION(BlueprintCallable, Category = "Panic Response")
    void BroadcastPanicEvent(const FVector& Location, ECrowd_PanicTrigger TriggerType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Panic Response")
    float GetAveragePanicLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Panic Response")
    bool IsGlobalPanicActive() const;

protected:
    UPROPERTY()
    TArray<UCrowd_PanicResponseComponent*> RegisteredManagers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Settings")
    float GlobalPanicThreshold = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Settings")
    float PanicBroadcastRadius = 2000.0f;

private:
    void UpdateGlobalPanicState();
};

// Panic response actor
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_PanicResponseManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_PanicResponseManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCrowd_PanicResponseComponent* PanicResponseComponent;

public:
    UFUNCTION(BlueprintCallable, Category = "Panic Response")
    void InitializeEvacuationZones();

    UFUNCTION(BlueprintCallable, Category = "Panic Response")
    void HandleThreatDetection(const FVector& ThreatLocation, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Panic Response")
    void HandleCombatEvent(const FVector& CombatLocation, float CombatIntensity);

    UFUNCTION(BlueprintImplementableEvent, Category = "Panic Response")
    void OnPanicTriggered(const FVector& TriggerLocation, ECrowd_PanicTrigger TriggerType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Panic Response")
    void OnEvacuationStarted(const FVector& EvacuationTarget);

    UFUNCTION(BlueprintImplementableEvent, Category = "Panic Response")
    void OnCrowdCalmed();
};