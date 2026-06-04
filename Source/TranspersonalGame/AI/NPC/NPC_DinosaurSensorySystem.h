#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "NPC_DinosaurSensorySystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_SenseType : uint8
{
    Sight = 0,
    Hearing = 1,
    Smell = 2,
    Vibration = 3,
    Heat = 4
};

UENUM(BlueprintType)
enum class ENPC_StimulusType : uint8
{
    Player = 0,
    Predator = 1,
    Prey = 2,
    Food = 3,
    Threat = 4,
    PackMember = 5,
    Territory = 6,
    Unknown = 7
};

USTRUCT(BlueprintType)
struct FNPC_SensoryStimulus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory")
    AActor* SourceActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory")
    ENPC_SenseType SenseType = ENPC_SenseType::Sight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory")
    ENPC_StimulusType StimulusType = ENPC_StimulusType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory")
    float Intensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory")
    float DetectionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory")
    float LastUpdateTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory")
    bool bIsActive = true;

    FNPC_SensoryStimulus()
    {
        SourceActor = nullptr;
        Location = FVector::ZeroVector;
        SenseType = ENPC_SenseType::Sight;
        StimulusType = ENPC_StimulusType::Unknown;
        Intensity = 0.0f;
        DetectionTime = 0.0f;
        LastUpdateTime = 0.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct FNPC_SensoryCapabilities
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory Capabilities")
    float SightRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory Capabilities")
    float SightAngle = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory Capabilities")
    float HearingRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory Capabilities")
    float SmellRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory Capabilities")
    float VibrationRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory Capabilities")
    float HeatDetectionRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory Capabilities")
    float SightAcuity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory Capabilities")
    float HearingAcuity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory Capabilities")
    float SmellAcuity = 1.0f;

    FNPC_SensoryCapabilities()
    {
        SightRange = 3000.0f;
        SightAngle = 120.0f;
        HearingRange = 2000.0f;
        SmellRange = 1500.0f;
        VibrationRange = 800.0f;
        HeatDetectionRange = 500.0f;
        SightAcuity = 1.0f;
        HearingAcuity = 1.0f;
        SmellAcuity = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurSensorySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurSensorySystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory System")
    FNPC_SensoryCapabilities SensoryCapabilities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory System")
    TArray<FNPC_SensoryStimulus> ActiveStimuli;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory System")
    float SensoryUpdateInterval = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory System")
    float StimulusDecayTime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory System")
    bool bDebugSensorySystem = false;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Sensory System")
    void PerformSensoryUpdate();

    UFUNCTION(BlueprintCallable, Category = "Sensory System")
    bool CanSeeActor(AActor* TargetActor, float& OutIntensity);

    UFUNCTION(BlueprintCallable, Category = "Sensory System")
    bool CanHearActor(AActor* TargetActor, float NoiseLevel, float& OutIntensity);

    UFUNCTION(BlueprintCallable, Category = "Sensory System")
    bool CanSmellActor(AActor* TargetActor, float ScentStrength, float& OutIntensity);

    UFUNCTION(BlueprintCallable, Category = "Sensory System")
    void AddStimulus(const FNPC_SensoryStimulus& Stimulus);

    UFUNCTION(BlueprintCallable, Category = "Sensory System")
    void RemoveStimulus(AActor* SourceActor);

    UFUNCTION(BlueprintCallable, Category = "Sensory System")
    TArray<FNPC_SensoryStimulus> GetActiveStimuli() const;

    UFUNCTION(BlueprintCallable, Category = "Sensory System")
    FNPC_SensoryStimulus GetStrongestStimulus() const;

    UFUNCTION(BlueprintCallable, Category = "Sensory System")
    TArray<FNPC_SensoryStimulus> GetStimuliByType(ENPC_StimulusType StimulusType) const;

    UFUNCTION(BlueprintCallable, Category = "Sensory System")
    bool HasStimulusOfType(ENPC_StimulusType StimulusType) const;

    UFUNCTION(BlueprintCallable, Category = "Sensory System")
    void SetSensoryCapabilities(const FNPC_SensoryCapabilities& NewCapabilities);

    UFUNCTION(BlueprintCallable, Category = "Sensory System")
    void ModifySensoryAcuity(ENPC_SenseType SenseType, float Multiplier);

private:
    void UpdateActiveStimuli(float DeltaTime);
    void CleanupExpiredStimuli();
    void ScanForNewStimuli();
    
    ENPC_StimulusType ClassifyActor(AActor* Actor);
    float CalculateVisualIntensity(AActor* TargetActor, float Distance);
    float CalculateAudioIntensity(float Distance, float NoiseLevel);
    float CalculateScentIntensity(AActor* TargetActor, float Distance, float ScentStrength);
    
    bool IsInSightCone(AActor* TargetActor);
    bool HasLineOfSight(AActor* TargetActor);
    
    float LastSensoryUpdate = 0.0f;
};