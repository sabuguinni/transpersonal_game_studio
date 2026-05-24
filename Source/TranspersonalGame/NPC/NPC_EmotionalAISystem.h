#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "NPC_EmotionalAISystem.generated.h"

// Emotional states for NPCs
UENUM(BlueprintType)
enum class ENPC_EmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Curious     UMETA(DisplayName = "Curious"),
    Protective  UMETA(DisplayName = "Protective"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Territorial UMETA(DisplayName = "Territorial"),
    Social      UMETA(DisplayName = "Social"),
    Stressed    UMETA(DisplayName = "Stressed"),
    Content     UMETA(DisplayName = "Content")
};

// Emotion intensity levels
UENUM(BlueprintType)
enum class ENPC_EmotionIntensity : uint8
{
    VeryLow     UMETA(DisplayName = "Very Low"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    VeryHigh    UMETA(DisplayName = "Very High"),
    Extreme     UMETA(DisplayName = "Extreme")
};

// Emotional response triggers
UENUM(BlueprintType)
enum class ENPC_EmotionTrigger : uint8
{
    PlayerPresence      UMETA(DisplayName = "Player Presence"),
    LoudNoise          UMETA(DisplayName = "Loud Noise"),
    PredatorSight      UMETA(DisplayName = "Predator Sight"),
    FoodSource         UMETA(DisplayName = "Food Source"),
    PackMemberDanger   UMETA(DisplayName = "Pack Member Danger"),
    TerritoryInvasion  UMETA(DisplayName = "Territory Invasion"),
    WeatherChange      UMETA(DisplayName = "Weather Change"),
    TimeOfDay          UMETA(DisplayName = "Time of Day"),
    HealthStatus       UMETA(DisplayName = "Health Status"),
    SocialInteraction  UMETA(DisplayName = "Social Interaction")
};

// Emotional memory structure
USTRUCT(BlueprintType)
struct FNPC_EmotionalMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Memory")
    ENPC_EmotionalState EmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Memory")
    ENPC_EmotionIntensity Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Memory")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Memory")
    float DecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Memory")
    bool bIsPersistent;

    FNPC_EmotionalMemory()
    {
        EmotionalState = ENPC_EmotionalState::Calm;
        Intensity = ENPC_EmotionIntensity::Medium;
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        DecayRate = 1.0f;
        bIsPersistent = false;
    }
};

// Personality traits that influence emotional responses
USTRUCT(BlueprintType)
struct FNPC_PersonalityTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggressiveness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fearfulness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Territoriality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Protectiveness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Adaptability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence;

    FNPC_PersonalityTraits()
    {
        Aggressiveness = 0.5f;
        Fearfulness = 0.5f;
        Curiosity = 0.5f;
        Sociability = 0.5f;
        Territoriality = 0.5f;
        Protectiveness = 0.5f;
        Adaptability = 0.5f;
        Intelligence = 0.5f;
    }
};

/**
 * Advanced emotional AI system for NPCs that drives behavior through emotional states
 * Provides dynamic, believable AI responses based on personality and environmental factors
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_EmotionalAISystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_EmotionalAISystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Current emotional state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional AI")
    ENPC_EmotionalState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional AI")
    ENPC_EmotionIntensity CurrentIntensity;

    // Personality traits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional AI")
    FNPC_PersonalityTraits PersonalityTraits;

    // Emotional memory system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional AI")
    TArray<FNPC_EmotionalMemory> EmotionalMemories;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional AI")
    float EmotionalDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional AI")
    float EmotionalSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional AI")
    float MaxEmotionalMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional AI")
    float EmotionalUpdateFrequency;

    // Core emotional AI functions
    UFUNCTION(BlueprintCallable, Category = "Emotional AI")
    void TriggerEmotionalResponse(ENPC_EmotionTrigger Trigger, ENPC_EmotionIntensity Intensity, FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Emotional AI")
    void SetEmotionalState(ENPC_EmotionalState NewState, ENPC_EmotionIntensity Intensity);

    UFUNCTION(BlueprintCallable, Category = "Emotional AI")
    ENPC_EmotionalState GetDominantEmotionalState() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional AI")
    float GetEmotionalIntensityValue() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional AI")
    bool IsInEmotionalState(ENPC_EmotionalState State) const;

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "Emotional AI")
    void AddEmotionalMemory(ENPC_EmotionalState State, ENPC_EmotionIntensity Intensity, FVector Location, bool bPersistent = false);

    UFUNCTION(BlueprintCallable, Category = "Emotional AI")
    void ClearEmotionalMemories();

    UFUNCTION(BlueprintCallable, Category = "Emotional AI")
    TArray<FNPC_EmotionalMemory> GetEmotionalMemoriesAtLocation(FVector Location, float Radius) const;

    // Personality influence
    UFUNCTION(BlueprintCallable, Category = "Emotional AI")
    float GetPersonalityInfluence(ENPC_EmotionalState State) const;

    UFUNCTION(BlueprintCallable, Category = "Emotional AI")
    void RandomizePersonality();

    UFUNCTION(BlueprintCallable, Category = "Emotional AI")
    void SetPersonalityPreset(const FString& PresetName);

    // Behavioral modifiers
    UFUNCTION(BlueprintCallable, Category = "Emotional AI")
    float GetMovementSpeedModifier() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional AI")
    float GetAggressionModifier() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional AI")
    float GetDetectionRangeModifier() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional AI")
    bool ShouldFleeFromThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional AI")
    bool ShouldInvestigateSound() const;

private:
    // Internal state management
    float LastEmotionalUpdate;
    float EmotionalStateTimer;
    TMap<ENPC_EmotionalState, float> EmotionalStateWeights;

    // Helper functions
    void UpdateEmotionalState(float DeltaTime);
    void ProcessEmotionalDecay(float DeltaTime);
    void CalculateEmotionalResponse(ENPC_EmotionTrigger Trigger, ENPC_EmotionIntensity Intensity);
    ENPC_EmotionalState DetermineNewEmotionalState(ENPC_EmotionTrigger Trigger) const;
    void CleanupOldMemories();
    float CalculateEmotionalWeight(ENPC_EmotionalState State) const;
};