#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "NPCEmotionalSystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_EmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Curious     UMETA(DisplayName = "Curious"),
    Protective  UMETA(DisplayName = "Protective"),
    Panicked    UMETA(DisplayName = "Panicked"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Territorial UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class ENPC_EmotionalTrigger : uint8
{
    PlayerApproach      UMETA(DisplayName = "Player Approach"),
    DinosaurSighting    UMETA(DisplayName = "Dinosaur Sighting"),
    LoudNoise          UMETA(DisplayName = "Loud Noise"),
    GroupThreat        UMETA(DisplayName = "Group Threat"),
    ResourceScarcity   UMETA(DisplayName = "Resource Scarcity"),
    TimeOfDay          UMETA(DisplayName = "Time of Day"),
    WeatherChange      UMETA(DisplayName = "Weather Change"),
    InjuryOrPain       UMETA(DisplayName = "Injury or Pain")
};

USTRUCT(BlueprintType)
struct FNPC_EmotionalResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Response")
    ENPC_EmotionalState TargetState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Response")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Response")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Response")
    float DecayRate;

    FNPC_EmotionalResponse()
    {
        TargetState = ENPC_EmotionalState::Calm;
        Intensity = 0.5f;
        Duration = 10.0f;
        DecayRate = 0.1f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_EmotionalMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Memory")
    ENPC_EmotionalTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Memory")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Memory")
    float EmotionalWeight;

    FNPC_EmotionalMemory()
    {
        TriggerType = ENPC_EmotionalTrigger::PlayerApproach;
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        EmotionalWeight = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCEmotionalSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCEmotionalSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Current emotional state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    ENPC_EmotionalState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float CurrentIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float BaselineStress;

    // Emotional responses to different triggers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Responses")
    TMap<ENPC_EmotionalTrigger, FNPC_EmotionalResponse> EmotionalResponses;

    // Memory of emotional events
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Memory")
    TArray<FNPC_EmotionalMemory> EmotionalMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Memory")
    int32 MaxMemories;

    // Personality traits affecting emotional responses
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Bravery; // 0.0 = coward, 1.0 = fearless

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Aggression; // 0.0 = peaceful, 1.0 = hostile

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Curiosity; // 0.0 = avoidant, 1.0 = investigative

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float SocialBond; // 0.0 = loner, 1.0 = group-dependent

    // Methods
    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    void TriggerEmotionalResponse(ENPC_EmotionalTrigger Trigger, float IntensityMultiplier = 1.0f, FVector TriggerLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    void AddEmotionalMemory(ENPC_EmotionalTrigger TriggerType, FVector Location, float EmotionalWeight);

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    float GetEmotionalInfluenceAtLocation(FVector Location, ENPC_EmotionalTrigger TriggerType) const;

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    bool ShouldAvoidLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    float GetCurrentStressLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    void ModifyPersonalityTrait(const FString& TraitName, float Value);

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    FString GetEmotionalStateDescription() const;

private:
    void UpdateEmotionalState(float DeltaTime);
    void ProcessEmotionalDecay(float DeltaTime);
    void CleanOldMemories();
    float CalculatePersonalityInfluence(ENPC_EmotionalTrigger Trigger) const;

    float EmotionalStateTimer;
    float LastMemoryCleanup;
};

#include "NPCEmotionalSystem.generated.h"