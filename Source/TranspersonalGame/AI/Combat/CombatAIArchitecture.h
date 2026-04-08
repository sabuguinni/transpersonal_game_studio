#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameplayTags.h"
#include "CombatAIArchitecture.generated.h"

/**
 * Core Combat AI Architecture for Jurassic Survival Game
 * Handles tactical combat behavior for dinosaurs and hostile entities
 * 
 * Design Principles:
 * - Player is always prey, never predator
 * - Each dinosaur has unique behavior patterns
 * - Combat should create constant tension
 * - AI should feel intelligent but fair
 */

UENUM(BlueprintType)
enum class EDinosaurCombatRole : uint8
{
    None            UMETA(DisplayName = "None"),
    Predator        UMETA(DisplayName = "Apex Predator"),
    PackHunter      UMETA(DisplayName = "Pack Hunter"),
    Ambusher        UMETA(DisplayName = "Ambush Predator"),
    Territorial     UMETA(DisplayName = "Territorial Defender"),
    Herbivore       UMETA(DisplayName = "Defensive Herbivore"),
    Scavenger       UMETA(DisplayName = "Opportunistic Scavenger")
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Passive         UMETA(DisplayName = "Passive/Neutral"),
    Investigating   UMETA(DisplayName = "Investigating Threat"),
    Stalking        UMETA(DisplayName = "Stalking Target"),
    Hunting         UMETA(DisplayName = "Active Hunting"),
    Attacking       UMETA(DisplayName = "Direct Attack"),
    Retreating      UMETA(DisplayName = "Tactical Retreat"),
    Defending       UMETA(DisplayName = "Defending Territory"),
    Fleeing         UMETA(DisplayName = "Full Flight Response")
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    None            UMETA(DisplayName = "No Threat"),
    Minimal         UMETA(DisplayName = "Minimal Threat"),
    Low             UMETA(DisplayName = "Low Threat"),
    Medium          UMETA(DisplayName = "Medium Threat"),
    High            UMETA(DisplayName = "High Threat"),
    Critical        UMETA(DisplayName = "Critical Threat"),
    Lethal          UMETA(DisplayName = "Lethal Threat")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatPersonality
{
    GENERATED_BODY()

    // Aggression level (0.0 = passive, 1.0 = extremely aggressive)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Aggression = 0.5f;

    // Curiosity level (affects investigation behavior)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Curiosity = 0.3f;

    // Fear threshold (lower = more easily frightened)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Courage = 0.5f;

    // Pack loyalty (affects group behavior)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float PackLoyalty = 0.7f;

    // Territorial instinct
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Territoriality = 0.4f;

    // Intelligence level (affects tactical decisions)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Intelligence = 0.5f;

    // Persistence in pursuit
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Persistence = 0.6f;

    FCombatPersonality()
    {
        // Generate slight random variations for uniqueness
        Aggression += FMath::RandRange(-0.1f, 0.1f);
        Curiosity += FMath::RandRange(-0.1f, 0.1f);
        Courage += FMath::RandRange(-0.1f, 0.1f);
        PackLoyalty += FMath::RandRange(-0.1f, 0.1f);
        Territoriality += FMath::RandRange(-0.1f, 0.1f);
        Intelligence += FMath::RandRange(-0.1f, 0.1f);
        Persistence += FMath::RandRange(-0.1f, 0.1f);

        // Clamp all values
        Aggression = FMath::Clamp(Aggression, 0.0f, 1.0f);
        Curiosity = FMath::Clamp(Curiosity, 0.0f, 1.0f);
        Courage = FMath::Clamp(Courage, 0.0f, 1.0f);
        PackLoyalty = FMath::Clamp(PackLoyalty, 0.0f, 1.0f);
        Territoriality = FMath::Clamp(Territoriality, 0.0f, 1.0f);
        Intelligence = FMath::Clamp(Intelligence, 0.0f, 1.0f);
        Persistence = FMath::Clamp(Persistence, 0.0f, 1.0f);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatCapabilities
{
    GENERATED_BODY()

    // Physical attributes
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HearingRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SmellRange = 1000.0f;

    // Combat stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StaminaMax = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StaminaRegenRate = 10.0f;

    // Special abilities
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanAmbush = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanClimb = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanSwim = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanFly = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanPackHunt = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanCallForHelp = false;
};

/**
 * Core Combat AI Component
 * Manages tactical combat behavior and decision making
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatAIComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    EDinosaurCombatRole CombatRole = EDinosaurCombatRole::Predator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombatPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombatCapabilities Capabilities;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    ECombatState CurrentCombatState = ECombatState::Passive;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    EThreatLevel CurrentThreatLevel = EThreatLevel::None;

    // Current target and threat tracking
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    TArray<AActor*> KnownThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    TArray<AActor*> PackMembers;

    // Gameplay Tags for combat states
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tags")
    FGameplayTagContainer CombatTags;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AddThreat(AActor* Threat, EThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RemoveThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    EThreatLevel EvaluateThreatLevel(AActor* PotentialThreat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetOptimalAttackPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CallForPackSupport();

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat AI")
    void OnCombatStateChanged(ECombatState OldState, ECombatState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat AI")
    void OnThreatDetected(AActor* Threat, EThreatLevel ThreatLevel);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat AI")
    void OnTargetLost();

private:
    // Internal state tracking
    float LastAttackTime = 0.0f;
    float CurrentStamina = 100.0f;
    float StateChangeTime = 0.0f;
    
    // Combat decision making
    void UpdateCombatLogic(float DeltaTime);
    void UpdateThreatAssessment();
    void UpdatePackCoordination();
    
    // Utility functions
    float CalculateDistance(AActor* Target);
    bool HasLineOfSight(AActor* Target);
    bool IsInAttackRange(AActor* Target);
};