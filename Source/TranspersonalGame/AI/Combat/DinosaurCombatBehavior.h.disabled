#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameplayTags.h"
#include "DinosaurCombatBehavior.generated.h"

UENUM(BlueprintType)
enum class EDinosaurCombatType : uint8
{
    Predator,
    Herbivore,
    Omnivore,
    Scavenger,
    Apex,
    Pack,
    Solitary,
    Territorial
};

UENUM(BlueprintType)
enum class EDinosaurAttackPattern : uint8
{
    Bite,
    Claw,
    Tail,
    Charge,
    Stomp,
    Headbutt,
    Spit,
    Roar,
    Grapple,
    Pounce
};

UENUM(BlueprintType)
enum class EDinosaurDefensePattern : uint8
{
    Flee,
    Fight,
    Hide,
    Intimidate,
    CallForHelp,
    Armor,
    Dodge,
    Counter,
    Block,
    Distract
};

USTRUCT(BlueprintType)
struct FDinosaurCombatProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    EDinosaurCombatType CombatType = EDinosaurCombatType::Predator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    float FearThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    float TerritorialRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    bool bCanAmbush = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    bool bUsesIntimidation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    float PreferredCombatRange = 300.0f;

    FDinosaurCombatProfile()
    {
        CombatType = EDinosaurCombatType::Predator;
        AggressionLevel = 0.5f;
        FearThreshold = 0.3f;
        TerritorialRadius = 1000.0f;
        bIsPackHunter = false;
        bCanAmbush = false;
        bUsesIntimidation = true;
        PreferredCombatRange = 300.0f;
    }
};

USTRUCT(BlueprintType)
struct FDinosaurAttackMove
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    EDinosaurAttackPattern AttackType = EDinosaurAttackPattern::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float Damage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float Range = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float Cooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float StaminaCost = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    bool bRequiresLineOfSight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float SuccessChance = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    FString AnimationName = TEXT("Attack_Bite");

    FDinosaurAttackMove()
    {
        AttackType = EDinosaurAttackPattern::Bite;
        Damage = 50.0f;
        Range = 200.0f;
        Cooldown = 2.0f;
        StaminaCost = 20.0f;
        bRequiresLineOfSight = true;
        SuccessChance = 0.8f;
        AnimationName = TEXT("Attack_Bite");
    }
};

/**
 * Component that defines species-specific combat behavior for dinosaurs
 * Handles attack patterns, defense mechanisms, and territorial behavior
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatBehavior();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat Profile
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FDinosaurCombatProfile CombatProfile;

    // Attack Moves
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<FDinosaurAttackMove> AvailableAttacks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    EDinosaurAttackPattern PreferredAttack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    EDinosaurDefensePattern PreferredDefense;

    // Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    float CurrentStamina = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    float MaxStamina = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    bool bIsInCombat = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    bool bIsIntimidating = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    float LastAttackTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    AActor* CurrentThreat = nullptr;

    // Territorial Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    bool bDefendsTerritory = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryAggressionMultiplier = 1.5f;

    // Pack Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bCallsForHelp = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackCoordinationRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bSharesTargets = true;

public:
    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanPerformAttack(EDinosaurAttackPattern AttackType) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(EDinosaurAttackPattern AttackType, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    EDinosaurAttackPattern SelectBestAttack(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteDefense(EDinosaurDefensePattern DefenseType);

    // Intimidation
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartIntimidation(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StopIntimidation();

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsIntimidationEffective(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PlayIntimidationAnimation();

    // Territorial Behavior
    UFUNCTION(BlueprintCallable, Category = "Territory")
    void SetTerritory(FVector Center, float Radius);

    UFUNCTION(BlueprintPure, Category = "Territory")
    bool IsInTerritory(FVector Position) const;

    UFUNCTION(BlueprintPure, Category = "Territory")
    bool IsTerritoryThreatened(AActor* Intruder) const;

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void DefendTerritory(AActor* Intruder);

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void CallPackForHelp();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void RespondToPackCall(AActor* Caller, AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void ShareThreatInformation(AActor* Threat);

    UFUNCTION(BlueprintPure, Category = "Pack")
    TArray<AActor*> GetNearbyPackMembers() const;

    // Stamina Management
    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void ConsumeStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void RegenerateStamina(float Amount);

    UFUNCTION(BlueprintPure, Category = "Stamina")
    float GetStaminaPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Stamina")
    bool HasEnoughStamina(float Required) const;

    // Combat Assessment
    UFUNCTION(BlueprintPure, Category = "Assessment")
    float AssessTargetThreat(AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "Assessment")
    bool ShouldEngageInCombat(AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "Assessment")
    bool ShouldRetreat(AActor* Threat) const;

    UFUNCTION(BlueprintPure, Category = "Assessment")
    float CalculateWinChance(AActor* Opponent) const;

    // Species-Specific Behaviors
    UFUNCTION(BlueprintCallable, Category = "Species")
    void ConfigureForSpecies(const FString& SpeciesName);

    UFUNCTION(BlueprintCallable, Category = "Species")
    void SetupPredatorBehavior();

    UFUNCTION(BlueprintCallable, Category = "Species")
    void SetupHerbivoreBehavior();

    UFUNCTION(BlueprintCallable, Category = "Species")
    void SetupApexPredatorBehavior();

    // Environmental Adaptation
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void AdaptToEnvironment();

    UFUNCTION(BlueprintPure, Category = "Environment")
    bool CanUseEnvironmentForAdvantage() const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    FVector FindCoverPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    FVector FindAmbushPosition(AActor* Target) const;

    // Learning and Adaptation
    UFUNCTION(BlueprintCallable, Category = "Learning")
    void LearnFromCombatOutcome(bool bVictorious, AActor* Opponent);

    UFUNCTION(BlueprintCallable, Category = "Learning")
    void AdaptCombatStyle(AActor* FrequentOpponent);

    UFUNCTION(BlueprintPure, Category = "Learning")
    float GetExperienceAgainst(AActor* OpponentType) const;

    // Debugging
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawCombatInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawTerritory() const;

protected:
    // Internal combat logic
    void UpdateCombatState(float DeltaTime);
    void ProcessStaminaRegeneration(float DeltaTime);
    void MonitorTerritorialThreats();
    void UpdatePackCoordination();
    
    // Attack calculations
    float CalculateAttackDamage(const FDinosaurAttackMove& Attack, AActor* Target) const;
    float CalculateHitChance(const FDinosaurAttackMove& Attack, AActor* Target) const;
    bool ValidateAttackRange(const FDinosaurAttackMove& Attack, AActor* Target) const;
    
    // Behavioral adaptation
    void AdaptAggressionLevel();
    void UpdatePreferredAttacks();
    void AnalyzeCombatPatterns();

    // Timers
    float StaminaRegenTimer = 0.0f;
    float TerritorialCheckTimer = 0.0f;
    float PackCoordinationTimer = 0.0f;
    float IntimidationTimer = 0.0f;

    // Combat memory
    TMap<AActor*, int32> CombatExperience;
    TMap<EDinosaurAttackPattern, float> AttackSuccessRates;
    TArray<AActor*> KnownThreats;
    TArray<AActor*> PackMembers;

    // Cached references
    class ACombatAIController* OwnerController;
    class UWorld* CachedWorld;
    class UAnimInstance* AnimInstance;
};