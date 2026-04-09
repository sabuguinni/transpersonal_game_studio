// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameplayTags.h"
#include "ThreatAssessmentComponent.generated.h"

UENUM(BlueprintType)
enum class EThreatType : uint8
{
    Unknown,
    Player,
    Predator,
    Prey,
    Neutral,
    Environmental,
    Territorial,
    Pack
};

UENUM(BlueprintType)
enum class EThreatResponse : uint8
{
    Ignore,
    Observe,
    Approach,
    Avoid,
    Flee,
    Investigate,
    Attack,
    Defend,
    CallForHelp
};

USTRUCT(BlueprintType)
struct FThreatData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    TWeakObjectPtr<AActor> ThreatActor;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    EThreatType ThreatType = EThreatType::Unknown;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    float Distance = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    float DetectionTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    float LastUpdateTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    bool bHasLineOfSight = false;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    bool bIsApproaching = false;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    bool bIsHostile = false;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    float HostilityLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    TArray<FGameplayTag> KnownCapabilities;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    EThreatResponse RecommendedResponse = EThreatResponse::Observe;

    FThreatData()
    {
        ThreatActor = nullptr;
        ThreatType = EThreatType::Unknown;
        ThreatLevel = 0.0f;
        Distance = 0.0f;
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        DetectionTime = 0.0f;
        LastUpdateTime = 0.0f;
        bHasLineOfSight = false;
        bIsMoving = false;
        bIsApproaching = false;
        bIsHostile = false;
        HostilityLevel = 0.0f;
        RecommendedResponse = EThreatResponse::Observe;
    }
};

USTRUCT(BlueprintType)
struct FThreatAssessmentConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float MaxDetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float CriticalThreatRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float HighThreatRange = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float MediumThreatRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float ThreatDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float MemoryDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float UpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    bool bUseLineOfSightChecks = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    bool bConsiderMovement = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    bool bConsiderSize = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    bool bConsiderPreviousExperience = true;

    FThreatAssessmentConfig()
    {
        MaxDetectionRange = 2000.0f;
        CriticalThreatRange = 300.0f;
        HighThreatRange = 600.0f;
        MediumThreatRange = 1000.0f;
        ThreatDecayRate = 0.1f;
        MemoryDuration = 30.0f;
        UpdateInterval = 0.5f;
        bUseLineOfSightChecks = true;
        bConsiderMovement = true;
        bConsiderSize = true;
        bConsiderPreviousExperience = true;
    }
};

/**
 * Threat Assessment Component for advanced AI threat evaluation and response
 * Analyzes multiple threat factors and recommends appropriate responses
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UThreatAssessmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UThreatAssessmentComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Threat Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    FThreatAssessmentConfig Config;

    // Current Threats
    UPROPERTY(BlueprintReadOnly, Category = "Threat Assessment")
    TMap<AActor*, FThreatData> ActiveThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Threat Assessment")
    TArray<FThreatData> ThreatHistory;

    // Assessment Results
    UPROPERTY(BlueprintReadOnly, Category = "Threat Assessment")
    AActor* PrimaryThreat;

    UPROPERTY(BlueprintReadOnly, Category = "Threat Assessment")
    float OverallThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Threat Assessment")
    EThreatResponse CurrentRecommendedResponse = EThreatResponse::Observe;

    UPROPERTY(BlueprintReadOnly, Category = "Threat Assessment")
    bool bInDanger = false;

    UPROPERTY(BlueprintReadOnly, Category = "Threat Assessment")
    bool bShouldFlee = false;

    UPROPERTY(BlueprintReadOnly, Category = "Threat Assessment")
    bool bShouldAttack = false;

public:
    // Threat Detection and Registration
    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    void RegisterThreat(AActor* ThreatActor, EThreatType ThreatType = EThreatType::Unknown);

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    void UnregisterThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    void UpdateThreatData(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    void ClearAllThreats();

    // Threat Analysis
    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    bool HasActiveThreats() const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    int32 GetActiveThreatCount() const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    AActor* GetHighestThreat() const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    float GetThreatLevel(AActor* ThreatActor) const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    FThreatData GetThreatData(AActor* ThreatActor) const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    TArray<AActor*> GetThreatsInRange(float Range) const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    TArray<AActor*> GetThreatsByType(EThreatType ThreatType) const;

    // Threat Response Recommendations
    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    EThreatResponse EvaluateBestResponse();

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    EThreatResponse GetRecommendedResponse(AActor* ThreatActor) const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    bool ShouldEngageThreat(AActor* ThreatActor) const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    bool ShouldAvoidThreat(AActor* ThreatActor) const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    bool ShouldFleeFromThreat(AActor* ThreatActor) const;

    // Situational Assessment
    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    bool IsInCombatSituation() const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    bool IsOutnumbered() const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    bool HasEscapeRoute() const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    FVector GetSafestDirection() const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    FVector GetBestAttackPosition(AActor* Target) const;

    // Threat Prediction
    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    FVector PredictThreatPosition(AActor* ThreatActor, float PredictionTime) const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    bool WillThreatReachLocation(AActor* ThreatActor, FVector Location, float TimeLimit) const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    float GetTimeToThreatContact(AActor* ThreatActor) const;

    // Environmental Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    void AssessEnvironmentalThreats();

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    bool IsLocationSafe(FVector Location, float SafetyRadius = 500.0f) const;

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    float GetLocationThreatLevel(FVector Location) const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    void SetThreatAssessmentConfig(const FThreatAssessmentConfig& NewConfig);

    UFUNCTION(BlueprintPure, Category = "Threat Assessment")
    FThreatAssessmentConfig GetThreatAssessmentConfig() const { return Config; }

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnThreatDetected, AActor*, ThreatActor, float, ThreatLevel);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreatLost, AActor*, ThreatActor);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreatLevelChanged, float, NewThreatLevel);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecommendedResponseChanged, EThreatResponse, NewResponse);

    UPROPERTY(BlueprintAssignable, Category = "Threat Assessment")
    FOnThreatDetected OnThreatDetected;

    UPROPERTY(BlueprintAssignable, Category = "Threat Assessment")
    FOnThreatLost OnThreatLost;

    UPROPERTY(BlueprintAssignable, Category = "Threat Assessment")
    FOnThreatLevelChanged OnThreatLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "Threat Assessment")
    FOnRecommendedResponseChanged OnRecommendedResponseChanged;

    // Debugging
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawThreatAssessment() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugPrintThreatData() const;

protected:
    // Internal Assessment Logic
    void UpdateAllThreats(float DeltaTime);
    void CalculateOverallThreatLevel();
    void DetermineRecommendedResponse();
    void CleanupExpiredThreats();

    // Threat Calculation Methods
    float CalculateThreatLevel(const FThreatData& ThreatData) const;
    float CalculateDistanceFactor(float Distance) const;
    float CalculateMovementFactor(const FThreatData& ThreatData) const;
    float CalculateSizeFactor(AActor* ThreatActor) const;
    float CalculateExperienceFactor(AActor* ThreatActor) const;
    float CalculateHostilityFactor(const FThreatData& ThreatData) const;

    // Response Evaluation
    EThreatResponse EvaluateResponseToThreat(const FThreatData& ThreatData) const;
    bool CanEngageThreat(const FThreatData& ThreatData) const;
    bool ShouldRetreatFromThreat(const FThreatData& ThreatData) const;

    // Utility Functions
    bool HasLineOfSight(AActor* Target) const;
    FVector GetOwnerLocation() const;
    float GetOwnerRadius() const;
    bool IsActorMoving(AActor* Actor) const;
    FVector GetActorVelocity(AActor* Actor) const;

    // Timers
    float UpdateTimer = 0.0f;
    float CleanupTimer = 0.0f;
    float EnvironmentalAssessmentTimer = 0.0f;

    // Cached References
    class AAIController* OwnerController;
    class APawn* OwnerPawn;
    class UWorld* CachedWorld;
    class UCombatMemoryComponent* CombatMemory;

    // Previous state tracking
    float PreviousOverallThreatLevel = 0.0f;
    EThreatResponse PreviousRecommendedResponse = EThreatResponse::Observe;
};