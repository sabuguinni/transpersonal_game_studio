#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "../Core/ConsciousnessSystem.h"
#include "../Archetypes/ArchetypalSystem.h"
#include "CollectiveConsciousness.generated.h"

UENUM(BlueprintType)
enum class ECollectiveState : uint8
{
    Fragmented      UMETA(DisplayName = "Fragmented Consciousness"),
    Emerging        UMETA(DisplayName = "Emerging Coherence"),
    Synchronized    UMETA(DisplayName = "Synchronized Field"),
    Unified         UMETA(DisplayName = "Unified Consciousness"),
    Transcendent    UMETA(DisplayName = "Transcendent Unity"),
    Cosmic          UMETA(DisplayName = "Cosmic Consciousness")
};

UENUM(BlueprintType)
enum class ECollectiveEvent : uint8
{
    MassAwakening       UMETA(DisplayName = "Mass Awakening"),
    CollectiveMeditation UMETA(DisplayName = "Collective Meditation"),
    SynchronicityStorm  UMETA(DisplayName = "Synchronicity Storm"),
    ArchetypalEmergence UMETA(DisplayName = "Archetypal Emergence"),
    ConsciousnessShift  UMETA(DisplayName = "Consciousness Shift"),
    UnityExperience     UMETA(DisplayName = "Unity Experience"),
    GlobalHealing       UMETA(DisplayName = "Global Healing")
};

USTRUCT(BlueprintType)
struct FCollectiveField
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
    float Coherence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
    float Resonance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
    float Harmony;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
    float Love;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
    float Wisdom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
    int32 ParticipantCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
    ECollectiveState State;

    FCollectiveField()
    {
        Coherence = 0.0f;
        Resonance = 0.0f;
        Harmony = 0.0f;
        Love = 0.0f;
        Wisdom = 0.0f;
        ParticipantCount = 0;
        State = ECollectiveState::Fragmented;
    }
};

USTRUCT(BlueprintType)
struct FCollectiveExperience
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experience")
    ECollectiveEvent EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experience")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experience")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experience")
    TArray<AActor*> Participants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experience")
    FVector EpicenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experience")
    float InfluenceRadius;

    FCollectiveExperience()
    {
        EventType = ECollectiveEvent::CollectiveMeditation;
        Intensity = 1.0f;
        Duration = 60.0f;
        EpicenterLocation = FVector::ZeroVector;
        InfluenceRadius = 1000.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UCollectiveConsciousness : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collective")
    FCollectiveField GlobalField;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collective")
    float CriticalMass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collective")
    float FieldAmplification;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float UpdateFrequency;

    UFUNCTION(BlueprintCallable, Category = "Collective")
    void UpdateCollectiveField(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Collective")
    void RegisterParticipant(AActor* Participant);

    UFUNCTION(BlueprintCallable, Category = "Collective")
    void UnregisterParticipant(AActor* Participant);

    UFUNCTION(BlueprintCallable, Category = "Collective")
    void TriggerCollectiveEvent(ECollectiveEvent EventType, FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Collective")
    bool CheckCriticalMass() const;

    UFUNCTION(BlueprintCallable, Category = "Collective")
    void InitiateMassAwakening();

    UFUNCTION(BlueprintCallable, Category = "Collective")
    void SynchronizeAllParticipants();

    UFUNCTION(BlueprintPure, Category = "Collective")
    float GetCollectiveCoherence() const;

    UFUNCTION(BlueprintPure, Category = "Collective")
    ECollectiveState GetCollectiveState() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnCollectiveStateChange(ECollectiveState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnCriticalMassReached();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnCollectiveEventTriggered(ECollectiveEvent EventType, float Intensity);

private:
    TArray<AActor*> Participants;
    TArray<FCollectiveExperience> ActiveExperiences;
    float FieldUpdateTimer;
    float LastCriticalMassCheck;

    void CalculateGlobalField();
    void ProcessActiveExperiences(float DeltaTime);
    void ApplyCollectiveInfluence(AActor* Participant, float Influence);
    void UpdateCollectiveState();
    float CalculateParticipantContribution(AActor* Participant) const;
    void CreateFieldResonance();
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCollectiveParticipant : public UActorComponent
{
    GENERATED_BODY()

public:
    UCollectiveParticipant();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Participation")
    float ContributionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Participation")
    float Receptivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Participation")
    bool bActiveParticipant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Participation")
    float FieldSensitivity;

    UFUNCTION(BlueprintCallable, Category = "Participation")
    void JoinCollective();

    UFUNCTION(BlueprintCallable, Category = "Participation")
    void LeaveCollective();

    UFUNCTION(BlueprintCallable, Category = "Participation")
    void ContributeToField(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Participation")
    void ReceiveCollectiveInfluence(float Influence);

    UFUNCTION(BlueprintPure, Category = "Participation")
    float GetFieldResonance() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnCollectiveInfluenceReceived(float Influence);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnFieldResonanceChange(float NewResonance);

private:
    UCollectiveConsciousness* CollectiveSystem;
    UConsciousnessComponent* ConsciousnessComp;
    float LastContribution;
    float FieldResonance;

    void UpdateFieldResonance(float DeltaTime);
    void ProcessCollectiveInfluence(float DeltaTime);
};

UCLASS()
class TRANSPERSONALGAME_API ACollectiveRitual : public AActor
{
    GENERATED_BODY()

public:
    ACollectiveRitual();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ritual")
    ECollectiveEvent RitualType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ritual")
    float RequiredParticipants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ritual")
    float RitualDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ritual")
    float PowerAmplification;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ritual")
    float ActivationRadius;

    UFUNCTION(BlueprintCallable, Category = "Ritual")
    void StartRitual();

    UFUNCTION(BlueprintCallable, Category = "Ritual")
    void EndRitual();

    UFUNCTION(BlueprintCallable, Category = "Ritual")
    void AddParticipant(AActor* Participant);

    UFUNCTION(BlueprintCallable, Category = "Ritual")
    void RemoveParticipant(AActor* Participant);

    UFUNCTION(BlueprintPure, Category = "Ritual")
    bool CanStartRitual() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnRitualStart();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnRitualComplete();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnParticipantJoin(AActor* Participant);

private:
    TArray<AActor*> RitualParticipants;
    bool bRitualActive;
    float RitualTimer;
    UCollectiveConsciousness* CollectiveSystem;

    void UpdateRitualField(float DeltaTime);
    void ApplyRitualEffects();
    void SynchronizeParticipants();
};