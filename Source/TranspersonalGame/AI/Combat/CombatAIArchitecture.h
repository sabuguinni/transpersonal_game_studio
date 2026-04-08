#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "StateTreeComponent.h"
#include "GameplayTagContainer.h"
#include "CombatAIArchitecture.generated.h"

/**
 * Combat AI Architecture for Transpersonal Game Studio
 * Hybrid system combining Behavior Trees, State Trees, EQS, and AI Perception
 * for intelligent dinosaur combat behavior
 */

UENUM(BlueprintType)
enum class EDinosaurArchetype : uint8
{
    ApexPredator     UMETA(DisplayName = "Apex Predator"),      // T-Rex, Giganotosaurus
    PackHunter       UMETA(DisplayName = "Pack Hunter"),        // Velociraptor, Deinonychus
    Ambusher         UMETA(DisplayName = "Ambusher"),           // Carnotaurus, Baryonyx
    TerritorialGuard UMETA(DisplayName = "Territorial Guard"),  // Triceratops, Ankylosaurus
    HerdAnimal       UMETA(DisplayName = "Herd Animal"),        // Parasaurolophus, Brachiosaurus
    Scavenger        UMETA(DisplayName = "Scavenger"),          // Compsognathus, Coelophysis
    AerialThreat     UMETA(DisplayName = "Aerial Threat"),      // Pteranodon, Quetzalcoatlus
    AquaticPredator  UMETA(DisplayName = "Aquatic Predator")    // Mosasaurus, Leedsichthys
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Passive          UMETA(DisplayName = "Passive"),
    Investigating    UMETA(DisplayName = "Investigating"),
    Stalking         UMETA(DisplayName = "Stalking"),
    Hunting          UMETA(DisplayName = "Hunting"),
    Attacking        UMETA(DisplayName = "Attacking"),
    Retreating       UMETA(DisplayName = "Retreating"),
    Territorial      UMETA(DisplayName = "Territorial"),
    Feeding          UMETA(DisplayName = "Feeding"),
    Fleeing          UMETA(DisplayName = "Fleeing")
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    None             UMETA(DisplayName = "None"),
    Low              UMETA(DisplayName = "Low"),
    Medium           UMETA(DisplayName = "Medium"),
    High             UMETA(DisplayName = "High"),
    Critical         UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatPersonality
{
    GENERATED_BODY()

    // Aggression level (0.0 = passive, 1.0 = extremely aggressive)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.5f;

    // Curiosity level (affects investigation behavior)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.3f;

    // Fear threshold (when to retreat)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FearThreshold = 0.7f;

    // Pack loyalty (for pack hunters)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PackLoyalty = 0.8f;

    // Territory size preference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "500.0", ClampMax = "5000.0"))
    float TerritoryRadius = 1500.0f;

    // Preferred attack distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float PreferredAttackDistance = 300.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurCombatStats
{
    GENERATED_BODY()

    // Base damage output
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0", ClampMax = "1000.0"))
    float BaseDamage = 50.0f;

    // Attack speed (attacks per second)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float AttackSpeed = 1.0f;

    // Movement speed multiplier during combat
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float CombatSpeedMultiplier = 1.2f;

    // Detection range in centimeters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "500.0", ClampMax = "10000.0"))
    float DetectionRange = 2000.0f;

    // Sight angle in degrees
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "30.0", ClampMax = "360.0"))
    float SightAngle = 120.0f;

    // Hearing range in centimeters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1000.0", ClampMax = "15000.0"))
    float HearingRange = 5000.0f;
};

/**
 * Base Combat AI Controller
 * Manages all combat-related AI behavior for dinosaurs
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UStateTreeComponent* StateTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UAIPerceptionComponent* PerceptionComponent;

    // Combat Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Config")
    EDinosaurArchetype DinosaurArchetype = EDinosaurArchetype::PackHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Config")
    FCombatPersonality CombatPersonality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Config")
    FDinosaurCombatStats CombatStats;

    // Behavior Tree Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Assets")
    class UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Assets")
    class UBlackboardAsset* CombatBlackboard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Assets")
    class UStateTree* CombatStateTree;

    // Current Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    ECombatState CurrentCombatState = ECombatState::Passive;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    EThreatLevel CurrentThreatLevel = EThreatLevel::None;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    TArray<AActor*> KnownThreats;

    // Gameplay Tags for Combat States
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Tags")
    FGameplayTagContainer CombatTags;

public:
    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetThreatLevel(EThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AddKnownThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RemoveKnownThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* SelectBestTarget();

    // Tactical Positioning
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector FindTacticalPosition(AActor* Target, float MinDistance, float MaxDistance);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool HasLineOfSight(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float CalculateThreatScore(AActor* PotentialThreat);

    // Pack Coordination (for pack hunters)
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<ACombatAIController*> GetNearbyPackMembers(float SearchRadius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePackAttack(AActor* Target);

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    // Internal Combat Logic
    void UpdateCombatState(float DeltaTime);
    void ProcessThreatAssessment();
    void ExecuteCombatBehavior();
    
    // Archetype-specific behavior
    void ExecuteApexPredatorBehavior();
    void ExecutePackHunterBehavior();
    void ExecuteAmbusherBehavior();
    void ExecuteTerritorialGuardBehavior();
    void ExecuteHerdAnimalBehavior();
    void ExecuteScavengerBehavior();
    void ExecuteAerialThreatBehavior();
    void ExecuteAquaticPredatorBehavior();

private:
    // Internal state tracking
    float LastCombatStateChange = 0.0f;
    float LastThreatAssessment = 0.0f;
    float CombatStateTimer = 0.0f;
    
    // Performance optimization
    static constexpr float THREAT_ASSESSMENT_INTERVAL = 0.5f;
    static constexpr float COMBAT_UPDATE_INTERVAL = 0.1f;
};