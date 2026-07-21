#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Combat_BattleManager.generated.h"

class UCombat_DinosaurCombatAI;
class ATranspersonalCharacter;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_BattleState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 ActiveEnemies = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float LastCombatTime = 0.0f;

    FCombat_BattleState()
    {
        bInCombat = false;
        CombatIntensity = 0.0f;
        ActiveEnemies = 0;
        LastCombatTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_BattleManager : public AActor
{
    GENERATED_BODY()

public:
    ACombat_BattleManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Combat State Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    FCombat_BattleState CurrentBattleState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Detection")
    USphereComponent* CombatDetectionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Detection")
    float CombatDetectionRadius = 2000.0f;

    // Combat Management Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartCombat(AActor* Enemy, AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EndCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatIntensity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsInCombat() const { return CurrentBattleState.bInCombat; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetCombatIntensity() const { return CurrentBattleState.CombatIntensity; }

    // Enemy Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterEnemy(AActor* Enemy);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UnregisterEnemy(AActor* Enemy);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    TArray<AActor*> GetActiveEnemies() const { return ActiveEnemies; }

    // Tactical AI Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CoordinateEnemyAttacks();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AssignTargetPriorities();

protected:
    // Internal state tracking
    UPROPERTY()
    TArray<AActor*> ActiveEnemies;

    UPROPERTY()
    AActor* PlayerCharacter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tuning")
    float CombatTimeoutDuration = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tuning")
    float IntensityDecayRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tuning")
    float MaxCombatIntensity = 1.0f;

    // Detection events
    UFUNCTION()
    void OnCombatDetectionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnCombatDetectionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    void FindPlayerCharacter();
    void UpdateEnemyStates(float DeltaTime);
    bool IsValidCombatant(AActor* Actor) const;
};