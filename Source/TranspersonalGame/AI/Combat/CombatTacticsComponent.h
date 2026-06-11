#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "CombatBehaviorTree.h"
#include "CombatTacticsComponent.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticType : uint8
{
    None            UMETA(DisplayName = "None"),
    Ambush          UMETA(DisplayName = "Ambush"),
    Flanking        UMETA(DisplayName = "Flanking"),
    PackHunt        UMETA(DisplayName = "Pack Hunt"),
    Intimidation    UMETA(DisplayName = "Intimidation"),
    Retreat         UMETA(DisplayName = "Retreat"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Defensive       UMETA(DisplayName = "Defensive")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    ECombat_TacticType TacticType = ECombat_TacticType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float ExecutionChance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float CooldownTime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float EffectiveRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    int32 RequiredAllies = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bRequiresLineOfSight = true;

    FCombat_TacticData()
    {
        TacticType = ECombat_TacticType::None;
        ExecutionChance = 0.5f;
        CooldownTime = 10.0f;
        EffectiveRange = 1000.0f;
        RequiredAllies = 0;
        bRequiresLineOfSight = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_EnvironmentFactor
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bInCover = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bHighGround = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bNearWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bInForest = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float VisibilityLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float NoiseLevel = 0.5f;

    FCombat_EnvironmentFactor()
    {
        bInCover = false;
        bHighGround = false;
        bNearWater = false;
        bInForest = false;
        VisibilityLevel = 1.0f;
        NoiseLevel = 0.5f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombatTacticsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatTacticsComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactics")
    TArray<FCombat_TacticData> AvailableTactics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactics")
    FCombat_TacticData CurrentTactic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactics")
    FCombat_EnvironmentFactor EnvironmentFactors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactics")
    float TacticalUpdateInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactics")
    float TacticalAwareness = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactics")
    bool bCanUseTactics = true;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void InitializeTactics();

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    bool SelectBestTactic(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void ExecuteCurrentTactic(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    bool CanExecuteTactic(const FCombat_TacticData& Tactic, AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void UpdateEnvironmentFactors();

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    float CalculateTacticEffectiveness(const FCombat_TacticData& Tactic, AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    TArray<AActor*> FindNearbyAllies(float SearchRadius = 2000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    bool HasLineOfSight(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void SetTacticalCooldown(ECombat_TacticType TacticType, float CooldownDuration);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Tactics")
    TMap<ECombat_TacticType, float> TacticalCooldowns;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Tactics")
    float LastTacticalUpdate = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Tactics")
    AActor* CurrentTarget = nullptr;

    void InitializeDefaultTactics();
    void UpdateTacticalCooldowns(float DeltaTime);
    bool IsTacticOnCooldown(ECombat_TacticType TacticType) const;
    void ExecuteAmbushTactic(AActor* Target);
    void ExecuteFlankingTactic(AActor* Target);
    void ExecutePackHuntTactic(AActor* Target);
    void ExecuteIntimidationTactic(AActor* Target);
    void ExecuteRetreatTactic(AActor* Target);
    void ExecuteTerritorialTactic(AActor* Target);
    void ExecuteDefensiveTactic(AActor* Target);
};