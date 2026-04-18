#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "CombatAIManager.generated.h"

class AEnemyAIController;
class APawn;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalPosition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bIsFlankingPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bHasCover;

    FCombat_TacticalPosition()
    {
        Position = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        bIsFlankingPosition = false;
        bHasCover = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FCombat_TacticalPosition> Positions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector CenterPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ECombat_FormationType FormationType;

    FCombat_PackFormation()
    {
        CenterPoint = FVector::ZeroVector;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TArray<AEnemyAIController*> RegisteredEnemies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    APawn* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_PackFormation CurrentFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float TacticalUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float MaxEngagementRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bUsePackTactics;

    FTimerHandle TacticalUpdateTimer;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterEnemy(AEnemyAIController* Enemy);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UnregisterEnemy(AEnemyAIController* Enemy);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(APawn* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTacticalPositions();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FCombat_TacticalPosition CalculateFlankingPosition(const FVector& TargetLocation, int32 FlankIndex);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget(APawn* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecutePackTactic(ECombat_FormationType TacticType);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<FCombat_TacticalPosition> GenerateFormationPositions(const FVector& CenterPoint, ECombat_FormationType FormationType, int32 NumPositions);

protected:
    UFUNCTION()
    void OnTacticalUpdate();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float CalculateThreatLevel(const FVector& Position, APawn* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool HasLineOfSight(const FVector& FromPosition, const FVector& ToPosition) const;
};