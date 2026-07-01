#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DinosaurBase.generated.h"

// ── Enums (global scope, Eng_ prefix to avoid conflicts) ──────────────────────

UENUM(BlueprintType)
enum class EEng_DinoSpecies : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    TyrannosaurusRex UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pteranodon      UMETA(DisplayName = "Pteranodon"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
};

UENUM(BlueprintType)
enum class EEng_DinoAggressionState : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Curious     UMETA(DisplayName = "Curious"),
    Threatened  UMETA(DisplayName = "Threatened"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Territorial UMETA(DisplayName = "Territorial"),
};

UENUM(BlueprintType)
enum class EEng_DinoBehaviorMode : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Charging    UMETA(DisplayName = "Charging"),
    Resting     UMETA(DisplayName = "Resting"),
};

// ── Structs (global scope) ────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FEng_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MoveSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float SprintSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float TerritoryRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AggressionThreshold = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Mass = 500.0f;

    FEng_DinoStats() {}
};

USTRUCT(BlueprintType)
struct FEng_DinoSpeciesData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    EEng_DinoSpecies Species = EEng_DinoSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    FString CommonName = TEXT("Unknown Dinosaur");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    bool bIsCarnivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    bool bIsFlying = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    float PackDetectionBonus = 1.0f;

    FEng_DinoSpeciesData() {}
};

// ── ADinosaurBase ─────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Dinosaurs))
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Species & Stats ───────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EEng_DinoSpecies Species = EEng_DinoSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FEng_DinoSpeciesData SpeciesData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FEng_DinoStats Stats;

    // ── Behavior State ────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EEng_DinoAggressionState AggressionState = EEng_DinoAggressionState::Passive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EEng_DinoBehaviorMode BehaviorMode = EEng_DinoBehaviorMode::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    float AggressionLevel = 0.0f;

    // ── Territory ─────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    bool bHasTerritoryOverride = false;

    // ── Pack ──────────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Pack")
    TArray<ADinosaurBase*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Pack")
    int32 MaxPackSize = 1;

    // ── Target ────────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Combat")
    AActor* CurrentTarget = nullptr;

    // ── Public API ────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void TakeDinoHit(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetAggressionState(EEng_DinoAggressionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehaviorMode(EEng_DinoBehaviorMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Territory")
    bool IsWithinTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Pack")
    void AddPackMember(ADinosaurBase* Member);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Pack")
    void RemovePackMember(ADinosaurBase* Member);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(CallInEditor, Category = "Dinosaur|Debug")
    void DebugPrintStats();

protected:
    virtual void InitializeSpeciesData();
    void UpdateAggressionLevel(float DeltaTime);
    void ScanForThreats();
    void OnDeath();

private:
    float TimeSinceLastScan = 0.0f;
    static constexpr float ScanInterval = 0.5f;
    bool bIsDead = false;
};
