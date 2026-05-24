#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "SurvivalInstinctComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_SurvivalState : uint8
{
    Calm UMETA(DisplayName = "Calm"),
    Alert UMETA(DisplayName = "Alert"), 
    Fleeing UMETA(DisplayName = "Fleeing"),
    Hiding UMETA(DisplayName = "Hiding"),
    Foraging UMETA(DisplayName = "Foraging"),
    Resting UMETA(DisplayName = "Resting")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None UMETA(DisplayName = "None"),
    Low UMETA(DisplayName = "Low"),
    Medium UMETA(DisplayName = "Medium"),
    High UMETA(DisplayName = "High"),
    Extreme UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct FNPC_ThreatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    AActor* ThreatActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    ENPC_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float LastSeenTime;

    FNPC_ThreatData()
    {
        ThreatActor = nullptr;
        ThreatLevel = ENPC_ThreatLevel::None;
        Distance = 0.0f;
        LastSeenTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_SurvivalInstinctComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_SurvivalInstinctComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core survival state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival State")
    ENPC_SurvivalState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival State")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival State")
    float HungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival State")
    float ThirstLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival State")
    float EnergyLevel;

    // Threat detection settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float ThreatDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float DinosaurThreatMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float PlayerThreatLevel;

    // Behavior settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float RestDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ForageDuration;

    // Current threats
    UPROPERTY(BlueprintReadOnly, Category = "Threat Data")
    TArray<FNPC_ThreatData> DetectedThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Threat Data")
    AActor* PrimaryThreat;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "Survival Instinct")
    void UpdateSurvivalState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Survival Instinct")
    void DetectThreats();

    UFUNCTION(BlueprintCallable, Category = "Survival Instinct")
    void ProcessThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Survival Instinct")
    void StartFleeing(AActor* ThreatSource);

    UFUNCTION(BlueprintCallable, Category = "Survival Instinct")
    void StartForaging();

    UFUNCTION(BlueprintCallable, Category = "Survival Instinct")
    void StartResting();

    UFUNCTION(BlueprintCallable, Category = "Survival Instinct")
    bool IsThreateningActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Survival Instinct")
    FVector GetFleeDirection(AActor* ThreatSource);

    UFUNCTION(BlueprintCallable, Category = "Survival Instinct")
    void UpdateNeedsOverTime(float DeltaTime);

private:
    float StateChangeTimer;
    FVector LastKnownSafeLocation;
    float LastThreatCheckTime;
    bool bIsInitialized;
};

#include "SurvivalInstinctComponent.generated.h"