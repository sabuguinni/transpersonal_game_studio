#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "Combat_CombatManager.generated.h"

class UCombat_TacticalAI;
class UCombat_DinosaurAI;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_CombatZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    TArray<AActor*> ActiveCombatants;

    FCombat_CombatZone()
    {
        Center = FVector::ZeroVector;
        Radius = 1000.0f;
        ThreatLevel = ECombat_ThreatLevel::Low;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_CombatManager : public AActor
{
    GENERATED_BODY()

public:
    ACombat_CombatManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zones")
    TArray<FCombat_CombatZone> CombatZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float CombatUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    int32 MaxSimultaneousCombats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    TArray<AActor*> ActiveCombatants;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    int32 CurrentCombatCount;

private:
    float LastCombatUpdate;
    TArray<AActor*> PendingCombatants;

public:
    UFUNCTION(BlueprintCallable, Category = "Combat Management")
    void RegisterCombatant(AActor* Combatant);

    UFUNCTION(BlueprintCallable, Category = "Combat Management")
    void UnregisterCombatant(AActor* Combatant);

    UFUNCTION(BlueprintCallable, Category = "Combat Management")
    void StartCombat(AActor* Attacker, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Management")
    void EndCombat(AActor* Combatant);

    UFUNCTION(BlueprintCallable, Category = "Combat Management")
    bool IsInCombatZone(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Management")
    ECombat_ThreatLevel GetThreatLevelAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Management")
    TArray<AActor*> GetNearbyCombatants(const FVector& Location, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Management")
    void UpdateCombatZones();

    UFUNCTION(BlueprintCallable, Category = "Combat Management")
    void ProcessCombatQueue();

    UFUNCTION(BlueprintCallable, Category = "Combat Management")
    bool CanStartNewCombat() const;

    UFUNCTION(BlueprintPure, Category = "Combat Management")
    int32 GetActiveCombatCount() const { return CurrentCombatCount; }

    UFUNCTION(BlueprintPure, Category = "Combat Management")
    bool IsActorInCombat(AActor* Actor) const;
};