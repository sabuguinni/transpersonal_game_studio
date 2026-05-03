#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "CombatAITypes.h"
#include "Combat_CombatStateManager.generated.h"

UENUM(BlueprintType)
enum class ECombat_CombatPhase : uint8
{
    PreCombat       UMETA(DisplayName = "Pre-Combat"),
    Engagement      UMETA(DisplayName = "Engagement"),
    ActiveCombat    UMETA(DisplayName = "Active Combat"),
    Retreat         UMETA(DisplayName = "Retreat"),
    PostCombat      UMETA(DisplayName = "Post-Combat"),
    Victory         UMETA(DisplayName = "Victory"),
    Defeat          UMETA(DisplayName = "Defeat")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None           UMETA(DisplayName = "None"),
    Low            UMETA(DisplayName = "Low"),
    Medium         UMETA(DisplayName = "Medium"),
    High           UMETA(DisplayName = "High"),
    Critical       UMETA(DisplayName = "Critical"),
    Overwhelming   UMETA(DisplayName = "Overwhelming")
};

USTRUCT(BlueprintType)
struct FCombat_CombatState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    ECombat_CombatPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    float CombatIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    float TimeInCombat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    int32 EnemiesInRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    bool bIsPlayerInvolved;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    bool bIsPackCombat;

    FCombat_CombatState()
    {
        CurrentPhase = ECombat_CombatPhase::PreCombat;
        ThreatLevel = ECombat_ThreatLevel::None;
        CombatIntensity = 0.0f;
        TimeInCombat = 0.0f;
        EnemiesInRange = 0;
        bIsPlayerInvolved = false;
        bIsPackCombat = false;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_CombatStateManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_CombatStateManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat State")
    void InitiateCombat(AActor* Target, ECombat_ThreatLevel InitialThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat State")
    void EndCombat(bool bVictory);

    UFUNCTION(BlueprintCallable, Category = "Combat State")
    void UpdateCombatPhase(ECombat_CombatPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Combat State")
    void UpdateThreatLevel(ECombat_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat State")
    void AddEnemyToRange(AActor* Enemy);

    UFUNCTION(BlueprintCallable, Category = "Combat State")
    void RemoveEnemyFromRange(AActor* Enemy);

    // State Queries
    UFUNCTION(BlueprintPure, Category = "Combat State")
    bool IsInCombat() const;

    UFUNCTION(BlueprintPure, Category = "Combat State")
    ECombat_CombatPhase GetCurrentPhase() const { return CombatState.CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Combat State")
    ECombat_ThreatLevel GetThreatLevel() const { return CombatState.ThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "Combat State")
    float GetCombatIntensity() const { return CombatState.CombatIntensity; }

    UFUNCTION(BlueprintPure, Category = "Combat State")
    int32 GetEnemyCount() const { return CombatState.EnemiesInRange; }

    // Combat Analytics
    UFUNCTION(BlueprintCallable, Category = "Combat Analytics")
    void LogCombatEvent(const FString& EventDescription);

    UFUNCTION(BlueprintCallable, Category = "Combat Analytics")
    void RecordCombatMetrics();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    FCombat_CombatState CombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float CombatTimeoutDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float IntensityDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float MaxCombatRange;

    // Internal tracking
    UPROPERTY()
    TArray<AActor*> EnemiesInCombat;

    UPROPERTY()
    AActor* PrimaryTarget;

    UPROPERTY()
    float LastCombatActivity;

    // Combat intensity calculation
    void UpdateCombatIntensity(float DeltaTime);
    void CalculateThreatLevel();
    void CheckCombatTimeout(float DeltaTime);

    // Event handling
    void OnCombatPhaseChanged(ECombat_CombatPhase OldPhase, ECombat_CombatPhase NewPhase);
    void OnThreatLevelChanged(ECombat_ThreatLevel OldLevel, ECombat_ThreatLevel NewLevel);
};