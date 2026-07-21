#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "CombatAIManager.generated.h"

class UCombat_TacticalComponent;
class ACombat_DinosaurPawn;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector ThreatLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* ThreatActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float LastAssessmentTime;

    FCombat_ThreatAssessment()
    {
        ThreatLevel = 0.0f;
        ThreatLocation = FVector::ZeroVector;
        ThreatActor = nullptr;
        LastAssessmentTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* AlphaLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector FormationCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_FormationType FormationType;

    FCombat_PackFormation()
    {
        AlphaLeader = nullptr;
        FormationCenter = FVector::ZeroVector;
        FormationRadius = 500.0f;
        FormationType = ECombat_FormationType::Circle;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIManager : public AActor
{
    GENERATED_BODY()

public:
    ACombatAIManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TArray<FCombat_ThreatAssessment> ActiveThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TArray<FCombat_PackFormation> ActivePacks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ThreatScanRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ThreatScanInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float PackCoordinationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    int32 MaxPackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bEnablePackHunting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bEnableTacticalRetreat;

private:
    float LastThreatScanTime;
    float LastPackUpdateTime;

public:
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ScanForThreats();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdatePackFormations();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterCombatActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UnregisterCombatActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FCombat_ThreatAssessment AssessThreat(AActor* PotentialThreat, AActor* Observer);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget(AActor* Predator, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetOptimalFlankingPosition(AActor* Predator, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePackAttack(const FCombat_PackFormation& Pack, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitiateTacticalRetreat(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<AActor*> GetNearbyAllies(AActor* Actor, float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<AActor*> GetNearbyEnemies(AActor* Actor, float SearchRadius);
};