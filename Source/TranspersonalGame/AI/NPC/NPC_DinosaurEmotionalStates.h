#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "NPC_DinosaurEmotionalStates.generated.h"

UENUM(BlueprintType)
enum class ENPC_EmotionalState : uint8
{
    Calm = 0,
    Alert = 1,
    Aggressive = 2,
    Fearful = 3,
    Territorial = 4,
    Hunting = 5,
    Feeding = 6,
    Resting = 7,
    Curious = 8,
    Protective = 9
};

UENUM(BlueprintType)
enum class ENPC_StressLevel : uint8
{
    Relaxed = 0,
    Mild = 1,
    Moderate = 2,
    High = 3,
    Extreme = 4
};

USTRUCT(BlueprintType)
struct FNPC_EmotionalProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    ENPC_EmotionalState CurrentState = ENPC_EmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    ENPC_StressLevel StressLevel = ENPC_StressLevel::Relaxed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float AggressionLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float CuriosityLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float TerritorialLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float SocialBondStrength = 0.0f;

    FNPC_EmotionalProfile()
    {
        CurrentState = ENPC_EmotionalState::Calm;
        StressLevel = ENPC_StressLevel::Relaxed;
        AggressionLevel = 0.0f;
        FearLevel = 0.0f;
        CuriosityLevel = 0.5f;
        TerritorialLevel = 0.3f;
        SocialBondStrength = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurEmotionalStates : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurEmotionalStates();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional System")
    FNPC_EmotionalProfile EmotionalProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional System")
    float StateTransitionSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional System")
    float StressDecayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional System")
    float EmotionalVolatility = 0.5f;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    void SetEmotionalState(ENPC_EmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    ENPC_EmotionalState GetCurrentEmotionalState() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    void ModifyStressLevel(float StressChange);

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    void ModifyAggression(float AggressionChange);

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    void ModifyFear(float FearChange);

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    void TriggerThreatResponse(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    void TriggerCalmingInfluence(float CalmingStrength);

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    bool IsInAggressiveState() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    bool IsInDefensiveState() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    float GetOverallStressLevel() const;

private:
    void UpdateEmotionalState(float DeltaTime);
    void ProcessStressDecay(float DeltaTime);
    void HandleStateTransitions();
    
    float LastStateChangeTime = 0.0f;
    ENPC_EmotionalState PreviousState = ENPC_EmotionalState::Calm;
};