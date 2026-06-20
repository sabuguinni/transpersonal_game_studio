#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinoThreatState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Stalk       UMETA(DisplayName = "Stalk"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Retreat     UMETA(DisplayName = "Retreat"),
    Feeding     UMETA(DisplayName = "Feeding")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Pterodactyl UMETA(DisplayName = "Pterodactyl")
};

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ChaseSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Health = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float StalkDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PackSize = 1;
};

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    FCombat_DinoStats Stats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    ECombat_DinoThreatState CurrentThreatState = ECombat_DinoThreatState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float DistanceToPlayer = 99999.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    bool bPlayerDetected = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float StalkTimer = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetThreatState(ECombat_DinoThreatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_DinoThreatState EvaluateThreat(float PlayerDistance, bool bPlayerVisible);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetAttackDamage() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsHostile() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnPlayerSpotted(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnPlayerLost();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage_Combat(float DamageAmount);

private:
    UPROPERTY()
    AActor* TrackedPlayer = nullptr;

    float StateTimer = 0.0f;

    void UpdateStateLogic(float DeltaTime);
    void TransitionToState(ECombat_DinoThreatState NewState);
};
