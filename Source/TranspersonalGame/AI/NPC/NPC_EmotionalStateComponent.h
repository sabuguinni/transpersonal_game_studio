#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "NPC_EmotionalStateComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_EmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Agitated    UMETA(DisplayName = "Agitated"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Curious     UMETA(DisplayName = "Curious"),
    Territorial UMETA(DisplayName = "Territorial"),
    Protective  UMETA(DisplayName = "Protective"),
    Hunting     UMETA(DisplayName = "Hunting")
};

USTRUCT(BlueprintType)
struct FNPC_EmotionalTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    ENPC_EmotionalState TargetState = ENPC_EmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float TriggerDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float IntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float DecayRate = 0.1f;

    FNPC_EmotionalTrigger()
    {
        TargetState = ENPC_EmotionalState::Calm;
        TriggerDistance = 1000.0f;
        IntensityMultiplier = 1.0f;
        DecayRate = 0.1f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_EmotionalStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_EmotionalStateComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Current emotional state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    ENPC_EmotionalState CurrentState = ENPC_EmotionalState::Calm;

    // Emotional intensity (0.0 to 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EmotionalIntensity = 0.0f;

    // Base emotional stability
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float EmotionalStability = 1.0f;

    // Emotional triggers for different situations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Triggers")
    TArray<FNPC_EmotionalTrigger> EmotionalTriggers;

    // State transition functions
    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    void TriggerEmotionalState(ENPC_EmotionalState NewState, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    void ModifyEmotionalIntensity(float DeltaIntensity);

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    bool IsInEmotionalState(ENPC_EmotionalState StateToCheck) const;

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    float GetEmotionalIntensityForState(ENPC_EmotionalState State) const;

    // Player proximity emotional response
    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    void UpdateEmotionalResponseToPlayer(AActor* PlayerActor);

    // Threat assessment
    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    void AssessThreatLevel(AActor* ThreatActor, float ThreatLevel);

    // Emotional decay over time
    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    void DecayEmotionalState(float DeltaTime);

    // Get emotional state as string for debugging
    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    FString GetEmotionalStateString() const;

private:
    // Internal state management
    float StateTransitionTimer = 0.0f;
    float LastPlayerDistance = 0.0f;
    ENPC_EmotionalState PreviousState = ENPC_EmotionalState::Calm;

    // Helper functions
    void UpdateEmotionalDecay(float DeltaTime);
    void ProcessEmotionalTriggers();
    bool ShouldTransitionToState(ENPC_EmotionalState NewState) const;
};