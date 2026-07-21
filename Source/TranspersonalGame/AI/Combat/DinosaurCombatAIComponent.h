// DinosaurCombatAIComponent.h
// Agent #12 — Combat & Enemy AI Agent
// PROD_CYCLE_AUTO_20260628_007
// Tactical combat AI for dinosaur enemies — threat assessment, attack patterns, territorial behavior

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "DinosaurCombatAIComponent.generated.h"

// ============================================================
// ENUMS — Global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Curious     UMETA(DisplayName = "Curious"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Charging    UMETA(DisplayName = "Charging"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpeciesType : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "T-Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Carnotaurus         UMETA(DisplayName = "Carnotaurus"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite        UMETA(DisplayName = "Bite"),
    Charge      UMETA(DisplayName = "Charge"),
    TailSwipe   UMETA(DisplayName = "Tail Swipe"),
    Stomp       UMETA(DisplayName = "Stomp"),
    Pounce      UMETA(DisplayName = "Pounce"),
    Headbutt    UMETA(DisplayName = "Headbutt"),
    Roar        UMETA(DisplayName = "Roar")
};

// ============================================================
// STRUCTS — Global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float BaseDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float Cooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float WindupTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float KnockbackForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    bool bCanInterrupt = false;
};

USTRUCT(BlueprintType)
struct FCombat_DinoSpeciesProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_DinoSpeciesType SpeciesType = ECombat_DinoSpeciesType::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float AggroRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float TerritorialRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float MaxHealth = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float ChargeSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    bool bUsesMotionDetection = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    bool bIsTerritorial = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    TArray<FCombat_AttackData> AvailableAttacks;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    AActor* ThreatTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    float ThreatScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    float DistanceToTarget = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    bool bTargetIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    bool bTargetIsVisible = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    float LastKnownTargetTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    FVector LastKnownTargetLocation = FVector::ZeroVector;
};

// ============================================================
// MAIN COMPONENT CLASS
// ============================================================

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAIComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ============================================================
    // SPECIES CONFIGURATION
    // ============================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    FCombat_DinoSpeciesProfile SpeciesProfile;

    // ============================================================
    // COMBAT STATE
    // ============================================================

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoThreatLevel CurrentThreatLevel = ECombat_DinoThreatLevel::None;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    FCombat_ThreatAssessment CurrentThreat;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float CurrentHealth = 200.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    bool bIsInCombat = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    bool bIsCharging = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float AttackCooldownRemaining = 0.0f;

    // ============================================================
    // PACK COORDINATION
    // ============================================================

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    TArray<UDinosaurCombatAIComponent*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsPackLeader = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    int32 PackFlankingRole = 0; // 0=center, 1=left, 2=right

    // ============================================================
    // CORE FUNCTIONS
    // ============================================================

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitializeSpeciesProfile(ECombat_DinoSpeciesType Species);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ScanForThreats();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float AssessThreat(AActor* PotentialTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateThreatLevel();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(const FCombat_AttackData& Attack);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SelectBestAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitiateCharge(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CoordinatePackAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AssignFlankingRoles();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetFlankingPosition(int32 FlankRole, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage_Combat(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnDeath();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsTargetVisible(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsTargetMoving(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AlertPackMembers(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterPackMember(UDinosaurCombatAIComponent* Member);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_DinoThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsInCombat() const { return bIsInCombat; }

    // ============================================================
    // DELEGATES
    // ============================================================

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_OnAttackExecuted, ECombat_AttackType, AttackType, float, Damage);
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnAttackExecuted OnAttackExecuted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_OnThreatLevelChanged, ECombat_DinoThreatLevel, NewLevel);
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnThreatLevelChanged OnThreatLevelChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCombat_OnDinoDefeated);
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnDinoDefeated OnDinoDefeated;

private:
    float ScanInterval = 0.25f;
    float ScanTimer = 0.0f;
    float LastAttackTime = 0.0f;
    FVector LastKnownPlayerVelocity = FVector::ZeroVector;
    FVector PreviousTargetLocation = FVector::ZeroVector;

    void TickCombatLogic(float DeltaTime);
    void TickPackCoordination(float DeltaTime);
    FCombat_AttackData BuildDefaultAttack(ECombat_AttackType Type) const;
};
