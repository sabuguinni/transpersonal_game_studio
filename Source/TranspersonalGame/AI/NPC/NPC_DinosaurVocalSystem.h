#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "SharedTypes.h"
#include "NPC_DinosaurVocalSystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_VocalizationType : uint8
{
    TerritorialRoar     UMETA(DisplayName = "Territorial Roar"),
    PackCall            UMETA(DisplayName = "Pack Call"),
    AlarmCall           UMETA(DisplayName = "Alarm Call"),
    HuntingCall         UMETA(DisplayName = "Hunting Call"),
    MatingCall          UMETA(DisplayName = "Mating Call"),
    DistressCall        UMETA(DisplayName = "Distress Call"),
    SubmissionCall      UMETA(DisplayName = "Submission Call"),
    AggressionGrowl     UMETA(DisplayName = "Aggression Growl")
};

USTRUCT(BlueprintType)
struct FNPC_VocalPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocal Pattern")
    ENPC_VocalizationType VocalizationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocal Pattern")
    float Range;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocal Pattern")
    float Frequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocal Pattern")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocal Pattern")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocal Pattern")
    bool bCanInterruptOtherCalls;

    FNPC_VocalPattern()
    {
        VocalizationType = ENPC_VocalizationType::TerritorialRoar;
        Range = 2000.0f;
        Frequency = 1.0f;
        Duration = 3.0f;
        Intensity = 1.0f;
        bCanInterruptOtherCalls = false;
    }
};

USTRUCT(BlueprintType)
struct FNPC_VocalResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocal Response")
    ENPC_VocalizationType TriggerCall;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocal Response")
    ENPC_VocalizationType ResponseCall;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocal Response")
    float ResponseDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocal Response")
    float ResponseProbability;

    FNPC_VocalResponse()
    {
        TriggerCall = ENPC_VocalizationType::PackCall;
        ResponseCall = ENPC_VocalizationType::PackCall;
        ResponseDelay = 1.0f;
        ResponseProbability = 0.8f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurVocalSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurVocalSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Vocal patterns for different dinosaur species
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocal System")
    TMap<ENPC_DinosaurSpecies, TArray<FNPC_VocalPattern>> SpeciesVocalPatterns;

    // Audio component for playing vocalizations
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vocal System")
    UAudioComponent* VocalAudioComponent;

    // Current active vocalization
    UPROPERTY(BlueprintReadOnly, Category = "Vocal System")
    ENPC_VocalizationType CurrentVocalization;

    // Vocal response patterns
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocal System")
    TArray<FNPC_VocalResponse> VocalResponses;

    // Communication range
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocal System", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
    float CommunicationRange;

    // Vocal frequency (calls per minute)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocal System", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float VocalFrequency;

    // Current emotional state affects vocalizations
    UPROPERTY(BlueprintReadWrite, Category = "Vocal System")
    ENPC_EmotionalState CurrentEmotionalState;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Vocal System")
    void PlayVocalization(ENPC_VocalizationType VocalizationType);

    UFUNCTION(BlueprintCallable, Category = "Vocal System")
    void RespondToVocalization(ENPC_VocalizationType HeardVocalization, AActor* Caller);

    UFUNCTION(BlueprintCallable, Category = "Vocal System")
    void BroadcastTerritorialClaim();

    UFUNCTION(BlueprintCallable, Category = "Vocal System")
    void CallForPackAssistance();

    UFUNCTION(BlueprintCallable, Category = "Vocal System")
    void SoundAlarmCall();

    UFUNCTION(BlueprintCallable, Category = "Vocal System")
    void InitiateHuntingCall();

    UFUNCTION(BlueprintCallable, Category = "Vocal System")
    TArray<AActor*> GetActorsInVocalRange();

    UFUNCTION(BlueprintCallable, Category = "Vocal System")
    bool CanHearVocalization(AActor* Caller, float CallRange);

    UFUNCTION(BlueprintCallable, Category = "Vocal System")
    void SetSpeciesVocalPatterns(ENPC_DinosaurSpecies Species);

private:
    // Internal timers
    float LastVocalizationTime;
    float VocalizationCooldown;

    // Initialize species-specific vocal patterns
    void InitializeVocalPatterns();

    // Process incoming vocalizations from other dinosaurs
    void ProcessIncomingVocalizations();

    // Determine appropriate vocal response based on context
    ENPC_VocalizationType DetermineVocalResponse(ENPC_VocalizationType HeardCall, AActor* Caller);

    // Modify vocalization based on emotional state
    void ModifyVocalizationForEmotionalState(FNPC_VocalPattern& Pattern);
};