#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "Combat_TacticalAIManager.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol      UMETA(DisplayName = "Patrol"),
    Hunt        UMETA(DisplayName = "Hunt"),
    Ambush      UMETA(DisplayName = "Ambush"),
    Flee        UMETA(DisplayName = "Flee"),
    Defend      UMETA(DisplayName = "Defend"),
    Regroup     UMETA(DisplayName = "Regroup")
};

USTRUCT(BlueprintType)
struct FCombat_TacticalZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Zone")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Zone")
    ECombat_TacticalState PreferredState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Zone")
    int32 MaxOccupants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Zone")
    float ThreatLevel;

    FCombat_TacticalZone()
    {
        Position = FVector::ZeroVector;
        Radius = 1000.0f;
        PreferredState = ECombat_TacticalState::Patrol;
        MaxOccupants = 5;
        ThreatLevel = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_TacticalAIManager : public AActor
{
    GENERATED_BODY()

public:
    ACombat_TacticalAIManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Tactical zone management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Zones")
    TArray<FCombat_TacticalZone> TacticalZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float TacticalUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float MaxEngagementRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    int32 MaxActiveHunters;

    // Combat state management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_TacticalState GetOptimalTacticalState(const FVector& Position, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetBestAmbushPosition(const FVector& TargetPosition);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanEngageTarget(const FVector& HunterPosition, const FVector& TargetPosition);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterCombatant(AActor* Combatant, bool bIsHostile);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTacticalSituation();

private:
    UPROPERTY()
    TArray<AActor*> ActiveCombatants;

    UPROPERTY()
    TArray<AActor*> HostileTargets;

    float LastTacticalUpdate;

    void InitializeTacticalZones();
    void ProcessCombatDecisions();
    FCombat_TacticalZone* FindNearestTacticalZone(const FVector& Position);
};