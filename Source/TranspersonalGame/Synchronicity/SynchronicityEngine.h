#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "../Core/ConsciousnessSystem.h"
#include "SynchronicityEngine.generated.h"

UENUM(BlueprintType)
enum class ESynchronicityType : uint8
{
    Numerical       UMETA(DisplayName = "Numerical Synchronicity"),
    Environmental   UMETA(DisplayName = "Environmental Synchronicity"),
    Interpersonal   UMETA(DisplayName = "Interpersonal Synchronicity"),
    Archetypal      UMETA(DisplayName = "Archetypal Synchronicity"),
    Temporal        UMETA(DisplayName = "Temporal Synchronicity"),
    Symbolic        UMETA(DisplayName = "Symbolic Synchronicity"),
    Quantum         UMETA(DisplayName = "Quantum Synchronicity")
};

USTRUCT(BlueprintType)
struct FSynchronicityEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synchronicity")
    ESynchronicityType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synchronicity")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synchronicity")
    float Probability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synchronicity")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synchronicity")
    FVector WorldLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synchronicity")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synchronicity")
    TArray<AActor*> AffectedActors;

    FSynchronicityEvent()
    {
        Type = ESynchronicityType::Environmental;
        Intensity = 1.0f;
        Probability = 0.1f;
        Description = TEXT("Synchronistic event");
        WorldLocation = FVector::ZeroVector;
        Duration = 5.0f;
    }
};

USTRUCT(BlueprintType)
struct FQuantumField
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quantum")
    float Coherence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quantum")
    float Entanglement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quantum")
    float NonLocality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quantum")
    TMap<AActor*, float> ActorResonance;

    FQuantumField()
    {
        Coherence = 0.0f;
        Entanglement = 0.0f;
        NonLocality = 0.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API USynchronicityEngine : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synchronicity")
    float GlobalSynchronicityField;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synchronicity")
    FQuantumField QuantumField;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float BaseSynchronicityRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float ConsciousnessAmplificationFactor;

    UFUNCTION(BlueprintCallable, Category = "Synchronicity")
    void UpdateSynchronicityField(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Synchronicity")
    void TriggerSynchronicityEvent(ESynchronicityType Type, FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Synchronicity")
    bool CheckForSynchronicity(AActor* Actor, float ConsciousnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Synchronicity")
    void CreateQuantumEntanglement(AActor* Actor1, AActor* Actor2);

    UFUNCTION(BlueprintPure, Category = "Synchronicity")
    float CalculateSynchronicityProbability(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Synchronicity")
    void ProcessMeaningfulCoincidence(const TArray<AActor*>& InvolvedActors, const FString& EventDescription);

private:
    TArray<FSynchronicityEvent> ActiveEvents;
    TMap<AActor*, float> ActorSynchronicityLevels;
    float FieldUpdateTimer;

    void ProcessActiveEvents(float DeltaTime);
    void CalculateQuantumField();
    void ApplySynchronicityEffects(const FSynchronicityEvent& Event, float DeltaTime);
    float GetCollectiveConsciousness() const;
    void GenerateNumericalSynchronicity(AActor* Actor);
    void GenerateEnvironmentalSynchronicity(FVector Location);
    void GenerateArchetypalSynchronicity(const TArray<AActor*>& Actors);
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USynchronicityDetector : public UActorComponent
{
    GENERATED_BODY()

public:
    USynchronicityDetector();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float SensitivityRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    bool bAutoDetection;

    UFUNCTION(BlueprintCallable, Category = "Detection")
    void ScanForSynchronicities();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnSynchronicityDetected(const FSynchronicityEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Detection")
    void RegisterMeaningfulEvent(const FString& EventDescription, float PersonalSignificance);

private:
    USynchronicityEngine* SyncEngine;
    TArray<FString> RecentEvents;
    float LastScanTime;

    void AnalyzePatterns();
    bool IsEventMeaningful(const FString& Event) const;
};

UCLASS()
class TRANSPERSONALGAME_API ASynchronicityManifestor : public AActor
{
    GENERATED_BODY()

public:
    ASynchronicityManifestor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manifestation")
    ESynchronicityType ManifestationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manifestation")
    float ManifestationPower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manifestation")
    float ActivationRadius;

    UFUNCTION(BlueprintCallable, Category = "Manifestation")
    void ActivateManifestor();

    UFUNCTION(BlueprintCallable, Category = "Manifestation")
    void DeactivateManifestor();

    UFUNCTION(BlueprintCallable, Category = "Manifestation")
    void SetManifestationIntention(const FString& Intention);

private:
    bool bIsActive;
    FString CurrentIntention;
    float ActivationTimer;
    USynchronicityEngine* SyncEngine;

    void ProcessManifestation(float DeltaTime);
    void CreateSynchronisticReality();
};