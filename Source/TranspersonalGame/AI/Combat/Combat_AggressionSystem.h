#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Combat_AggressionSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_AggressionLevel : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Cautious    UMETA(DisplayName = "Cautious"), 
    Alert       UMETA(DisplayName = "Alert"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Enraged     UMETA(DisplayName = "Enraged")
};

UENUM(BlueprintType)
enum class ECombat_ThreatType : uint8
{
    None        UMETA(DisplayName = "None"),
    Player      UMETA(DisplayName = "Player"),
    Predator    UMETA(DisplayName = "Predator"),
    Territory   UMETA(DisplayName = "Territory"),
    Food        UMETA(DisplayName = "Food"),
    Pack        UMETA(DisplayName = "Pack")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_AggressionState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggression")
    ECombat_AggressionLevel CurrentLevel = ECombat_AggressionLevel::Passive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggression")
    float AggressionValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggression")
    float LastAggressionIncrease = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggression")
    ECombat_ThreatType PrimaryThreat = ECombat_ThreatType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggression")
    AActor* ThreatTarget = nullptr;

    FCombat_AggressionState()
    {
        CurrentLevel = ECombat_AggressionLevel::Passive;
        AggressionValue = 0.0f;
        LastAggressionIncrease = 0.0f;
        PrimaryThreat = ECombat_ThreatType::None;
        ThreatTarget = nullptr;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_AggressionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_AggressionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Aggression State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggression State")
    FCombat_AggressionState CurrentState;

    // Aggression Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggression Tuning")
    float BaseAggressionDecay = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggression Tuning")
    float MaxAggression = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggression Tuning")
    float PassiveThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggression Tuning")
    float CautiousThreshold = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggression Tuning")
    float AlertThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggression Tuning")
    float AggressiveThreshold = 75.0f;

    // Species-specific modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Traits")
    float SpeciesAggressionModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Traits")
    bool bNaturallyAggressive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Traits")
    bool bTerritorial = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Traits")
    bool bPackHunter = false;

    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Aggression")
    void IncreaseAggression(float Amount, ECombat_ThreatType ThreatType, AActor* ThreatSource = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Aggression")
    void DecreaseAggression(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Aggression")
    void SetAggressionLevel(ECombat_AggressionLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Aggression")
    ECombat_AggressionLevel GetAggressionLevel() const { return CurrentState.CurrentLevel; }

    UFUNCTION(BlueprintCallable, Category = "Aggression")
    float GetAggressionValue() const { return CurrentState.AggressionValue; }

    UFUNCTION(BlueprintCallable, Category = "Aggression")
    bool IsAggressive() const { return CurrentState.CurrentLevel >= ECombat_AggressionLevel::Aggressive; }

    UFUNCTION(BlueprintCallable, Category = "Aggression")
    bool IsEnraged() const { return CurrentState.CurrentLevel == ECombat_AggressionLevel::Enraged; }

    UFUNCTION(BlueprintCallable, Category = "Aggression")
    AActor* GetCurrentThreat() const { return CurrentState.ThreatTarget; }

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    void AssessThreat(AActor* PotentialThreat);

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    float CalculateThreatLevel(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    ECombat_ThreatType ClassifyThreat(AActor* Actor) const;

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Aggression Events")
    void OnAggressionLevelChanged(ECombat_AggressionLevel OldLevel, ECombat_AggressionLevel NewLevel);

    UFUNCTION(BlueprintImplementableEvent, Category = "Aggression Events")
    void OnThreatDetected(AActor* ThreatActor, ECombat_ThreatType ThreatType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Aggression Events")
    void OnBecameAggressive(AActor* Target);

    UFUNCTION(BlueprintImplementableEvent, Category = "Aggression Events")
    void OnCalmedDown();

protected:
    void UpdateAggressionLevel();
    void ProcessAggressionDecay(float DeltaTime);
    void HandleSpeciesModifiers();
    
private:
    ECombat_AggressionLevel PreviousLevel;
    float LastThreatAssessmentTime;
    
    // Internal threat tracking
    TMap<AActor*, float> ThreatHistory;
    float ThreatAssessmentInterval = 1.0f;
};