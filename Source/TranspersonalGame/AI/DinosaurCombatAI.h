#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Charging    UMETA(DisplayName = "Charging"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Feeding     UMETA(DisplayName = "Feeding"),
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    TyrannosaurusRex UMETA(DisplayName = "Tyrannosaurus Rex"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
};

USTRUCT(BlueprintType)
struct FCombat_DinoTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float AttackRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float FleeHealthThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    bool bIsAmbushPredator = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float MovementSpeedBase = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float MovementSpeedCharge = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float AttackDamageBase = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float StalkingRadius = 800.0f;
};

USTRUCT(BlueprintType)
struct FCombat_PackFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    TArray<FVector> FlankingOffsets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    int32 PackSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float FlankingAngleDeg = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bAlphaLeads = true;
};

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurCombatAI : public AActor
{
    GENERATED_BODY()

public:
    ADinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Species and traits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Identity")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Identity")
    FCombat_DinoTraits Traits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    FCombat_PackFormation PackFormation;

    // Current AI state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|State")
    float MaxHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    bool bIsAlpha = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    TArray<ADinosaurCombatAI*> PackMembers;

    // Combat functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void DetectPlayer(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteFlankingManeuver(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAmbushCharge(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void NotifyPackMembers(ECombat_DinoState AlertState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsPlayerInDetectionRange(AActor* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsPlayerInAttackRange(AActor* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitializePackHunterTraits();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitializeAmbushPredatorTraits();

protected:
    void TickIdle(float DeltaTime);
    void TickStalking(float DeltaTime);
    void TickFlanking(float DeltaTime);
    void TickCharging(float DeltaTime);
    void TickAttacking(float DeltaTime);

    FVector GetFlankingPosition(int32 MemberIndex, AActor* Target) const;
    bool ShouldFlee() const;

private:
    float StateTimer = 0.0f;
    FVector PatrolOrigin;
    FVector LastKnownTargetLocation;
};
