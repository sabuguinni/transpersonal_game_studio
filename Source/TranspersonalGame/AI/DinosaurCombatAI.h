#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinoThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Charging    UMETA(DisplayName = "Charging"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Retreating  UMETA(DisplayName = "Retreating")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl UMETA(DisplayName = "Pterodactyl")
};

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BaseDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Cooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float KnockbackForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanInterrupt = true;
};

USTRUCT(BlueprintType)
struct FCombat_DinoAIState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    ECombat_DinoThreatLevel ThreatLevel = ECombat_DinoThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float AggroRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    bool bIsPackMember = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float LastAttackTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    bool bPlayerDetected = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;
};

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void DetectPlayer(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TakeDamage_Dino(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetPackCoordination(bool bEnabled, int32 PackMemberCount);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void Retreat(FVector SafeLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetThreatScore() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_DinoThreatLevel GetCurrentThreatLevel() const { return AIState.ThreatLevel; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_DinoAIState AIState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_AttackData PrimaryAttack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FleeHealthThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsAlpha = false;

private:
    UPROPERTY()
    AActor* CurrentTarget = nullptr;

    float TimeSinceLastDetectionCheck = 0.0f;
    float DetectionCheckInterval = 0.5f;

    void UpdateThreatLevel();
    void PerformDetectionSweep();
    void HandlePackBehavior();
    bool CanAttack() const;
};
