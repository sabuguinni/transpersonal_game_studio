#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "NPC_EmotionalBehaviorSystem.generated.h"

// Emotional states for NPCs
UENUM(BlueprintType)
enum class ENPC_EmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Happy       UMETA(DisplayName = "Happy"),
    Angry       UMETA(DisplayName = "Angry"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Sad         UMETA(DisplayName = "Sad"),
    Excited     UMETA(DisplayName = "Excited"),
    Suspicious  UMETA(DisplayName = "Suspicious"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Depressed   UMETA(DisplayName = "Depressed"),
    Curious     UMETA(DisplayName = "Curious")
};

// Emotional triggers
UENUM(BlueprintType)
enum class ENPC_EmotionalTrigger : uint8
{
    PlayerApproach      UMETA(DisplayName = "Player Approach"),
    DinosaurSighting    UMETA(DisplayName = "Dinosaur Sighting"),
    FoodFound           UMETA(DisplayName = "Food Found"),
    ThreatDetected      UMETA(DisplayName = "Threat Detected"),
    SocialInteraction   UMETA(DisplayName = "Social Interaction"),
    WeatherChange       UMETA(DisplayName = "Weather Change"),
    TimeOfDay           UMETA(DisplayName = "Time Of Day"),
    InjuryReceived      UMETA(DisplayName = "Injury Received"),
    ResourceLoss        UMETA(DisplayName = "Resource Loss"),
    TerritoryViolation  UMETA(DisplayName = "Territory Violation")
};

// Emotional response data
USTRUCT(BlueprintType)
struct FNPC_EmotionalResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    ENPC_EmotionalState NewState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    bool bOverrideCurrentEmotion;

    FNPC_EmotionalResponse()
    {
        NewState = ENPC_EmotionalState::Calm;
        Intensity = 0.5f;
        Duration = 10.0f;
        bOverrideCurrentEmotion = false;
    }
};

// Emotional memory entry
USTRUCT(BlueprintType)
struct FNPC_EmotionalMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_EmotionalTrigger Trigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_EmotionalState EmotionalResponse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float EmotionalWeight;

    FNPC_EmotionalMemory()
    {
        Trigger = ENPC_EmotionalTrigger::PlayerApproach;
        EmotionalResponse = ENPC_EmotionalState::Calm;
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        EmotionalWeight = 1.0f;
    }
};

/**
 * Advanced emotional behavior system for NPCs
 * Manages emotional states, triggers, and behavioral responses
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_EmotionalBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_EmotionalBehaviorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Current emotional state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    ENPC_EmotionalState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float CurrentEmotionalIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float EmotionalDecayRate;

    // Emotional thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Thresholds")
    float FearThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Thresholds")
    float AngerThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Thresholds")
    float HappinessThreshold;

    // Emotional responses to triggers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Responses")
    TMap<ENPC_EmotionalTrigger, FNPC_EmotionalResponse> EmotionalResponses;

    // Emotional memory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Memory")
    TArray<FNPC_EmotionalMemory> EmotionalMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Memory")
    int32 MaxEmotionalMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Memory")
    float MemoryDecayTime;

    // Personality traits affecting emotions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Neuroticism;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Extraversion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Agreeableness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Conscientiousness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Openness;

    // Timers
    float EmotionalStateTimer;
    float MemoryCleanupTimer;

public:
    // Emotional state management
    UFUNCTION(BlueprintCallable, Category = "Emotional Behavior")
    void TriggerEmotionalResponse(ENPC_EmotionalTrigger Trigger, float Intensity = 1.0f, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Emotional Behavior")
    void SetEmotionalState(ENPC_EmotionalState NewState, float Intensity, float Duration = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotional Behavior")
    ENPC_EmotionalState GetCurrentEmotionalState() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional Behavior")
    float GetCurrentEmotionalIntensity() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional Behavior")
    bool IsInEmotionalState(ENPC_EmotionalState State) const;

    // Emotional memory management
    UFUNCTION(BlueprintCallable, Category = "Emotional Memory")
    void AddEmotionalMemory(ENPC_EmotionalTrigger Trigger, ENPC_EmotionalState Response, const FVector& Location, float Weight = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotional Memory")
    TArray<FNPC_EmotionalMemory> GetEmotionalMemoriesAtLocation(const FVector& Location, float Radius = 1000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Emotional Memory")
    FNPC_EmotionalMemory GetStrongestEmotionalMemory() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional Memory")
    void ClearEmotionalMemories();

    // Personality and emotional modulation
    UFUNCTION(BlueprintCallable, Category = "Personality")
    void SetPersonalityTraits(float InNeuroticism, float InExtraversion, float InAgreeableness, float InConscientiousness, float InOpenness);

    UFUNCTION(BlueprintCallable, Category = "Personality")
    float GetEmotionalModifier(ENPC_EmotionalTrigger Trigger) const;

    // Behavioral responses based on emotions
    UFUNCTION(BlueprintCallable, Category = "Emotional Behavior")
    bool ShouldFleeFromThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional Behavior")
    bool ShouldApproachPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional Behavior")
    bool ShouldBeAggressive() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional Behavior")
    float GetMovementSpeedModifier() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional Behavior")
    float GetInteractionWillingness() const;

private:
    void UpdateEmotionalState(float DeltaTime);
    void CleanupOldMemories();
    float CalculatePersonalityModifier(ENPC_EmotionalTrigger Trigger) const;
    void InitializeDefaultEmotionalResponses();
};