#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "../SharedTypes.h"
#include "Combat_TerritorialAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TerritorialBehavior : uint8
{
    Passive UMETA(DisplayName = "Passive"),
    Defensive UMETA(DisplayName = "Defensive"), 
    Aggressive UMETA(DisplayName = "Aggressive"),
    Territorial UMETA(DisplayName = "Territorial"),
    Hunting UMETA(DisplayName = "Hunting")
};

USTRUCT(BlueprintType)
struct FCombat_TerritoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    ECombat_TerritorialBehavior BehaviorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    int32 ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    bool bIsActiveTerritory;

    FCombat_TerritoryData()
    {
        CenterLocation = FVector::ZeroVector;
        TerritoryRadius = 500.0f;
        BehaviorType = ECombat_TerritorialBehavior::Passive;
        ThreatLevel = 1;
        PatrolRadius = 300.0f;
        bIsActiveTerritory = true;
    }
};

USTRUCT(BlueprintType)
struct FCombat_IntruderData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intruder")
    AActor* IntruderActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intruder")
    float DistanceToTerritory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intruder")
    float ThreatAssessment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intruder")
    float DetectionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intruder")
    bool bIsHostile;

    FCombat_IntruderData()
    {
        IntruderActor = nullptr;
        DistanceToTerritory = 0.0f;
        ThreatAssessment = 0.0f;
        DetectionTime = 0.0f;
        bIsHostile = false;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TerritorialAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TerritorialAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Territory Management
    UFUNCTION(BlueprintCallable, Category = "Territorial AI")
    void SetTerritoryData(const FCombat_TerritoryData& NewTerritoryData);

    UFUNCTION(BlueprintCallable, Category = "Territorial AI")
    FCombat_TerritoryData GetTerritoryData() const { return TerritoryData; }

    UFUNCTION(BlueprintCallable, Category = "Territorial AI")
    bool IsLocationInTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Territorial AI")
    float GetDistanceToTerritoryCenter(const FVector& Location) const;

    // Intruder Detection
    UFUNCTION(BlueprintCallable, Category = "Territorial AI")
    void DetectIntruders();

    UFUNCTION(BlueprintCallable, Category = "Territorial AI")
    void AddIntruder(AActor* IntruderActor);

    UFUNCTION(BlueprintCallable, Category = "Territorial AI")
    void RemoveIntruder(AActor* IntruderActor);

    UFUNCTION(BlueprintCallable, Category = "Territorial AI")
    TArray<FCombat_IntruderData> GetCurrentIntruders() const { return CurrentIntruders; }

    // Behavioral Responses
    UFUNCTION(BlueprintCallable, Category = "Territorial AI")
    void RespondToIntruder(const FCombat_IntruderData& IntruderData);

    UFUNCTION(BlueprintCallable, Category = "Territorial AI")
    void ExecuteTerritorialBehavior();

    UFUNCTION(BlueprintCallable, Category = "Territorial AI")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Territorial AI")
    void StopPatrolling();

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Territorial AI")
    float AssessThreatLevel(AActor* PotentialThreat) const;

    UFUNCTION(BlueprintCallable, Category = "Territorial AI")
    bool ShouldEngageTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Territorial AI")
    ECombat_TerritorialBehavior GetCurrentBehavior() const { return TerritoryData.BehaviorType; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    FCombat_TerritoryData TerritoryData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_IntruderData> CurrentIntruders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    float DetectionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    float IntruderTimeoutDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (AllowPrivateAccess = "true"))
    bool bIsPatrolling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (AllowPrivateAccess = "true"))
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (AllowPrivateAccess = "true"))
    FVector CurrentPatrolTarget;

private:
    float LastDetectionUpdate;
    float PatrolTimer;
    
    void UpdatePatrolTarget();
    void CleanupExpiredIntruders();
    FVector GetRandomPatrolPoint() const;
    bool IsValidPatrolPoint(const FVector& Point) const;
};