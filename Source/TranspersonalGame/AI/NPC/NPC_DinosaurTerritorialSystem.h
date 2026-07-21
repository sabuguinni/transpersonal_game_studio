#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "NPC_DinosaurTerritorialSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_TerritoryBounds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float PatrolRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float DefenseRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    bool bIsActiveTerritory = true;

    FNPC_TerritoryBounds()
    {
        CenterLocation = FVector::ZeroVector;
        TerritoryRadius = 5000.0f;
        PatrolRadius = 2500.0f;
        DefenseRadius = 1000.0f;
        bIsActiveTerritory = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_TerritorialThreat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    FVector ThreatLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float LastSeenTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    bool bIsActiveThreat = false;

    FNPC_TerritorialThreat()
    {
        ThreatActor = nullptr;
        ThreatLocation = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        LastSeenTime = 0.0f;
        bIsActiveThreat = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurTerritorialSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurTerritorialSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Territory Management
    UFUNCTION(BlueprintCallable, Category = "Territory")
    void SetTerritoryCenter(const FVector& NewCenter);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void SetTerritoryRadius(float NewRadius);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsLocationInTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsLocationInPatrolArea(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsLocationInDefenseZone(const FVector& Location) const;

    // Threat Detection
    UFUNCTION(BlueprintCallable, Category = "Territory")
    void DetectTerritorialThreats();

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void AddThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void RemoveThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    FNPC_TerritorialThreat GetHighestThreat() const;

    // Territory Defense
    UFUNCTION(BlueprintCallable, Category = "Territory")
    FVector GetDefensePosition(const FVector& ThreatLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool ShouldDefendTerritory() const;

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void StartTerritorialDefense();

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void StopTerritorialDefense();

    // Patrol Behavior
    UFUNCTION(BlueprintCallable, Category = "Territory")
    FVector GetNextPatrolPoint() const;

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void UpdatePatrolRoute();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    FNPC_TerritoryBounds TerritoryBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_TerritorialThreat> ActiveThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    bool bIsDefendingTerritory = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    float ThreatDetectionRadius = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    float ThreatUpdateInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    float LastThreatUpdate = 0.0f;

private:
    void GeneratePatrolPoints();
    void CleanupOldThreats();
    float CalculateThreatLevel(AActor* ThreatActor) const;
};