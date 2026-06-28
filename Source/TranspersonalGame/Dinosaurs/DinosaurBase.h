// DinosaurBase.h
// Engine Architect #02 — Transpersonal Game Studio
// Base class for all dinosaur species in the game.
// Provides: species data, survival stats, patrol AI, threat response, damage/death.
// All dinosaur species (TRex, Raptor, Brachiosaurus, etc.) inherit from this.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DinosaurBase.generated.h"

// ─── ENUMS ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EDinosaurDiet : uint8
{
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Omnivore    UMETA(DisplayName = "Omnivore"),
    Piscivore   UMETA(DisplayName = "Piscivore")
};

UENUM(BlueprintType)
enum class EDinosaurSocial : uint8
{
    Solitary    UMETA(DisplayName = "Solitary"),
    Pair        UMETA(DisplayName = "Pair"),
    SmallPack   UMETA(DisplayName = "Small Pack (3-6)"),
    LargePack   UMETA(DisplayName = "Large Pack (7+)"),
    Herd        UMETA(DisplayName = "Herd")
};

UENUM(BlueprintType)
enum class EDinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Dead        UMETA(DisplayName = "Dead")
};

// ─── SPECIES DATA STRUCT ──────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FDinosaurSpeciesData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FName SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float MoveSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float AggroRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float AttackRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EDinosaurDiet Diet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EDinosaurSocial SocialBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsNocturnal;

    FDinosaurSpeciesData()
        : SpeciesName(NAME_None)
        , MaxHealth(200.f)
        , MoveSpeed(400.f)
        , AttackDamage(30.f)
        , AggroRadius(1500.f)
        , AttackRadius(200.f)
        , Diet(EDinosaurDiet::Herbivore)
        , SocialBehavior(EDinosaurSocial::Solitary)
        , bIsNocturnal(false)
    {}
};

// ─── DINOSAUR BASE CLASS ──────────────────────────────────────────────────────

UCLASS(Abstract, BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ─── SPECIES DATA ─────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    FDinosaurSpeciesData SpeciesData;

    // ─── RUNTIME STATE ────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    EDinosaurState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    bool bIsAlive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHunger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentThirst;

    // ─── PATROL ───────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    float PatrolWaitTime;

    // ─── THREAT ───────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    AActor* ThreatActor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    FVector LastKnownThreatLocation;

    // ─── COMPONENTS ───────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* PerceptionSphere;

    // ─── PUBLIC INTERFACE ─────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void ApplyDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|State")
    void SetState(EDinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void MoveToLocation(const FVector& Destination);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual void PerformAttack();

    // ─── BLUEPRINT EVENTS ─────────────────────────────────────────────────────

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDamageReceived(float DamageAmount, AActor* DamageSource);
    virtual void OnDamageReceived_Implementation(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDeath(AActor* Killer);
    virtual void OnDeath_Implementation(AActor* Killer);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnStateChanged(EDinosaurState OldState, EDinosaurState NewState);
    virtual void OnStateChanged_Implementation(EDinosaurState OldState, EDinosaurState NewState);

private:
    // ─── INTERNAL STATE ───────────────────────────────────────────────────────

    FVector HomeLocation;
    FVector CurrentPatrolTarget;
    bool bIsWaiting;

    FTimerHandle PatrolTimerHandle;
    FTimerHandle HungerTimerHandle;

    // ─── TICK HELPERS ─────────────────────────────────────────────────────────

    void TickPatrol(float DeltaTime);
    void TickChase(float DeltaTime);
    void TickFlee(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickSurvivalNeeds();

    void ChooseNextPatrolPoint();
    void OnPatrolWaitComplete();
    void Die(AActor* Killer);

    // ─── PERCEPTION CALLBACKS ─────────────────────────────────────────────────

    UFUNCTION()
    void OnPerceptionOverlapBegin(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnPerceptionOverlapEnd(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);
};
