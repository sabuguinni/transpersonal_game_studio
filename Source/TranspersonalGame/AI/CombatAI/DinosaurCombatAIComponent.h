#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "DinosaurCombatAIComponent.generated.h"

// ============================================================
// Combat AI enums — unique prefix ECombatAI_ to avoid conflicts
// ============================================================

UENUM(BlueprintType)
enum class ECombatAI_TacticalState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Flanking        UMETA(DisplayName = "Flanking"),
    Charging        UMETA(DisplayName = "Charging"),
    Biting          UMETA(DisplayName = "Biting"),
    Retreating      UMETA(DisplayName = "Retreating"),
    PackCoordinating UMETA(DisplayName = "PackCoordinating"),
    Ambushing       UMETA(DisplayName = "Ambushing")
};

UENUM(BlueprintType)
enum class ECombatAI_PackRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha"),
    Flanker     UMETA(DisplayName = "Flanker"),
    Distractor  UMETA(DisplayName = "Distractor"),
    Ambusher    UMETA(DisplayName = "Ambusher"),
    Lone        UMETA(DisplayName = "Lone")
};

UENUM(BlueprintType)
enum class ECombatAI_DinoSpecies : uint8
{
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    TRex            UMETA(DisplayName = "T-Rex"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus")
};

UENUM(BlueprintType)
enum class ECombatAI_AttackType : uint8
{
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw"),
    Charge      UMETA(DisplayName = "Charge"),
    TailSwipe   UMETA(DisplayName = "TailSwipe"),
    Stomp       UMETA(DisplayName = "Stomp"),
    Pounce      UMETA(DisplayName = "Pounce"),
    Roar        UMETA(DisplayName = "Roar")
};

// ============================================================
// Combat AI structs
// ============================================================

USTRUCT(BlueprintType)
struct FCombatAI_AttackProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombatAI_AttackType AttackType = ECombatAI_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float BaseDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float Cooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float WindupTime = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float KnockbackForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bCanInterrupt = false;
};

USTRUCT(BlueprintType)
struct FCombatAI_ThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float ThreatScore = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float TimeSinceDetected = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bIsVisible = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bIsFleeing = false;
};

USTRUCT(BlueprintType)
struct FCombatAI_PackSignal
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    ECombatAI_PackRole SignalRole = ECombatAI_PackRole::Alpha;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    AActor* TargetActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float SignalTimestamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bIsAttackSignal = false;
};

// ============================================================
// UDinosaurCombatAIComponent
// ============================================================

UCLASS(ClassGroup = (CombatAI), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAIComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Species & Role ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Identity")
    ECombatAI_DinoSpecies Species = ECombatAI_DinoSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Identity")
    ECombatAI_PackRole PackRole = ECombatAI_PackRole::Lone;

    // --- Tactical State ---
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    ECombatAI_TacticalState CurrentTacticalState = ECombatAI_TacticalState::Idle;

    // --- Detection ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Detection")
    float DetectionRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Detection")
    float AggressionRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Detection")
    float AttackRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Detection")
    float FieldOfViewDegrees = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Detection")
    bool bCanDetectBySound = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Detection")
    float SoundDetectionRadius = 800.0f;

    // --- Combat Stats ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Stats")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float ChargeSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float StalkSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float StaminaMax = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Stats")
    float CurrentStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float StaminaDrainRate = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float StaminaRecoveryRate = 8.0f;

    // --- Attack Profiles ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attacks")
    TArray<FCombatAI_AttackProfile> AttackProfiles;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Attacks")
    float LastAttackTime = 0.0f;

    // --- Threat Memory ---
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Threats")
    TArray<FCombatAI_ThreatEntry> ThreatMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Threats")
    int32 MaxThreatMemorySize = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Threats")
    float ThreatMemoryFadeTime = 90.0f;

    // --- Pack Coordination ---
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Pack")
    AActor* PackAlpha = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Pack")
    FCombatAI_PackSignal LastPackSignal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    float PackCoordinationRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    bool bIsPackHunter = false;

    // --- Primary Target ---
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Target")
    AActor* PrimaryTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Target")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    // ============================================================
    // UFUNCTIONS — Combat Logic
    // ============================================================

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Detection")
    void ScanForThreats();

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Detection")
    bool IsActorInFieldOfView(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Detection")
    void RegisterThreat(AActor* ThreatActor, float InitialScore, bool bIsFleeing = false);

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Detection")
    void TickThreatMemory(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Detection")
    AActor* SelectPrimaryTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI|State")
    void EvaluateTacticalState();

    UFUNCTION(BlueprintCallable, Category = "Combat AI|State")
    void TransitionToState(ECombatAI_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Attacks")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Attacks")
    FCombatAI_AttackProfile SelectBestAttack(float DistanceToTarget) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Attacks")
    void ExecuteAttack(AActor* Target, const FCombatAI_AttackProfile& Attack);

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Attacks")
    float ApplyDamage(AActor* Target, float Damage, ECombatAI_AttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Pack")
    void BroadcastPackSignal(FCombatAI_PackSignal Signal);

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Pack")
    void ReceivePackSignal(FCombatAI_PackSignal Signal);

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Pack")
    void ScanForPackMembers();

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Pack")
    FVector ComputeFlankPosition(AActor* Target, bool bFlankLeft) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Pack")
    FVector ComputeAmbushPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Stats")
    void TakeDamage_Combat(float Damage);

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Stats")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Stats")
    void TickStamina(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Queries")
    bool HasActiveThreats() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Queries")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Queries")
    bool IsTargetFleeing() const;

    UFUNCTION(BlueprintPure, Category = "Combat AI|Queries")
    ECombatAI_TacticalState GetCurrentTacticalState() const { return CurrentTacticalState; }

    UFUNCTION(BlueprintPure, Category = "Combat AI|Queries")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Combat AI|Queries")
    float GetStaminaPercent() const;

private:
    float TickAccumulator = 0.0f;
    float ThreatScanInterval = 0.3f;
    float StateScanInterval = 0.5f;
    float LastStateScanTime = 0.0f;
    bool bIsCharging = false;
};
