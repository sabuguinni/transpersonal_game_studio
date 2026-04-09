// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTags.h"
#include "Engine/World.h"
#include "ThreatAssessmentComponent.generated.h"

UENUM(BlueprintType)
enum class EThreatType : uint8
{
    None,
    Player,
    Predator,
    Prey,
    Environmental,
    Unknown
};

UENUM(BlueprintType)
enum class EThreatResponse : uint8
{
    Ignore,
    Monitor,
    Investigate,
    Approach,
    Flee,
    Attack,
    CallForHelp
};

USTRUCT(BlueprintType)
struct FThreatData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite)
    EThreatType ThreatType = EThreatType::None;

    UPROPERTY(BlueprintReadWrite)
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float Distance = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    float FirstDetectedTime = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bIsVisible = false;

    UPROPERTY(BlueprintReadWrite)
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadWrite)
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    EThreatResponse RecommendedResponse = EThreatResponse::Monitor;

    FThreatData()
    {
        ThreatActor = nullptr;
        ThreatType = EThreatType::None;
        ThreatLevel = 0.0f;
        Distance = 0.0f;
        LastKnownLocation = FVector::ZeroVector;
        FirstDetectedTime = 0.0f;
        LastSeenTime = 0.0f;
        bIsVisible = false;
        bIsMoving = false;
        Velocity = FVector::ZeroVector;
        RecommendedResponse = EThreatResponse::Monitor;
    }
};

USTRUCT(BlueprintType)
struct FThreatAssessmentResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float OverallThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    AActor* PrimaryThreat = nullptr;

    UPROPERTY(BlueprintReadWrite)
    TArray<FThreatData> ActiveThreats;

    UPROPERTY(BlueprintReadWrite)
    EThreatResponse RecommendedAction = EThreatResponse::Monitor;

    UPROPERTY(BlueprintReadWrite)
    FVector SafeDirection = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    bool bShouldCallForHelp = false;

    FThreatAssessmentResult()
    {
        OverallThreatLevel = 0.0f;
        PrimaryThreat = nullptr;
        RecommendedAction = EThreatResponse::Monitor;
        SafeDirection = FVector::ZeroVector;
        bShouldCallForHelp = false;
    }
};

/**
 * Component that analyzes threats and determines appropriate responses
 * Implements intelligent threat assessment for combat AI
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UThreatAssessmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UThreatAssessmentComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Assessment Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float AssessmentRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float CriticalThreatThreshold = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float HighThreatThreshold = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float MediumThreatThreshold = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float LowThreatThreshold = 0.2f;

    // AI Personality
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float Courage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float Caution = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float SocialTendency = 0.5f;

    // Current Assessment
    UPROPERTY(BlueprintReadOnly, Category = "Threat Assessment")
    FThreatAssessmentResult CurrentAssessment;

    UPROPERTY(BlueprintReadOnly, Category = "Threat Assessment")
    TMap<AActor*, FThreatData> TrackedThreats;

public:
    // Main Assessment Functions
    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    void AssessThreats();

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    FThreatAssessmentResult GetThreatAssessment();

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    void AddThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    void RemoveThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    void UpdateThreat(AActor* ThreatActor);

    // Threat Analysis
    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    float CalculateThreatLevel(AActor* ThreatActor) const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    EThreatType DetermineThreatType(AActor* ThreatActor) const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    EThreatResponse DetermineResponse(const FThreatData& ThreatData) const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    bool IsActorThreatening(AActor* Actor) const;

    // Threat Queries
    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    AActor* GetHighestThreat() const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    TArray<AActor*> GetThreatsInRange(float Range) const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    TArray<AActor*> GetThreatsByType(EThreatType Type) const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    float GetOverallThreatLevel() const { return CurrentAssessment.OverallThreatLevel; }

    // Environmental Assessment
    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    FVector FindSafeDirection();

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    TArray<FVector> FindSafePositions(float SearchRadius, int32 MaxPositions = 5);

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    bool IsPositionSafe(FVector Position, float SafetyRadius = 500.0f) const;

    // Response Recommendations
    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    bool ShouldAttack() const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    bool ShouldCallForHelp() const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    bool ShouldHide() const;

    // Personality Modifiers
    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    void SetPersonality(float InCourage, float InAggression, float InCaution, float InSocialTendency);

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    float GetPersonalityModifier(EThreatResponse Response) const;

protected:
    // Internal Assessment Functions
    void UpdateTrackedThreats();
    void CalculateOverallThreat();
    void DetermineRecommendedAction();
    void CleanupOldThreats();

    // Threat Calculation Helpers
    float CalculateDistanceFactor(float Distance) const;
    float CalculateVisibilityFactor(bool bIsVisible, float TimeSinceLastSeen) const;
    float CalculateMovementFactor(const FVector& Velocity) const;
    float CalculateTypeFactor(EThreatType Type) const;
    float CalculatePersonalityFactor(EThreatResponse Response) const;

    // Environmental Helpers
    bool HasLineOfSight(FVector FromLocation, FVector ToLocation) const;
    float GetTerrainCover(FVector Position) const;
    TArray<FVector> SamplePositionsInRadius(FVector Center, float Radius, int32 NumSamples) const;

    // Timers
    FTimerHandle AssessmentTimer;
    FTimerHandle CleanupTimer;

    void OnAssessmentUpdate();
    void OnCleanupUpdate();

    // Cached Data
    float LastAssessmentTime = 0.0f;
    float AssessmentFrequency = 0.2f; // 5 times per second
};